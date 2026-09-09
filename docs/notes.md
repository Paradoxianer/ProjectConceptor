# Notizen: Build & Stabilität auf Haiku

## Issue #46: Connections nach dem Laden unsichtbar, bis man selbst eine zeichnet
Datum: 2026-08-13 · Verifiziert: ja (Root Cause gelesen, Fix gebaut, 3x
`./dev.sh smoke` + manueller Test durch den Nutzer) · Quelle:
`src/plugins/GraphEditor/GraphEditor.cpp`, `ConnectionRenderer.cpp`

Hatte nichts mit dem Indexer/der ID-Serialisierung zu tun (die ist inzwischen
sauber, `IndexerTest` deckt das ab) - der Bug sitzt rein in der Live-Rendering-
Schicht. `ConnectionRenderer::Init()` ruft `ValueChanged()` einmalig beim
Bauen des Renderers auf; das liest sich die *Renderer*-Pointer (nicht die
Daten) seiner from/to-Knoten:

```cpp
tmpNode->FindPointer(editor->RenderString(),(void **)&from);
```

Existiert der Knoten-Renderer zu diesem Zeitpunkt noch nicht, bleibt `from`/
`to` NULL, und `CalcLine()` überspringt das Zeichnen still (kein Log, kein
Fallback-Wert - `if (from != NULL && to != NULL)`).

`GraphEditor::ValueChanged()` (der Handler für den `P_C_VALUE_CHANGED`-
Broadcast nach `PDocument::Load()`) hat Knoten und Connections in einem
einzigen Durchlauf über `set<BMessage*> *changedNodes` verarbeitet -
`std::set<BMessage*>` sortiert nach Zeigerwert, nicht nach Anlage-/
Abhängigkeitsreihenfolge. Landet eine Connection vor ihren Endknoten in
dieser (zur Laufzeit von Heap-Adressen abhängigen, faktisch zufälligen)
Sortierung, bekommt sie ihren Renderer gebaut, bevor die Endknoten ihren
haben - und bleibt mit `from=to=NULL` hängen, bis irgendein *späterer*
`P_C_VALUE_CHANGED`-Broadcast (z.B. weil man von Hand eine neue Connection
zeichnet) sie erneut anfasst. Erklärt exakt das gemeldete Verhalten:
mal komplett unsichtbar nach dem Laden, mal nicht - abhängig vom Zufall der
Zeigeradressen - und "erscheinen alle auf einmal", sobald irgendeine andere
Aktion einen erneuten Broadcast auslöst.

`GraphEditor::InitAll()` selbst hatte dieses Problem nie - dort laufen zwei
getrennte Schleifen (erst `allNodes`, dann `allConnections`), also immer in
der richtigen Reihenfolge. Nur der `ValueChanged()`-Pfad (Laden eines
Dokuments in ein bereits bestehendes Fenster, der Normalfall beim Start mit
Datei-Argument oder File > Open) war betroffen.

Fix: `ValueChanged()`s Schleife in zwei Pässe über dasselbe `changedNodes`-
Set aufgeteilt - erster Pass alles außer `P_C_CONNECTION_TYPE`, zweiter Pass
nur Connections. Garantiert, dass jeder Knoten/jede Gruppe ihren Renderer
schon hat, bevor eine Connection, die auf sie verweist, gebaut wird. Die
per-Knoten-Logik wanderte dafür in eine neue Methode
`GraphEditor::ProcessChangedNode()`.

Fallstrick: bei jedem Code, der über ein `std::set<T*>` iteriert und dabei
erwartet, dass "vorher angelegte" Objekte auch "vorher" in der Iteration
auftauchen - das gilt nur zufällig, nie garantiert. Wo Reihenfolge zählt,
braucht es entweder mehrere Pässe (wie hier) oder eine Datenstruktur, die
Einfügereihenfolge tatsächlich abbildet.

## `./dev.sh smoke` schien beim Beenden zu hängen - war ein Test-Tooling-Bug, kein App-Bug (Issue #88)
Datum: 2026-08-19 · Verifiziert: ja (3x `./dev.sh smoke` in Folge sauber durchgelaufen nach dem Fix)

