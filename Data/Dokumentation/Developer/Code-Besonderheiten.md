# Code-Besonderheiten & Fallstricke

Sammlung von Architektur-Eigenheiten und Fallstricken, die beim Arbeiten am
GraphEditor-Kern wiederholt aufgetreten sind oder leicht übersehen werden.
Kein Änderungslog (das ist `docs/notes.md`, chronologisch pro Bug) - dieses
Dokument beschreibt **strukturelle Eigenheiten, die beim nächsten Anfassen
desselben Codes wieder relevant werden**. Ergänzt die bestehenden
`Aufbau Connection`/`Aufbau DataContainer`-Docs in diesem Ordner.

---

## Datenmodell: alles ist eine `BMessage`

Ein Knoten (`P_C_CLASS_TYPE`), eine Gruppe (`P_C_GROUP_TYPE`) und eine
Verbindung (`P_C_CONNECTION_TYPE`) sind alle drei einfach `BMessage`-Objekte,
unterschieden nur über `what`. Es gibt keine eigene C++-Klassenhierarchie für
Knotendaten - Feldnamen sind Konstanten aus `ProjectConceptorDefs.h`
(niemals String-Literal inline, siehe CLAUDE.md). Für den konkreten
Feld-Aufbau: `Aufbau Connection`, `Aufbau DataContainer` in diesem Ordner.

## Renderer-Pointer werden direkt auf dem Node-BMessage gecacht

`GraphEditor::CreateRendererFor()` schreibt den frisch gebauten `Renderer*`
per `node->AddPointer(renderString, newRenderer)` **direkt in das
Node-BMessage selbst** - es gibt keine separate Map von Node zu Renderer.
`InitAll()`/`ProcessChangedNode()` prüfen genau dieses Feld
(`FindPointer(renderString, ...)`), um zu entscheiden, ob für einen Knoten
schon ein Renderer existiert. Konsequenz: ein Node-BMessage, das für
Speichern/Laden serialisiert wird, muss dieses (und alle anderen
Pointer-)Felder vorher entfernen - das macht `PreprocessBeforSave()`.

## Zeichenreihenfolge: EINE einzige Liste entscheidet, nicht die Gruppen-eigene

`GraphEditor::Draw()` zeichnet ausschließlich über
`renderer->DoForEach(DrawRenderer,this)` - die **eigene, oberste**
`BList *renderer` von `GraphEditor`, in Listenreihenfolge (später = oben).
Eine `GroupRenderer`-Instanz hat zwar auch eine eigene interne Liste ihrer
Kinder, aber die ist **reine Buchhaltung** fürs Kaskadieren von
`MoveBy()`/`ResizeBy()` - sie wird nie selbst gezeichnet. Jeder Renderer,
egal ob top-level oder von einer Gruppe erzeugtes Kind, landet über
`GraphEditor::AddRenderer()` in genau dieser einen Liste.

Daraus folgt: **Kinder müssen in dieser Liste nach ihrer Gruppe stehen**,
sonst malt die Gruppe (die einen deckenden Hintergrund zeichnet) über sie
drüber. Das ist kein einmaliger Bug, sondern eine strukturelle Invariante,
die an mehreren, unabhängigen Stellen gebrochen werden kann:

- **Reaktiv beim Selektieren**: `GraphEditor::BringToFront()` bringt eine
  Gruppe UND ihre Kinder/Connections gemeinsam nach vorn.
- **Beim kompletten Neuaufbau**: `GraphEditor::InitAll()` iteriert
  `doc->GetAllNodes()` und baut fehlende Renderer - diese Liste hat aber
  keine garantierte Gruppen-vor-Kindern-Reihenfolge (sie spiegelt die
  Dokumenten-Speicherung, nicht Verschachtelung). Deshalb läuft `InitAll()`
  in zwei Pässen: erst alle `P_C_GROUP_TYPE`-Knoten, dann der Rest.

**Wann `InitAll()` erneut läuft, ist selbst ein Fallstrick** (siehe nächster
Abschnitt) - jede neue Stelle, die Renderer neu aufbaut, muss dieselbe
Gruppen-zuerst-Regel beachten, sonst reproduziert sie denselben Bug in neuer
Form.

## Tab-Wechsel zerstört und baut ALLE Renderer neu auf

