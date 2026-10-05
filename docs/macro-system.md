# The macro system - developer notes

How macros are recorded, stored, edited and played back, and how to add
a command. For using macros, see the [user guide](help/macros.md).

## Overview

| Part | Where | Job |
|---|---|---|
| Commands | `src/plugins/*Command/` | One `PCommand` plugin per command; its `PropertyInfo()` is the schema. |
| Command manager | `src/app/Commands/PCommandManager.*` | Executes commands, records macros, plays them, owns the value context. |
| Indexer | `src/app/Tools/Indexer.*` | Turns live node pointers into ids (recording) and back (playback). |
| Macro text | `src/plugins/MacroEditor/MacroText.*` | The text format: `SerializeCommands()` / `ParseCommands()`, schema lookup. |
| Macro editor | `src/plugins/MacroEditor/MacroEditor.*`, `MacroOutlineView.*` | The MacroEditor tab: macro list, tree, command reference. |
| Document | `src/app/Document/PDocument.cpp` | Routes Play/Step messages, scripting suite (`hey`). |

## Data model

A **macro** is a `BMessage` with `what == P_C_MACRO_TYPE`, a `"Name"`
string and one `"Macro::Commmand"` message per top-level command (the
triple m is historical; it's the stored field name). Macros live in
`PCommandManager::macroList` and are archived with the document as
repeated `"macro"` fields.

A **command** is a settings `BMessage`:

- `"Command::Name"` - which `PCommand` runs it.
- its own fields, as declared in the command's `PropertyInfo()`.
- `"PCommand::subPCommand"` - nested commands (Batch, Repeat, ForEach,
  If), each again a command message.
- `"PCommand::bindings"` - field name -> variable name, for `$name`
  fields (see [Bindings](#bindings)).

Nodes and connections inside a stored macro are never pointers. The
`"node"` field holds int32 ids, and the node content travels along as
`"included_node"` messages carrying their own `"this"` id. A connection's
`Node::from`/`Node::to` are ids of nodes in the same macro.

## Recording

`PCommandManager::StartMacro()` creates the recording and a dedicated
`Indexer`; `StopMacro()` asks for a name and appends it to `macroList`.
In between, `Execute()` appends every executed, non-`shadow` command via
`Indexer::IndexMacroCommand()`, which replaces each `"node"` pointer by
an id and embeds the node (`IndexNode()`) or connection
(`IndexConnection()`) the first time it's seen.

Before that, `NormalizeToSelection()` (#132) rewrites ChangeValue,
AddAttribute, RemoveAttribute, Copy and Move: if the command's explicit
`"node"` pointers are exactly the selection from before `Do()`, they are
replaced by `Node::selected=true`, so direct manipulation records the
same portable form as the toolbar.

## Playback

Two entry points share the per-command work:

- **`PlayMacro()`** - synchronous, returns only when everything ran.
  Used by tests and programmatic callers.
- **`PlayMacroInteractive()` / `PlayMacroStep()`** - used by
  **Macro > Play** (`P_C_MACRO_TYPE` in `PDocument`) and the macro
  shortcuts (`P_C_PLAY_MACRO_BY_NAME`). Every top-level command runs in
  its own `P_C_MACRO_PLAY_STEP` message the document posts to itself.
  `BLooper::DispatchMessage()` keeps the document locked for a whole
  `MessageReceived()` call, so this is what lets the editors redraw
  between commands. Only one interactive playback runs at a time.

For each top-level command:

1. `Indexer::DeIndexCommand()` - registers every `included_node` under
   its `"this"` id (nodes first, then connections, which need their
   endpoints), and turns `"node"` ids back into pointers. It recurses
   into `"PCommand::subPCommand"`, so **a loop body is resolved once**,
   before the loop runs.
2. `Execute()` - locks the document, `ResolveBindings()`,
   `InterpolateStrings()`, the command's `Do()`, appends the undo entry,
   unlocks and broadcasts the changed nodes.
3. Container commands run their children through
   `PCommand::RunSubCommandsOnce()`, which reads a **fresh copy** of each
   child from the unchanged template every pass and resolves its
   bindings and placeholders again.

Because the loop body is a template, `Insert::Do()` never inserts the
same node object twice: a node already in the document is cloned, and a
node whose text contains `${...}` is inserted as a filled-in copy
(`ReplaceInterpolatedNodes()`); `RepointReplayNode()` makes later
top-level `@id` references reach the copy, and a connection in the same
Insert is copied to follow its copied endpoints.

`FinishMacroPlayback()` builds the result text - empty macro, failed
command, unresolved node references, then playback errors, else
"Played N command(s)." - broadcasts `P_C_MACRO_PLAYED` and shows an
alert on failure.

## Value context

`PCommandManager::valueContext` is a flat `BMessage` that exists only
during one playback (`GetValueContext()` is `NULL` otherwise). Commands
write into it: Repeat (counter, int32), ForEach (node, pointer), Insert
(`resultVariable`, pointers), Remember (selection, pointers), Calculate
(float), GetValue (float, string or bool), Ask (string). A command that
writes must remove the old value first, so a loop pass never sees the
previous pass's value.

## Bindings

`fieldName=$variable` is stored as an entry in `"PCommand::bindings"`,
never in the field itself. `ResolveBindings()` runs right before each
`Do()` and fills the field from the value context:

- declared float <- int32 (loop counters), declared int32 <- float;
- declared float/int32 <- string: parsed; a single comma without a dot
  is a decimal comma; anything else is a playback error;
- declared pointer (`node`) <- int32/float: an id resolved through the
  playing macro's `Indexer` (computed ids, e.g. from Calculate);
- otherwise the value is copied as it is (pointers from Insert/ForEach).

Bindings only apply to a command's own top-level fields; the parser
rejects `$` inside a `~` block.

## Text placeholders

`InterpolateStrings()` replaces `${name}` in every string field of the
settings - nested messages included, `"PCommand::subPCommand"` and
`"PCommand::bindings"` skipped - using `FormatVariable()`: strings as
they are, whole numbers without decimals, other numbers via `%g`, bools
as true/false. `$${` is a literal `${`. Pointers and multi-valued
variables are errors. Like bindings it is a no-op outside playback.

## Errors

A command that can't do what its settings say reports through
`AddPlaybackError()` instead of substituting a value. The count and the
first message end up in the playback result. Outside playback it only
logs. Never fall back to a default silently - report it.

## The text format

`MacroText.cpp` converts between a list of command messages and the
text format described in the user guide. `ParseCommands()` checks
every command name against the registry and every field against the
command's schema (`FindFieldType()` - up to 3 `ctypes` entries with 5
fields each). Content of `~` blocks has no schema and is accepted as
written. Types without a text form round-trip as `raw:<type>:<base64>`.
`CommandExampleText()` holds the example snippet shown for each
command when it's dragged into the tree.

The text format is the file format of **Macro > Open/Save** and of the
fixtures in `docs/fixtures/`; the editor itself works on the messages
directly.

## The editor

`MacroEditor` owns the macro list, the command reference
(`CommandReferenceListView`, drag source) and the Macro menu actions.
`MacroOutlineView` shows one macro as a tree. Every row knows where its
data lives - top-level index, a path of `{field, index}` steps, field
name and index - so an edit writes straight into the command messages
(`WriteContainer()`), the tree is rebuilt (`RebuildAllRows()`, keeping
expanded/collapsed state by path) and `MacroEditor::CommitOutlineChange()`
copies the commands back into the macro.

`M_E_IMPORT_MACRO_FILE` (`'meIF'`, field `path`) imports a file without
the file panel, for scripting:

```
hey application/x-vnd.ProjectConceptor let View 'MacroEditor' of Window '[0]' do 'meIF' with path="/boot/home/m.txt"
```

## Interaction with GraphEditor

Every `Execute()` broadcasts the changed nodes. GraphEditor queues them
and processes the queue on its 20 ms animation tick, taking the document
lock with `LockWithTimeout(0)` and retrying on the next tick when it's
busy. It must never block there: while a macro holds the document, a
blocked window thread stops reading its port, the tick runner fills it,
and the next broadcast blocks forever (#142).

## Adding a command

1. Copy `src/plugins/CalculateCommand/` to `src/plugins/<Name>Command/`
   and rename the files, classes, `NAME` in the makefile and the target
   in the Jamfile. Add `localestub` to `LIBS` if you use `B_TRANSLATE`.
2. Declare the fields in `PropertyInfo()` - name and type per field,
   at most 5 per `ctypes` entry, and a usage text (it's the tooltip in
   the command reference). A `"node"` field is `B_POINTER_TYPE`; also
   declare `"included_node"` (`B_MESSAGE_TYPE`) then, so recorded
   macros with embedded nodes parse.
3. `Do()` reads its fields, changes the document, stores what `Undo()`
   needs in the settings, and returns `PCommand::Do(doc,settings)`.
   Report problems with `manager->AddPlaybackError()`. Mind that
   `BMessage::Find*()` overwrites the target even when a field is
   missing - keep a default only on `B_OK`.
4. Add the directory to `src/plugins/makefile` and `src/plugins/Jamfile`.
5. Add an example to `CommandExampleText()` in `MacroText.cpp`
   (`EveryCommandExampleParses` checks it).
6. Tests: add the source to `SRCS` in `src/tests/makefile`, register it
   with `TEST_PLUGIN(...)` and in `NewRegisteredTestDocument()` in
   `MacroTextTest.cpp`, and play a macro through `PlayMacroText()`.

A command that runs other commands takes `"PCommand::subPCommand"`
children and runs them with `RunSubCommandsOnce()`; add its name to
`kContainerCommandNames` in `MacroOutlineView.cpp` so commands can be
dropped into it.

## Tests

- `src/tests/MacroTextTest.cpp` - text format, tree editor, playback,
  bindings, placeholders, the newer commands.
- `src/tests/PCommandTest.cpp` - individual commands, Repeat/ForEach/If.
- `./dev.sh smoke` - its last stage imports
  `docs/fixtures/smoke-macro-three-nodes.txt` and plays it.

## Localization

The MacroEditor plugin has its own catalog
(`src/plugins/MacroEditor/locales/`); playback reports and errors are
in the app catalog (context `CommandManager`). Catalogs are compiled in
by `make bindcatalogs`, which `./dev.sh build` doesn't run - development
builds are always English, and the smoke test relies on English menu
names. After changing strings, regenerate `en.catkeys` with
`make catkeys` (in the plugin or app directory, on Haiku).

## The HTML help

`docs/help/macros.html` is generated from `macros.md` with
Python-Markdown (extensions `tables`, `fenced_code`, `toc`) and the
styles of `user-guide.html`; regenerate it after editing the Markdown.

## Known limitations

- Connections inserted inside a loop aren't re-created per pass (#139).
- Inside a loop, a literal `@id` refers to the template node, not the
  pass's copy; use `resultVariable`.
- No redraw within a single long command (a Repeat runs as one step).
- Copy, Delete, Paste and Resize have no schema, so their fields can't
  be edited or written in a file.
- Parser error messages are English and not translatable.
