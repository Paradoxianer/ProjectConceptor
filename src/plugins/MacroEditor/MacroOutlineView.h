#ifndef MACRO_OUTLINE_VIEW_H
#define MACRO_OUTLINE_VIEW_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <app/Message.h>
#include <interface/OutlineListView.h>
#include <interface/TextControl.h>
#include <support/List.h>
#include <support/String.h>

#include <set>
#include <utility>
#include <vector>

class MacroEditor;
class PCommand;
class PCommandManager;

/** Marker field on the drag message CommandReferenceListView builds - see
 * MacroOutlineView::MessageReceived(). Same marker MacroTextView used to
 * use, kept so CommandReferenceListView (unchanged) doesn't need to know
 * which editor view is on the receiving end. */
extern const char* const kCommandSnippetDragMarker;
/** Marker on the drag message InitiateDrag() builds for an existing
 * command row being repositioned (as opposed to a brand new one coming in
 * from the reference sidebar) - see MoveCommandRow(). */
extern const char* const kCommandMoveDragMarker;
/** Marker on the drag message InitiateDrag() builds for a data chip (an
 * included_node/included_connection's own "this=N" id) being dragged onto
 * another command to reference it - see AddNodeReference(). The chip
 * itself never moves or gets removed, only a new "node=@N" field is added
 * to whatever it's dropped on. */
extern const char* const kNodeReferenceDragMarker;

/** One {field name, repeat index} step locating a BMessage inside another -
 * see MacroOutlineView.cpp's path-based read/write/delete. A row's own
 * "container path" is a sequence of these from the selected macro's Nth
 * top-level command down to whichever BMessage directly holds that row's
 * data (a command's own settings, or a nested block's own content). */
struct MacroPathStep
{
	BString	field;
	int32	index;
};
typedef std::vector<MacroPathStep>	MacroPath;

/** Where a drop/move lands, resolved once from the row under the pointer -
 * shared by dropping a new command in from the reference sidebar and
 * moving an existing command row (see MacroOutlineView.cpp's
 * ResolveTargetPosition()/InsertCommandAt()). `valid` false means "no row
 * there, insert as a new top-level command". `containerPath`+`fieldName`+
 * `fieldIndex` locate the TARGET row itself (its own container, and which
 * slot in it), same convention as MacroRowItem. */
struct MacroTargetPosition
{
	bool		valid;
	bool		isCommandRow;
	bool		isTopLevelCommand;
	int32		topLevel;
	MacroPath	containerPath;
	BString		fieldName;
	int32		fieldIndex;
	bool		lowerHalf;
};

/**
 * @class MacroOutlineView
 *
 * @brief Direct structural view/editor of one macro's command list -
 * replaces the old text+chip editor (MacroTextView) entirely. A
 * BOutlineListView showing exactly four row kinds (see MacroRowItem::Kind):
 * a command, one of its scalar fields, an embedded node/connection ("data
 * chip"), or any other nested field ("generic block") - each with its own
 * fixed visual language and its own narrow editing affordance, so there is
 * no free-text surface left for a stray keystroke to corrupt.
 *
 * There is no draft/Apply step: every edit (a field's inline overlay
 * committing, a dropped command snippet, add/delete/move-a-row) is written
 * straight back into the working copy of the selected macro's command list
 * (fCommands) via the path mechanism in the .cpp, and MacroEditor is told
 * to persist that into the real macro's "Macro::Commmand" list right away -
 * every state fWorkingCommands is ever in is a fully valid, already-typed
 * BMessage tree, so there is nothing left to validate or lose on a macro
 * switch.
 */
class MacroOutlineView : public BOutlineListView
{
public:
							MacroOutlineView(BRect frame, const char *name,
								uint32 resizingMode);
	virtual	void			MessageReceived(BMessage *message);
	virtual	void			MouseDown(BPoint where);
	virtual	void			KeyDown(const char *bytes, int32 numBytes);
	virtual	void			SelectionChanged(void);
	/** Refuses a command-snippet drop for BOutlineListView's own list-
	 * reordering drag handling - MessageReceived() places it itself. */
	virtual	bool			InitiateDrag(BPoint where, int32 index, bool wasSelected);

			void			SetEditor(MacroEditor *editor) {fEditor = editor;};
			/** A real PCommandManager for tests to exercise schema-dependent
			 * behavior against (AddNodeReference()'s refusal, the add-field
			 * menu's contents, ...), where a real MacroEditor/PDocument
			 * doesn't exist to reach one through - see Registry(). Not used
			 * outside tests; normal operation always goes through fEditor. */
			void			SetRegistryForTests(PCommandManager *registry) {fTestRegistry = registry;};