`MainView`s `BTabView` hat kein `SetLayout()` gesetzt. Ohne Layout macht
Haikus `BTab::Select()`/`Deselect()` (`src/kits/interface/TabView.cpp`)
beim Tab-Wechsel ein echtes `owner->AddChild(fView)` /
`fView->RemoveSelf()` - **nicht** nur Show/Hide. Das feuert bei jedem
Wechsel zwischen GraphEditor und NavigatorEditor
`GraphEditor::DetachedFromWindow()` (löscht *alle* Renderer über
`RemoveRenderer()`) und danach `AttachedToWindow()` → `InitAll()` (baut sie
komplett neu). Wer ein Verhalten testet, das nur beim allerersten Laden
auftritt, muss auch einen Tab-Wechsel gegenprüfen - der nimmt strukturell
denselben Codepfad wie ein Fenster-Neuaufbau, nicht wie ein einfaches
Redraw.

## `PEditorManager::BroadCast()` geht an ALLE Editoren, nicht nur den sichtbaren

`SetActivePEditor()`/`SetInactivePEditor()` (`PEditorManager.cpp`) pflegen
nur eine `activeEditor`-Liste fürs Archivieren des Fensterzustands -
`BroadCast()` selbst iteriert unabhängig davon über `editorMessenger`
(Messenger zu **allen** registrierten Editoren) und schickt jeden
`P_C_VALUE_CHANGED` an GraphEditor und NavigatorEditor gleichzeitig, egal
welcher Tab gerade sichtbar ist. Das hält beide Editoren synchron, auch
ohne dass ein Tab-Wechsel selbst etwas triggert (siehe nächster Abschnitt) -
aber wer Performance-Annahmen macht ("der inaktive Editor bekommt das doch
eh nicht mitgeteilt") liegt daneben.

## `std::set<BMessage*>`/`std::map` - Iterationsreihenfolge ist Zeigeradresse, nicht Anlage

Bereits in CLAUDE.md als Regel festgehalten, hier der konkrete Ort, an dem
es wiederholt zuschlägt: `PDocument::GetChangedNodes()` liefert ein
`std::set<BMessage*>`, sortiert nach Pointerwert. Jeder Code, der davon
ausgeht, dass darin "zuerst angelegte" Objekte auch zuerst auftauchen, ist
falsch - das gilt nur zufällig, abhängig von Heap-Adressen zur Laufzeit
(siehe `docs/notes.md`, Issue #46: Connections wurden vor ihren Endknoten
verarbeitet, blieben mit `from=to=NULL` hängen). Wo Reihenfolge zählt,
braucht es einen expliziten zweiten Pass (wie in `InitAll()` und
`GraphEditor::ValueChanged()`) - dieselbe Vorsicht gilt für den `Indexer`s
`map<int32, BMessage*> sorter`, auch wenn dort bisher kein konkreter Bug
daraus entstanden ist.

## `BShape`/`StrokeShape()` ignoriert `BView::SetScale()`

Live verifiziert (2026-08-15): Eine über `BShape`/`StrokeShape()` gezeichnete
Bézierkurve rendert bei jedem Zoom außer 100% an ihrer **unskalierten**
Originalposition/-größe, während jede andere Zeichenoperation im selben
View (`StrokeLine`, `FillRoundRect`, ...) korrekt mitskaliert. Betraf
`ConnectionRenderer::DrawBended()` (Connection-Typ "Bended"). Workaround
(kein Fix am App-Server möglich): Kurve stattdessen mit `PointOnBezier()`
in kurze `StrokeLine()`-Segmente zerlegen - das respektiert `SetScale()`
korrekt, weil es dieselbe Zeichen-Primitive wie alles andere nutzt. Wer neue
Bézier-artige Zeichenelemente einführt: `BShape` für alles vermeiden, das
bei Nicht-100%-Zoom sichtbar sein muss.

## `BView::SetScale()` ist multiplikativ über Push-Ebenen, nicht absolut

Aus Haikus eigener Doku (BShape.md-Nachbarschaft, `BView::SetScale()`):
zwei `SetScale()`-Aufrufe auf derselben Push-Ebene sind absolut (der zweite
gewinnt), aber ein `SetScale()` **nach** einem `PushState()` multipliziert
mit dem geerbten Wert der Elternebene. `GraphEditor::Draw()` und der
`G_E_NEW_SCALE`-Handler rufen `PushState()` mehrfach auf, **ohne** jemals
`PopState()` - im gesamten Code gibt es aktuell keinen einzigen
`PopState()`-Aufruf. Das ist ein echter, unbehobener Ressourcen-Leak
(jeder Redraw hinterlässt einen weiteren nie abgebauten Zustand auf dem
App-Server-Stack), aber ein naiver Fix (jedes `PushState()` einfach mit
`PopState()` paaren) ist **nicht sicher** - genau das wurde ausprobiert und
hat den effektiven Zoom quadriert (`scale²`), weil der bestehende Code
implizit davon ausgeht, in welcher Push-Tiefe er sich befindet. Ein
korrekter Fix braucht eine vollständige Bestandsaufnahme, welche
`SetScale()`-Aufrufe auf welcher Ebene wirken sollen - nicht angefasst,
bewusst so belassen (Stand 2026-08-15).

## `BInvoker`-Subklassen übernehmen Ownership der übergebenen `BMessage`

`TextEditorControl : public BTextView, public BInvoker` - wie jede
`BInvoker`-Subklasse löscht sie die ihr übergebene `BMessage` in ihrem
eigenen Destruktor (Haiku-Doku, `BInvoker::~BInvoker()`/`SetMessage()`).
Jeder Code, der **dieselbe** Message mehrfach an aufeinanderfolgende
Instanzen weitergibt (statt bei jeder Instanz eine frische Kopie zu bauen),
produziert beim zweiten Mal einen Use-after-free, beim Zerstören der
zweiten Instanz einen Double-free. War in `StringRenderer::MouseDown()` der
Fall (das `editMessage`-Template wird einmalig in `ClassRenderer::Init()`
gebaut und für jede Umbenennung wiederverwendet) - Fix: `new
BMessage(*changeMessage)` statt `changeMessage` direkt übergeben.
`AttributRenderer.cpp` macht es bereits richtig (eigene Kopie pro Instanz) -
als Referenzmuster für neue Editor-Controls verwenden.

## Connection-Endpunkte sind gecachte Pointer, nicht live aufgelöst

`ConnectionRenderer::from`/`to` werden einmalig in `ValueChanged()` über
`tmpNode->FindPointer(editor->RenderString(), ...)` aufgelöst und bleiben
bis zum nächsten Broadcast bestehen, der **genau diese** Connection anfasst.
Wird ein Knoten-Renderer gelöscht (Undo von Insert, Delete, ...), ohne dass
der auslösende Command jede daran hängende Connection ebenfalls als
"changed" markiert, bleibt der gecachte Pointer hängen -
`CalcLine()`/`Draw()` dereferenzieren dann freigegebenen Speicher.
`GraphEditor::RemoveRenderer()` ist die **einzige** Stelle, an der je ein
Renderer gelöscht wird, und ruft deshalb vor jedem `delete` proaktiv
`ConnectionRenderer::InvalidateEndpoint()` für jede Connection in der
Top-Level-Liste auf. Jeder neue Lösch-Pfad muss entweder über
`RemoveRenderer()` laufen oder dieselbe Invalidierung selbst nachbauen.

## `BRect`-Default ist `(0,0,-1,-1)` - Union damit korrumpiert stillschweigend

Eine frisch konstruierte `BRect` ist ungültig (`IsValid() == false`,
`(0,0,-1,-1)`). Wird sie ungeprüft mit `|` (Union) in ein Ergebnis
eingerechnet, kann das ein eigentlich korrektes Rechteck auf einen riesigen,
bei `(0,0)` verankerten Bereich aufblasen. Passiert in
`GroupRenderer::RecalcFrame()`, wenn die Gruppe (noch) keine registrierten
Kinder hat - z. B. direkt bei Konstruktion, bevor Kinder zugewiesen sind.
Fix: früher Return, wenn die aus den Kindern akkumulierte `groupFrame`
ungültig ist. Jeder Code, der ein `BRect` aus einer potenziell leeren
Collection aufbaut, sollte `IsValid()` prüfen, bevor er es weiterverwendet.

## Default-Parameter mit stillschweigender Wirkung

`GraphEditor::GenerateInsertCommand(uint32 newWhat, bool connected = false)` -
der Default ist leicht zu übersehen, wenn ein Aufrufer eigentlich eine
Verbindung zur aktuellen Selektion erwartet. Historisch wurde das lange
durch einen separaten Bug maskiert (die `if (connected)`-Prüfung um die
Verbindungslogik war auskommentiert, Verbindung wurde unabhängig vom Flag
immer erzeugt) - nach dessen Fix fiel auf, dass `G_E_INSERT_NODE`/
`G_E_INSERT_SIBLING` das Flag nie explizit gesetzt hatten. Bei Änderungen an
Funktionen mit Default-Parametern: alle Call-Sites prüfen, ob sie sich
(unbewusst) auf den Default verlassen.

## Zwei Build-Systeme, nur eines ist vollständig

Siehe CLAUDE.md - hier nur als Erinnerung im Code-Kontext: das `Jamfile`
bindet `src/translators/` nicht ein. Eine Änderung, die nur gegen das
Jamfile getestet wurde, ist ungetestet für den translators-Pfad.