Ursprünglich als App-Hang vermutet (siehe #88) und mehrfach über
`ProcessChangedNode`/`BringToFront`/`QuitRequested`-Instrumentierung
untersucht - alles davon war eine falsche Fährte. Der eigentliche Grund:
`/boot/home/quit` auf der VM ist ein winziges Custom-Binary für ein
anderes, dieselbe VM nutzendes Projekt (Quelle: `/boot/home/quit.cpp`,
sendet `B_QUIT_REQUESTED` fest an `application/x-vnd.Scripture-Guide`,
ignoriert sein Argument komplett). Da bei `ssh` `.` vor `/boot/system/bin`
im `PATH` steht, hat jeder bloße `quit $APP_SIG`-Aufruf (aus `dev.sh` wie
auch aus manuellen Diagnose-Befehlen) dieses Fremd-Binary getroffen statt
den echten System-`quit` - `B_QUIT_REQUESTED` kam bei ProjectConceptor nie
an. Das Tool druckt bei fehlender Zielsignatur "not running" und beendet
sich mit Exit 1, unabhängig vom übergebenen Argument - sah täuschend nach
einem stillen, erfolgreichen No-op aus.

Fix: `dev.sh` ruft `quit` jetzt mit explizitem Pfad
(`/boot/system/bin/quit`) auf.

Fallstrick, der auf dieser geteilten VM jederzeit wieder zuschlagen kann:
bei einem eigenen ~/-Skript auf der VM, das denselben Namen wie ein
System-Tool trägt, gewinnt das eigene Skript wegen `.` im `PATH` -
bei unerklärlichem Tool-Verhalten auf dieser VM immer zuerst `which
<tool>` prüfen, nicht den Tool-Namen blind vertrauen.

Projektspezifische Erkenntnisse, die beim Arbeiten mit `dev.sh` gegen die
Haiku-VM auffielen. Format wie in `~/repos/haiku-reference-md/notes/`, aber
hier abgelegt, weil es ProjectConceptor-spezifisch ist, nicht generisches
Haiku-API-Wissen.

## dev.sh: sync/build löscht von Hand auf der VM angelegte Dateien
Datum: 2026-07-31 · Verifiziert: ja (hat ein echtes Testdokument des Nutzers
tatsächlich gelöscht)

`do_sync()` macht `rsync --delete` von lokal nach remote - alles, was auf der
VM liegt, aber lokal nicht existiert, wird geloescht. Der Kommentar über der
Funktion ("Host ist die einzige Wahrheit") widerspricht dem tatsächlichen
Verhalten. Ein direkt auf der VM unter `docs/fixtures/` gespeichertes
Testdokument wurde beim nächsten `./dev.sh build` kommentarlos gelöscht.

Fix: `docs/` komplett von `do_sync` ausgeschlossen. Für einzelne Dateien von
dort `./dev.sh get` benutzen, nie `sync`/`build` als Backup-Mechanismus
verstehen. Gleiches Muster wie der frühere `bin*`-Fund - beim Anfassen von
`do_sync`'s Exclude-Liste immer fragen "was liegt sonst noch nur auf der VM
und nicht lokal".

## GraphEditor::RemoveRenderer() - double free bei Gruppen
Datum: 2026-07-31 · Verifiziert: ja (Crash-Report des Nutzers, danach mit
echtem 42-Knoten-Dokument reproduziert und den Fix verifiziert)

`GraphEditor::RemoveRenderer()` ruft für einen gruppierten Knoten
`gRenderer->RemoveRenderer(wichRenderer)` auf - `GroupRenderer::RemoveRenderer()`
loescht das Objekt bereits selbst. Direkt danach prüfte `GraphEditor::RemoveRenderer`
aber ungeachtet dessen trotzdem noch seine eigene Top-Level-Liste
(`renderer->HasItem(wichRenderer)`) und loeschte im positiven Fall ein zweites
Mal. Fix: beide Pfade sind jetzt einander ausschliessend (if/else statt zwei
unabhängige if-Bloecke).

## TextEditorControl::Invoke() ohne return (Node umbenennen stürzt ab)
Datum: 2026-07-29 · Verifiziert: ja (reproduziert beim Umbenennen eines
Knotens, Fix gebaut und erneut getestet) · Quelle: Syslog-Stacktrace

`TextEditorControl::Invoke(BMessage*)` in
`src/plugins/GraphEditor/TextEditorControl.cpp` ist als `status_t` deklariert,
hatte aber in einem Codepfad (wenn `changed && commit` falsch ist, oder auch
im wahren Zweig) keinen `return`. Kontrollfluss, der ans Ende einer
Non-void-Funktion faellt, ist UB - GCC13 baut dafuer teils eine `ud2`
(invalid opcode) Trap ein statt nur zu warnen. Absturz trat exakt beim
Bestaetigen eines umbenannten Knotennamens auf:

```
KERN: debug_server: Thread 5231 entered the debugger: Invalid opcode exception
KERN:   ... GraphEditor> _ZN17TextEditorControl6InvokeEP8BMessage + 0x1ae
KERN:   ... GraphEditor> _ZN17TextEditorControl7KeyDownEPKci + 0x4b
```

Fix: beide Pfade geben jetzt explizit einen Wert zurueck
(`BInvoker::Invoke(&copy)` bzw. `B_OK`). Andere Renderer-Klassen im selben
Verzeichnis (StringRenderer, BoolRenderer, AttributRenderer) haben keine
eigene `Invoke()`-Ueberschreibung, sind also nicht betroffen.

## NavigatorEditor haelt einen Rohzeiger auf allNodes/allConnections ueber Load() hinweg
Datum: 2026-07-29 · Verifiziert: ja (reproduziert mit `docs/fixtures/smoke-test.pcd`
per `./dev.sh debug <pfad>`, Fix gebaut und erneut getestet) · Quelle:
Syslog-Stacktrace

`NavigatorEditor::InitGraph()` holt sich beim Erzeugen des Editors einmalig
`doc->GetAllNodes()` und reicht den `BList*` an `NodeListView` durch, die ihn
dauerhaft als Member haelt. `PDocument::Load()` hat bisher aber
`delete allNodes; allNodes = docLoader->GetAllNodes();` gemacht - also den
Container komplett durch ein neues Objekt ersetzt statt ihn wiederzuverwenden.
`NodeListView::nodes` zeigte danach auf laengst freigegebenen Speicher; der
naechste `ValueChanged()`-Aufruf (passiert automatisch direkt nach jedem
Load(), weil `PDocument::Load()` am Ende `P_C_VALUE_CHANGED` broadcastet)
knallte zuverlaessig:

```
KERN: debug_server: Thread 5920 entered the debugger: General protection fault
KERN:   ... BList::DoForEach + 0x28
KERN:   ... NavigatorEditor> NodeListView::ValueChanged + 0x4e
KERN:   ... NavigatorEditor> NavigatorEditor::ValueChanged + 0x7a
```

Betraf nur `NavigatorEditor` - alle anderen Plugins holen sich `GetAllNodes()`/
`GetAllConnections()`/`GetSelected()` frisch pro Methodenaufruf, keine
weiteren Cache-Stellen gefunden (geprueft per grep ueber alle Plugins).

Fix in `PDocument::Load()`: `allNodes`/`allConnections` bleiben dieselben
Objekte (per `MakeEmpty()` geleert und neu befuellt), nur die vom `PDocLoader`
gelieferten temporaeren Listen-Container werden gel​oescht. Nicht der Indexer
selbst (der kuemmert sich nur um die Pointer-als-ID-Serialisierung pro Knoten),
sondern eine Ebene hoeher in `PDocument::Load()`.

## Zwei Crash-on-quit-Ursachen behoben (Issue #52)
Datum: 2026-07-30 · Verifiziert: ja (`./dev.sh smoke` 10x hintereinander
sauber durchgelaufen, keine neuen Eintraege im Syslog)

**1. `GraphEditor::DetachedFromWindow()` - use-after-free.** Griff beim
Detach immer auf `pWindow->FindView(P_M_STATUS_BAR)`/`FindView(P_M_STANDART_TOOL_BAR)`
zu, um eigene Menu-/Toolbar-Items zu entfernen. Wenn dieses Detach Teil der
Zerstoerung des GESAMTEN Fensters ist (nicht nur dieser eine Editor wird
entfernt), sind Menubar/Toolbar zu dem Zeitpunkt eventuell schon selbst
Teil derselben `~BWindow()`-Kaskade und bereits weg - `FindView` liefert
dann einen Pointer auf laengst freigegebenen Speicher, der beobachtete
Crash-Report zeigte entsprechend offensichtlich korrupte Objektdaten in
`configBar`.

Fix: `PWindow` bekommt ein `closing`-Flag, gesetzt in
`PDocument::QuitRequested()` direkt vor `window->Quit()` (der Stelle, die
die Zerstoerung ueberhaupt erst auslpest). `GraphEditor::DetachedFromWindow()`
ueberspringt die Menu-/Toolbar-Aufraeumarbeit, wenn das Flag gesetzt ist -
diese Views werden ohnehin gleich mit zerstoert. Die eigene
Renderer-Aufraeumarbeit (`RemoveRenderer`) bleibt in jedem Fall bestehen,
die betrifft nur GraphEditors eigenen Zustand.

**2. `PDocumentManager::~PDocumentManager()` - double free.** Rief
`currentDocument->Quit()` (PDocument ist ein `BLooper`) UND direkt danach
`delete currentDocument` auf. Laut Haiku-Doku
(`haiku-book-md/application_kit/BLooper.md`, Abschnitt `Quit()`): *"You
will not have to delete the looper object, if a looper quits it will
delete itself."* - das explizite `delete` war ein Double-free auf
dasselbe Objekt. Nebenbei iterierte die Schleife per aufsteigendem Index
ueber `documentList`, waehrend `PDocument::~PDocument()` (via
`RemoveDocument()`) genau diese Liste waehrenddessen verkuerzt - ein
zweiter, unabhaengiger Bug (Items wuerden uebersprungen), der bisher
nur nicht auffiel, weil meist nur ein Dokument offen ist.

Fix: `delete` entfernt, Schleife auf `while (documentList->CountItems() > 0)`
mit `ItemAt(0)` umgestellt statt aufsteigendem Index.

## `dev.sh smoke`: ein invertierter Vergleich hat stundenlang "Absturz"
vorgetaeuscht
Datum: 2026-07-30 · Verifiziert: ja

Beim ersten Bauen der Absturz-beim-Beenden-Erkennung in `smoke()` (siehe
oben) stand am Ende `if [ "$still_running" -eq 0 ]; then echo "HAENGT..."`.
`still_running=0` heisst aber "Prozess ist weg, sauber beendet" - der
Vergleich war exakt verkehrt herum und hat jeden erfolgreichen, sogar
schnellen Quit als Haenger gemeldet. Das sah beim Testen aus wie eine
zeitabhaengige Race Condition (mal bestand ein manueller Repro-Versuch,
mal schlug `./dev.sh smoke` mit identischer Befehlsfolge fehl) und hat
viel Zeit gekostet, bis der Vergleich selbst als Ursache auffiel.

Fallstrick: bei jedem knappen `-eq 0`/`-eq 1`-Vergleich in einem
Shellskript kurz gegenlesen, was die Variable in jedem Zweig *wirklich*
bedeutet - "sieht plausibel aus" reicht nicht, wenn beide Zweige einen
Text ausgeben, der fuer sich genommen Sinn ergibt.

## Weitere, noch offene Befunde vom ersten interaktiven Testlauf (2026-07-29)
- **Gruppieren** wurde laut Log tatsaechlich mit `Command::Name=Group` und
  3 Knoten ausgeloest, kein Absturz dabei, aber auch keine sichtbare
  Aenderung im GraphEditor fuer den Tester. Ursache noch unklar, braucht
  gezielten Repro.
- **Keine Translator-Auswahl im Speichern-unter-Dialog**: haengt an Issue #67
  (Translators werden nur in den lokalen Repo-`bin/`-Baum gebaut, nie an einen
  von `BTranslatorRoster` gescannten Ort installiert). Normales Speichern
  (nicht "Speichern unter") braucht keinen Translator und funktioniert.

## Ohne Translator: geladenes Dokument bleibt leer, ganz ohne Fehler (Issue #67)
Datum: 2026-07-29 · Verifiziert: ja (mit `docs/fixtures/smoke-test.pcd`
reproduziert: `Indexer::DeIndexNode` lief 0x, nach Installation des
Translators 9x fuer dieselbe Datei) · Quelle: `./dev.sh debug <pfad>`,
`grep` auf die Trace-Zeilen

`PDocument::Save()` schreibt im Standardfall (kein Format explizit gewaehlt)
die Bytes direkt und roh per `BFile` - ganz ohne Translator. `PDocument::Load()`
liest aber *immer* ueber `BTranslatorRoster::Identify()`/`Translate()`, auch
fuers eigene native Format:

```cpp
err = roster->Translate(file,indentifed,NULL,output,P_C_DOCUMENT_RAW_TYPE);
if (err == B_OK) {
    err = loaded->Unflatten(output);
```

Findet `BTranslatorRoster::Default()` keinen passenden Translator (z.B. weil
`ProjectConceptorTranslator` wie in Issue #67 beschrieben nirgends installiert
ist, wo sie gescannt wird), ist `err != B_OK`, `Unflatten()` wird uebersprungen,
und `loaded` bleibt eine leere `BMessage` - **ohne jede Fehlermeldung**.
`PDocLoader::Spread()` findet dann schlicht keine `"node"`-Eintraege und
liefert eine leere Liste. Ergebnis: Dokument "laedt" ohne Absturz und ohne
Fehler, zeigt aber nichts an. Genau das hat den Tester im ersten Durchlauf
verwirrt.

Fix (Workaround, kein echtes Package-Install - das bleibt #67): `./dev.sh build`
kopiert die gebauten Translatoren jetzt automatisch nach
`~/config/non-packaged/add-ons/Translators/` auf der VM, wo
`BTranslatorRoster::Default()` sie findet.

## Werkzeug zum Inspizieren von .pcd-Dateien
Datum: 2026-07-29 · Verifiziert: ja

`.pcd`-Dokumente sind ein direkt geflattenter `BMessage` (kein XML - das
TinyXML-Zeug betrifft nur `ConfigManager` und die FreeMind/Standard-Translatoren).
Zum Nachschauen/Bearbeiten von Hand: `pkgman install xmlbmessage`
(`./dev.sh pkg xmlbmessage`), dann `xmlbmessage --force datei.pcd datei.xml`.

## TinyXML-ABI-Mismatch beim Start (Issue #62, Regression von #31)
Datum: 2026-07-29 · Verifiziert: ja (reproduziert per `./dev.sh debug`,
Fix getestet: Absturz tritt danach nicht mehr auf) · Quelle: Syslog-Stacktrace
via `./dev.sh syslog`, Commit d67225a (2019-11-24)

Issue #31 ("Remove TinyXML from project") wollte 2019 bewusst weg vom
mitgelieferten TinyXML hin zu einer externen Paket-Abhängigkeit. Commit
d67225a hat dafür nur `tinyxml.cpp`/`tinystr.cpp` aus `src/app/makefile`
entfernt (damit die Implementierung nicht mehr mitgebaut wird) - die
`#include "tinyxml/tinyxml.h"`-Pfade und die mitgelieferten Header unter
`src/include/tinyxml/` blieben liegen. Seitdem kompilierte die App gegen den
**mitgelieferten** Header (`TIXML_USE_STL` nicht gesetzt, nutzt die
projekteigene `TiXmlString`-Klasse), linkte aber gegen die **System**-Lib
`libtinyxml.so`, deren Header (`/boot/system/develop/headers/tinyxml.h`)
`TIXML_USE_STL` fest auf 1 setzt (System-Lib nutzt intern `std::string`-Layout
für `TiXmlDocument`/`TiXmlElement`/etc.). Zwei inkompatible Objektlayouts fuer
dieselbe Klasse - Heap-Korruption beim ersten Destruktor-Aufruf.

Reproduzierbarer Crash vor dem Fix, bereits beim allerersten Start
(`ConfigManager`-Konstruktor schreibt die Default-Config):

```
KERN: 2642: DEBUGGER: bogus pointer (double free?) 0x10381091198
...
_ZN11TiXmlString4quitEv + 0x3a
_ZN11TiXmlStringD1Ev + 0x18
_ZN13TiXmlDocumentD2Ev + 0x2e
_ZN16MessageXmlWriter5SetToERK7BString + 0x84
_ZN13ConfigManager10SaveConfigEv + 0x80
_ZN13ConfigManagerC2EPcP8BMessage + 0x91
_ZN16ProjektConceptor10ReadyToRunEv + 0x12a
```

`src/translators/FreeMind/FreeMindTranslator.h` macht es seit jeher richtig:
`#include "tinyxml.h"` (ohne `tinyxml/`-Präfix) trifft nie den mitgelieferten
Header, sondern über Gccs Default-Suchpfad direkt den System-Header - dadurch
war der Translator nie von diesem Bug betroffen.

**Fix:** `MessageXmlWriter.h`, `MessageXmlReader.h/.cpp` auf
`#include "tinyxml.h"` umgestellt (wie FreeMindTranslator), `src/include/tinyxml/`
entfernt. Enthielt ohnehin nur Header, keine `.cpp` - die App war seit 2019
architektonisch bereits auf eine externe tinyxml-Lib angewiesen, nur die
Include-Pfade zeigten noch auf die Leiche.

Fallstrick: Nach dem Entfernen eines Headers, auf den eine alte `.d`-Datei
noch verweist, bricht `make` mit `No rule to make target '.../tinyxml.h'` ab,
weil es versucht die (jetzt fehlende) Header-"Regel" zu bauen. Erst
`./dev.sh clean` beseitigt das.

## PDocumentManager double free beim Beenden (Issue #52)
Datum: 2026-07-29 · Verifiziert: ja (reproduziert per sauberem `./dev.sh stop`,
also regulärem `B_QUIT_REQUESTED`, nicht nur bei hartem Kill) · Quelle:
Syslog-Stacktrace via `./dev.sh syslog`

Issue #52 vermutete die Ursache in nicht threadsicherer BMenu-Deregistrierung
in GraphEditor. Der tatsächlich beobachtete Stacktrace zeigt den Absturz
(„double free") aber in `PDocumentManager::~PDocumentManager()`:

```
KERN: 4591: DEBUGGER: double free 0x78bd9b7540
KERN: debug_server: Thread 4591 entered the debugger: Debugger call: `double free 0x78bd9b7540'
KERN:   ... _ZN16PDocumentManagerD2Ev + 0x9e
```

Nicht weiter untersucht (Stack-Walk bricht bei manchen Läufen nach wenigen
Frames ab, vermutlich weil der Heap zu diesem Zeitpunkt schon korrupt ist).
Ob das dieselbe Ursache wie die vermutete BMenu-Geschichte ist oder ein
zweiter, unabhängiger Bug in derselben Teardown-Phase, ist offen. Gehört zu
Phase 2 im Release-Plan, nicht angefasst.

## `./dev.sh debug` vs. `./dev.sh run` fuer Absturz-Diagnose
Datum: 2026-07-29 · Verifiziert: ja

`run` leitet stdout/stderr der App remote in eine Datei um - da das kein TTY
ist, puffert die libc dort blockweise. Bei einem Absturz mitten im
Programmlauf fehlt im lokal abgeholten `debug.log` typischerweise der Rest
des zuletzt angefangenen (ungeflushten) Puffers, und zwar exakt an derselben
Stelle bei jedem Lauf (weil die Ausgabemenge bis dahin deterministisch ist) -
das sieht täuschend nach "Programm haengt immer an Stelle X" aus, ist aber nur
ein Pufferungsartefakt. `./dev.sh debug` startet stattdessen über `ssh -tt`
mit echtem Pseudo-TTY, dadurch zeilengepuffert und der tatsächliche Endpunkt
sichtbar. Fuer Absturz-Diagnose immer `debug`, nicht `run`, verwenden.

## Headless CppUnit-Tests (Issue #68) - PDocument ohne Fenster, PluginManager-Falle

Datum: 2026-08-01 · Verifiziert: ja (`./dev.sh test`, `src/tests/`)

`Indexer`/`PCommand` brauchen kein `app_server`, aber `PDocumentManager::Init()`
crasht ohne laufende `be_app` (kein Null-Check vor `be_app->GetAppInfo()`), und
`PDocument::Init()` erzeugt immer ein echtes `PWindow`. Lösung: minimale
`BApplication` im Test-`main()` (nie `Run()`), `PDocument(PDocumentManager*,bool
headless)` (neuer Konstruktor, ruft nur `_InitData()` + `PCommandManager`, kein
Fenster/`PEditorManager`), `PDocumentManager` über den Archiv-Konstruktor mit
leerer `BMessage` bauen (`Init(BMessage*)` erzeugt nur Dokumente für vorhandene
`"document"`-Eintraege - bei keinem entfaellt `CreateDocument()`, das sonst
unbedingt ein echtes Dokument+Fenster anlegt).

**Fallstrick, der stundenlang wie ein Deadlock aussah:**
`PluginManager::LoadPlugins()` zeigt eine *blockierende* `BAlert::Go()`, wenn
am Ende null Plugins geladen wurden ("Cant find plugins") - ohne Nutzer, der
klickt (jeder nicht-interaktive Testlauf), haengt der Prozess für immer, ohne
jede Fehlermeldung. Der Test-Binary braucht daher zwingend ein `Plugins/`-
Verzeichnis mit mindestens einem ladbaren Plugin neben sich; `./dev.sh test`
legt dafür einen Symlink auf die echten gebauten Plugins an (`ln -sfn
../../bin/apps/ProjectConceptor/Plugins src/tests/Plugins`) - **bei jedem
Lauf neu**, weil `do_sync`s `rsync --delete` ihn sonst beim naechsten Sync
wieder loescht (VM-only, existiert lokal nicht).

Testbinary linkt (fast) denselben Sourcen-Satz wie die App selbst
(`src/tests/makefile`, minus `ProjectConceptor.cpp`) statt gegen die App als
Bibliothek zu linken - Translator-Add-ons duerfen unaufgeloeste Symbole haben
(werden zur Laufzeit in den ladenden Prozess reinaufgeloest), ein eigenstaendiges
Testbinary nicht.

## 300-Knoten-Stressfixture deckt echten fd-Leak in Indexer auf (Issue #71)

Datum: 2026-08-01 · Verifiziert: ja (`src/tools/GenerateStressFixture/`, reproduziert
mit und ohne Editor-Plugins geladen)

`.pcd`-Dateien sind rohes geflattetes `BMessage` (Haiku "HMF1"), keine XML -
`MessageXmlWriter` ist nur fuer `ConfigManager`s Settings-Datei. Der Generator
baut deshalb ein echtes `PDocument` (headless-Konstruktor, siehe oben) und
ruft dessen echten `Archive()`/`Flatten()`-Pfad auf, statt das Format von
Hand nachzubauen.

Beim Erzeugen mit allen echten Plugins (inkl. GraphEditor/NavigatorEditor)
schlug `Flatten()` ab ~250 Knoten mit "Too many open files" fehl -
`Indexer::IndexNode()`/`IndexConnection()`/deren DeIndex-Gegenstuecke rufen
pro Knoten/Verbindung `plugin->GetNewObject(NULL)` fuer jedes Editor-Plugin
auf und geben die Instanz nie wieder frei (kein `delete`, kein Cache). Mit
nur Command-Plugins geladen (keine Editor-Plugins) verschwindet der Leak
komplett - bestaetigt die Editor-Preprocessing-Schleife als Quelle, nicht
die Knotenzahl an sich. Als #71 gemeldet, echtes Save-Pfad-Problem, kein
reines Test-Artefakt.

**Fix (2026-08-01):** `Indexer::GetCachedEditors()` haelt jetzt eine
PEditor-Instanz pro Editor-Plugin fuer die Lebensdauer des `Indexer`-Objekts
(einmal pro Save/Load, nicht mehr pro Knoten/Verbindung), freigegeben in
`~Indexer()`. Sicher, weil `PreprocessBeforSave()`/`PreprocessAfterLoad()`
in GraphEditor/NavigatorEditor nachweislich zustandslos sind (arbeiten nur
auf der uebergebenen `BMessage`, nie auf eigenem Instanzzustand). Verifiziert:
mit allen echten Plugins (GraphEditor+NavigatorEditor+Commands) laeuft der
Generator jetzt bei 2200 Knoten durch, vorher schlug er bei ~250 fehl - der
Commands-only-Workaround unten ist fuer den fd-Leak nicht mehr noetig,
bleibt aber wegen des Symlink-Fallstricks relevant.

Nebenbefund beim Debuggen: ein `Plugins/`-Verzeichnis mit genau einem
Plugin, das per Symlink *direkt* (nicht ueber seinen echten Unterordner)
eingehaengt wird, haengt sich in `PluginManager::LoadPlugins()` auf -
Ursache nicht weiter verfolgt (out of scope), Workaround: immer den echten
Unterordner symlinken (z.B. `Plugins/Commands`), nie einzelne Plugin-Dateien.

## Live-Preview waehrend Drag/Pick: Muster ist "nur Renderer, ein Commit am Ende" - kein shadow-Flag

Datum: 2026-08-27 · Verifiziert: ja (Code gelesen, `src/plugins/MoveCommand/Move.cpp`,
`src/plugins/GraphEditor/ClassRenderer.cpp`) · Quelle: siehe oben

Beim Bau des ColorToolItem-Redesigns (#112) stand die Frage: wie bekommt man
eine *Live-Vorschau* waehrend eines interaktiven Vorgangs (Farbe im Picker
ziehen, Knoten mit der Maus verschieben), ohne dass jeder Zwischenschritt
einen eigenen Undo-Eintrag erzeugt?

`PCommandManager::Execute()` kennt zwar ein `"shadow"`-Bool-Feld auf der
Command-`settings`-BMessage (`Copy` nutzt es, um sich selbst nicht in die
Undo-Liste einzutragen) - das legt nahe, man koennte jeden Zwischenschritt
als `shadow=true` schicken und nur den letzten Schritt "echt" (nicht-shadow)
senden. Funktioniert so aber NICHT sauber: `ChangeValue::DoChangeValue()`
liest den "alten Wert" fuer den Undo-Eintrag immer aus dem *aktuellen*
Knotenzustand, nie aus einem uebergebenen Wert - der letzte, "echte" Befehl
wuerde als Undo-"davor" also die letzte Vorschau-Farbe zeigen, nicht die
Farbe von vor dem Oeffnen des Pickers.

**Das tatsaechlich im Code verwendete Muster ist ein anderes, saubereres:**
Move/Resize benutzen `shadow` gar nicht. `ClassRenderer::MouseMoved()`
aendert waehrend des Ziehens *nur* die Renderer-Geometrie direkt (Offset auf
die gezeichnete Position, `editor->Invalidate()`) - das Dokument/die
Node-BMessage wird waehrenddessen ueberhaupt nicht angefasst, kein
`P_C_EXECUTE_COMMAND` wird gesendet. Erst `ClassRenderer::MouseUp()`
schickt *einen einzigen* `"Move"`-Befehl mit dem Gesamt-Delta
(`where - startMouseDown`, nicht die Summe vieler kleiner Deltas). Der Move-
Command selbst arbeitet ohnehin relativ (`dx`/`dy`), nicht absolut - genau
deshalb reicht ein einziger Befehl mit der Gesamtstrecke.

Fuer Farbe (oder jede andere "live vorschau, dann committen"-Interaktion)
heisst das: die Live-Vorschau muss auf Renderer-Ebene passieren (ein
"Vorschau-Override" fuer die gezeichnete Fuellfarbe, der die echten
BMessage-Daten des Knotens nicht anfasst), nicht ueber shadow-Commands. Erst
beim Schliessen/Commit des Pickers wird ein einziger echter `ChangeValue`-
Befehl mit der final gewaehlten Farbe gesendet - dessen automatisch
eingelesener "alter Wert" ist dann korrekt die echte Ausgangsfarbe, weil
zuvor nichts anderes die Knotendaten veraendert hat.

## SetViewColor(B_TRANSPARENT_COLOR) + eigenes Draw() ohne volle Deckung = Ghosting

Datum: 2026-08-29 · Verifiziert: ja (Live-Screenshot auf der VM zeigte das
Artefakt, Fix gebaut+getestet) · Quelle: `AlphaSlider.cpp`
(`src/app/ToolBar/`), portiert aus Icon-O-Matic

Icon-O-Matics `AlphaSlider` setzt im Konstruktor `SetViewColor(B_TRANSPARENT_
COLOR)`. Das deaktiviert Haikus automatisches Loeschen der View auf ihre
ViewColor *vor* jedem `Draw()`-Aufruf - die View ist dann komplett selbst
dafuer verantwortlich, bei jedem Redraw wirklich jeden Pixel neu zu setzen.
Im Original ist das unproblematisch, weil `DrawBitmap()` die komplette
Bar-Flaeche jedes Mal deckend neu zeichnet.

Beim Erweitern um einen zusaetzlichen "End-Cap"-Swatch und einen eigenen
Thumb (statt der urspruenglichen simplen Marker-Linien) blieb zwischen
Gradient-Bar und End-Cap ein paar Pixel breiter Zwischenraum, der von
*nichts* explizit gemalt wurde (`_BitmapRect()` und `_EndCapRect()` decken
zusammen nicht ganz `Bounds()` ab). Unter `B_TRANSPARENT_COLOR` wird dieser
Zwischenraum nie geloescht - alte Pixel (z.B. Reste eines Thumb an einer
frueheren Position) blieben stehen und sammelten sich ueber mehrere Redraws
zu sichtbaren "Schleifspuren" an.

Fix: `SetViewColor()` auf eine echte Hintergrundfarbe (hier `ui_color(B_
PANEL_BACKGROUND_COLOR)`, passend zum Fenster drumherum) statt
`B_TRANSPARENT_COLOR` - dann loescht Haiku die View vor jedem `Draw()`
zuverlaessig selbst, unabhaengig davon, ob die eigene Draw()-Logik wirklich
jeden Pixel abdeckt.

Fallstrick: `B_TRANSPARENT_COLOR` nur verwenden, wenn `Draw()` garantiert
*jeden* Pixel der Bounds bei *jedem* Aufruf neu zeichnet (z.B. ein einzelnes
deckendes `DrawBitmap()` ueber die volle Flaeche) - sobald mehrere
Teil-Zeichenoperationen mit Luecken dazwischen ins Spiel kommen, ist das
bruechig und der Fehler zeigt sich erst als optisches Artefakt, nicht als
Crash oder Compilerfehler.

## Ueberlappende Sibling-Views direkt an einem BWindow fangen alle Klicks weg

Datum: 2026-08-29 · Verifiziert: ja (Live-Regression: alle Controls im
ColorPickerWindow liessen sich nicht mehr bedienen, Fix gebaut+getestet)
· Quelle: `ColorPickerWindow.cpp` (`src/app/ToolBar/`)

Um das oben beschriebene Hintergrund-Artefakt zu fixen, wurde eine
fensterfuellende `BView` als Hintergrund per `AddChild()` **direkt am
BWindow** hinzugefuegt, als erstes Kind - vor Palette, History, dem
`BColorControl` und dem `AlphaSlider`, die alle *ebenfalls* direkt als
Kinder des Fensters haengen (Geschwister, kein gemeinsamer Eltern-View).
Ergebnis: kein einziger Klick im Fenster kam mehr bei irgendeinem Control
an - alle Slider/Swatches liessen sich nicht mehr bedienen.

Ursache: mehrere ueberlappende *Sibling*-Views, die direkt an einem
`BWindow` haengen, werden von Haiku beim Hit-Testing nicht zuverlaessig
nach Z-Reihenfolge (zuletzt hinzugefuegt = oberste) an den richtigen View
weitergeleitet - der zuerst hinzugefuegte, fensterfuellende Hintergrund-View
gewinnt den Treffer-Test fuer praktisch jeden Klick im Fenster, unabhaengig
davon, was optisch "darueber" gezeichnet wird.

Fix: der Hintergrund-View bleibt weiterhin einziges direktes Kind des
Fensters, aber *alle* echten Controls (Palette, History, ColorControl,
AlphaSlider, TextControls) werden stattdessen per `background->AddChild()`
als **echte Kinder des Hintergrund-Views** eingehaengt, nicht mehr als
Geschwister direkt am Fenster. Echtes Eltern-Kind-Nesting leitet
Maus-Events zuverlaessig an das tiefste/oberste passende Kind weiter -
im Gegensatz zu ueberlappenden Geschwistern am selben `BWindow`.

Fallstrick: ein fensterfuellender Hintergrund-View, der Klicks "durchlassen"
soll, muss der **einzige** direkte Fenster-Kind sein und alle interaktiven
Controls muessen *seine* Kinder sein - nicht seine Geschwister, egal in
welcher Add-Reihenfolge.

## PDocument::Load() crashte auf einem headless PDocument - jetzt gefixt

Datum: 2026-09-04, Fix 2026-09-08 · Verifiziert: ja (siehe unten) ·
Quelle: `src/app/Document/PDocument.cpp`

**Update 2026-09-08:** gefixt, im Zuge von Issue #117 (derselbe
NULL-`editorManager`-Fehler saß unabhängig davon auch dreimal in
`PCommandManager::Execute()`/`Undo()`/`Redo()`). Beide verbleibenden
unbedingten `editorManager->BroadCast(...)`-Aufrufe in `PDocument.cpp`
(`Resize()` und das Ende von `Load()`) haben jetzt denselben
`if (editorManager != NULL)`-Guard. Nicht separat mit einem headless-
`Load()`-Test verifiziert (`Load()` braucht einen bereits gesetzten
`entryRef`, dafür gibt es noch keinen öffentlichen Setter - siehe
Fallstrick unten, weiterhin unverändert) - Verifikation ist der
identische, bereits dreifach getestete Guard aus #117
(`PCommandTest::ExecuteViaRealMessageDispatchSurvivesProcessExit`) plus
die bestehenden Smoke-Tests, die `Load()` in der echten App laufen
lassen und weiterhin sauber durchlaufen.

Ursprünglicher Befund (2026-09-04): `PDocument::Load()` rief am Ende
unbedingt `editorManager->BroadCast(new BMessage(P_C_VALUE_CHANGED))`
auf. Ein per `PDocument(PDocumentManager*, bool headless)` gebauter
Testdokument (siehe `TestDocument.h`/`NewHeadlessTestDocument()`) hat
laut eigenem Klassenkommentar bewusst kein `editorManager` - der bleibt
dort `NULL`. Ein CppUnit-Test, der `Load()` auf einem headless Dokument
aufruft, crashte deterministisch beim `BroadCast()`-Aufruf, unabhängig
vom eigentlich zu testenden Verhalten.

Fallstrick (weiterhin gültig): `Load()` ist mit der aktuellen
Testinfrastruktur nicht headless testbar, unabhängig vom obigen Fix -
`Load()` braucht einen bereits gesetzten `entryRef`, und der ist ein
`PDocument`-Member ohne öffentlichen Setter, ähnlich wie `window` vor der
`SetWindow()`-Ergänzung. Bis es einen entsprechenden Setter gibt, bleibt
`Load()` nur end-to-end über eine echte laufende App testbar (was die
Smoke-Tests bereits abdecken), nicht über CppUnit.

## Design-Prinzipien-Check vor dem Release (letzte ~12 Monate)

Datum: 2026-09-08 · Verifiziert: ja (Commits gelesen, aktueller Code
gegen die dokumentierte Absicht geprüft) · Quelle: `git log --since
"1 year ago"` (129 Commits), alte `@class`/`@brief`-Doxygen-Kommentare im
Quellcode, CLAUDE.md-Abschnitt "Design Patterns"

Stichprobenartige, gezielte Prüfung statt Commit-für-Commit-Review (129
Commits) - CLAUDE.md's eigener "Design Patterns"-Abschnitt ist bereits
eine Destillation der alten, im Code dokumentierten Absicht (Command für
`PCommand`, Memento für Undo-Snapshots, Repository/Identity Map für
`Indexer`, Abstract Factory für Plugin-/Renderer-Erzeugung, Observer vs.
`PEditorManager::BroadCast()`). Ergebnis pro Muster:

- **Command (`PCommand::Do()`/`Undo()`):** #116 (`PCommand::Do()`
  überschrieb Undo-Infos aller Subcommands außer des letzten) war eine
  echte, jetzt gefixte Verletzung dieses Vertrags. Sonst keine weitere
  Abweichung gefunden.
- **Repository/Identity Map (`Indexer`):** `3f42cba` ("Replace
  pointer-based node identity with stable int32 ids") hat die
  dokumentierte Absicht (stabile IDs statt Zeiger-Identität über eine
  Save/Load-Grenze hinweg) tatsächlich erst *durchgesetzt* - vorher
  wich der Code selbst davon ab. Diese Session's eigene neue Felder
  (`P_C_NODE_CONNECTION_ARROWS` u.a.) sind einfache Skalare, keine
  Zeiger, und brauchen daher keine Indexer-Sonderbehandlung.
- **Abstract Factory (`GraphEditor::CreateRendererFor()`):**
  weiterhin die einzige Stelle, die Renderer per `switch(node->what)`
  erzeugt. Mehrere Commits dieses Jahr (`c2ec756`, `2876e4e`, `75a9485`,
  `5c01bd4`, `24d6783`, `16fd583`) drehten sich um
  Eigentümerschaft/Doppelfreigabe genau an dieser Stelle - keine davon
  hat die Fabrik selbst umgangen, alle reparierten Bugs *innerhalb*
  des Musters.
- **Observer vs. `PEditorManager::BroadCast()`:** unverändert der
  dokumentierte "alles benachrichtigen"-Holzhammer, keine Verschlimmerung
  durch neue, breitere Broadcasts gefunden. Durch #117 sogar robuster
  geworden (NULL-`editorManager` jetzt an allen bekannten Stellen
  abgesichert, siehe oben).

Kein Scope-Drift gefunden, der einen eigenen Fix bräuchte, über die
bereits behobenen #116/#117 hinaus. Ein niedrigrisikiges, nicht
reproduzierbares Fundstück am Rande: `PDocument::Print()` ruft
`editorManager->GetActivPEditor()` ebenfalls ungeschützt auf
(`PDocument.cpp:497`) - nur über eine echte Druck-Aktion erreichbar, für
die `editorManager` immer gesetzt ist, also aktuell nicht auslösbar;
nicht spekulativ gefixt, hier nur vermerkt.
