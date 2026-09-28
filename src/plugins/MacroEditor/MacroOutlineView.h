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
#include <vector>

class MacroEditor;
class PCommand;
class PCommandManager;

/** Marker field on the drag message CommandReferenceListView builds - see
 * MacroOutlineView::MessageReceived(). Same marker MacroTextView used to
 * use, kept so CommandReferenceListView (unchanged) doesn't need to know
 * which editor view is on the receiving end. */
extern const char* const kCommandSnippetDragMarker;

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
			 * `containerPath` (topLevelIndex + path), offering whichever of
			 * its schema's fields aren't already exhausted - a repeatable
			 * one (e.g. Select's "node") stays offered after being added. */
			void			ShowAddFieldMenu(BPoint screenWhere, int32 topLevelIndex,
								const MacroPath &path, PCommand *command);
			/** Deletes the field/chip/block/subcommand row at `rowIndex`. */
			void			DeleteRow(int32 rowIndex);
			void			MoveRow(int32 rowIndex, int32 direction);
			/** Parses `snippet` (a DSL example string, see
			 * CommandExampleText()/BuildCommandSnippet()) into a real
			 * command BMessage and inserts it as a sibling of (or, dropped
			 * onto a container command, a subcommand of) the row at
			 * `targetRow`. */
			void			DropCommandSnippet(int32 targetRow, BPoint where,
								const char *snippet, int32 length);

			MacroEditor		*fEditor;
			/** Working copy of the selected macro's "Macro::Commmand" list -
			 * owns every BMessage* in it. */
			BList			fCommands;

			BTextControl	*fOverlay;
			/** Row the open overlay belongs to, -1 if none is open. */
			int32			fOverlayRow;
};

#endif