			/** Rebuilds every row from `commands` (BList of BMessage*, the
			 * same shape as a macro's "Macro::Commmand" entries) - takes
			 * ownership of a deep copy, not of `commands` itself. Call only
			 * when switching to a genuinely different macro (or reloading
			 * after an outside change), not after this view's own edits -
			 * those already keep fCommands and the displayed rows in step
			 * without a full rebuild. */
			void			SetCommands(BList *commands);

			/** The current, fully up to date command list - what
			 * MacroEditor::ShowSelectedMacro()/the Save path should write
			 * into the macro's "Macro::Commmand" field. Caller does not
			 * own the BMessage* entries. */
			BList*			Commands(void) {return &fCommands;};

			/** The row index of the command at `topLevel`/`selfPath` (its
			 * own SelfPath() - empty for a top-level command), -1 if none
			 * matches. Exposed mainly so tests can find a target row
			 * without reaching into MacroRowItem (private to the .cpp) -
			 * see MacroTextTest.cpp's MoveCommandRow() coverage. */
			int32			RowIndexForCommand(int32 topLevel, const MacroPath &selfPath);

private:
			/** BOutlineListView::AddUnder() appends right after `superitem`,
			 * not after its last existing child - see the .cpp for why this
			 * exists instead. */
			void			AppendUnder(class MacroRowItem *item, class MacroRowItem *superitem);
			/** Ends any open inline value-edit overlay, committing it
			 * first unless `commit` is false (Escape). */
			void			CloseOverlay(bool commit);
			/** Opens the right control for `item`'s field over its own row
			 * - BTextControl for everything but bool, which just toggles. */
			void			BeginOverlayEdit(int32 rowIndex);
			void			ToggleBoolField(int32 rowIndex);
			/** Rebuilds every row from fCommands - cheap enough to call
			 * after every single edit (a macro has at most a few dozen
			 * rows) and far simpler than surgically patching one subtree
			 * in place. Preserves which data-chip/generic-block rows were
			 * expanded (by a path-based key, not by BListItem identity -
			 * every rebuild replaces every item), so editing a field
			 * inside an expanded node chip doesn't re-collapse it. */
			void			RebuildAllRows(void);
			/** Adds one row per field/subcommand/nested block of `container`
			 * under `superitem`, recursing into every nested block - see
			 * MacroOutlineView.cpp's class comment on the four row kinds.
			 * `schemaCommand` is the PCommand owning `container`'s own
			 * PropertyInfo() (NULL inside a nested block, which has none -
			 * see MacroText.h), used for the bound-field/add-field-row
			 * decisions. `expandedKeys` is this rebuild's snapshot of which
			 * chip/block rows were open before it started (see
			 * RebuildAllRows()). */
			void			BuildChildren(BMessage *container, int32 topLevelIndex,
								const MacroPath &containerPath, class MacroRowItem *superitem,
								uint32 level, PCommand *schemaCommand,
								const std::set<BString> &expandedKeys);
			/** Shows the "+ Feld hinzufügen" popup for the command/block at
			 * `containerPath` (topLevelIndex + path). `command` NULL (a
			 * generic block/chip, no schema) opens ShowFreeformAddFieldMenu()
			 * instead of the schema-driven list below. */
			void			ShowAddFieldMenu(BPoint screenWhere, int32 topLevelIndex,
								const MacroPath &path, PCommand *command);
			/** A generic block/chip has no schema to offer field names
			 * from (see MacroText.h) - offers a fixed type choice instead
			 * (every type FormatFieldValue()/ParseFieldValue() round-trip),
			 * the field's own name typed in afterward (see
			 * MessageReceived()'s 'mvFT' case). Reached only via
			 * ShowAddFieldMenu()'s `command == NULL` branch. */
			void			ShowFreeformAddFieldMenu(BPoint screenWhere, int32 topLevelIndex,
								const MacroPath &path);
			/** Adds a fresh `field` of `type` (a per-type zero value, or -
			 * for "field"=="included_node" - a renumbered node prototype,
			 * see BuildInsertPrototype()) to the container at
			 * `topLevel`/`path`. Shared by the schema-driven 'mvAF' menu
			 * items and the freeform 'mvFT' flow's typed-in name. */
			void			AddNamedField(int32 topLevel, const MacroPath &path,
								const char *field, type_code type);
			/** Adds a new "node"=@nodeId reference field to the command at
			 * `topLevel`/`selfPath` (its own SelfPath() - empty for a
			 * top-level command) - a data chip dragged onto that command
			 * (see InitiateDrag()/kNodeReferenceDragMarker), so e.g.
			 * dragging Insert's own node chip onto a Select adds a
			 * "node=@<that id>" field to it, wiring the two together
			 * without typing the id by hand. Refuses (beeps) if the
			 * command's own schema doesn't declare a "node" field at all -
			 * nothing to wire up there. */
			void			AddNodeReference(int32 topLevel, const MacroPath &selfPath, int32 nodeId);
			/** Same drag, dropped directly onto an EXISTING field row
			 * instead of the command's header - replaces that one field
			 * instance's value in place (same remove-then-add idiom
			 * CloseOverlay() uses for a normal edit) rather than adding a
			 * new field next to it. No schema check needed here: the field
			 * already passed AddNodeReference()'s check (or the add-field
			 * menu's own type filter) to exist at all. */
			void			ReplaceNodeReferenceField(int32 topLevel, const MacroPath &containerPath,
								const char *fieldName, int32 fieldIndex, int32 nodeId);
			/** Deletes the field/chip/block/subcommand/command row at
			 * `rowIndex` (or, if it's a top-level command, that whole
			 * macro entry). */
			void			DeleteRow(int32 rowIndex);
			/** Deletes every currently selected row (Delete key, or the
			 * context menu's Delete with more than one row selected) in
			 * one pass - a row nested inside another selected row is
			 * skipped (removing the ancestor already removes it). Move Up/
			 * Move Down stay single-row only (the context menu disables
			 * them outright once more than one row is selected) - the
			 * context menu's own Move Up/Move Down, not dragging (see
			 * MoveCommandRows(), which does support moving several rows
			 * together). */
			void			DeleteSelectedRows(void);
			/** The actual removal DeleteRow()/DeleteSelectedRows() share -
			 * see DeleteSelectedRows()'s own comment on why it doesn't
			 * just call DeleteRow() once per row. */
			void			DeleteIdentity(int32 topLevel, const MacroPath &containerPath,
								const BString &fieldName, int32 fieldIndex, bool isTopLevelCommand);
			void			MoveRow(int32 rowIndex, int32 direction);
			/** Parses `snippet` (a DSL example string, see
			 * CommandExampleText()/BuildCommandSnippet()) into a real
			 * command BMessage and inserts it via InsertCommandAt(). */
			void			DropCommandSnippet(int32 targetRow, BPoint where,
								const char *snippet, int32 length);
			/** Moves the existing command at `sourceTopLevel`/`sourcePath`
			 * (its own SelfPath(), empty = a top-level command) to wherever
			 * `targetRow` resolves to - a sibling of it (or, dropped onto a
			 * container command, that command's LAST subcommand), also
			 * reachable by dropping below every row to move it out to the
			 * macro's own top level. Refuses (beeps) a drop onto the
			 * command's own current position or into one of its own
			 * subcommands - that would either do nothing or corrupt the
			 * tree by nesting a command inside itself. A thin single-source
			 * wrapper around MoveCommandRows() - InitiateDrag() only ever
			 * calls this one directly when just one command is selected. */
			void			MoveCommandRow(int32 sourceTopLevel, const MacroPath &sourcePath,
								int32 targetRow, BPoint where);
			/** MoveCommandRow(), for several commands (every one InitiateDrag()
			 * found selected) dragged and dropped together in one go - they
			 * land at the target in their own original relative order. A
			 * source nested inside another one being moved is skipped
			 * (moving the ancestor already carries it along); the drop is
			 * refused (beeps) if it would land inside any moved source's own
			 * subtree, same as the single-source guard. */
			void			MoveCommandRows(std::vector<std::pair<int32,MacroPath> > &sources,
								int32 targetRow, BPoint where);
			/** Inserts `command` (caller transfers ownership) at `pos` - a
			 * single-item wrapper around InsertCommandsAt(). */
			void			InsertCommandAt(BMessage *command, const MacroTargetPosition &pos);
			/** Inserts `commands`, in the order given, as one contiguous
			 * block at `pos` - shared insertion-position logic for a
			 * freshly parsed drop (DropCommandSnippet(), always one
			 * command), and one or several existing rows being moved
			 * (MoveCommandRow()/MoveCommandRows(), called only once `pos`
			 * already accounts for every source's own removal shifting
			 * other indices - see their own comments). */
			void			InsertCommandsAt(std::vector<BMessage> &commands, const MacroTargetPosition &pos);
			/** fTestRegistry if a test set one (see SetRegistryForTests()),
			 * else fEditor->CommandManagerForOutline() - every command-
			 * schema lookup in this file goes through this, not fEditor
			 * directly. */
			PCommandManager*	Registry(void);

			MacroEditor		*fEditor;
			/** Working copy of the selected macro's "Macro::Commmand" list -
			 * owns every BMessage* in it. */
			BList			fCommands;
			/** See SetRegistryForTests(). NULL outside tests. */
			PCommandManager	*fTestRegistry;

			BTextControl	*fOverlay;
			/** Row the open overlay belongs to, -1 if none is open. */
			int32			fOverlayRow;
};

#endif
