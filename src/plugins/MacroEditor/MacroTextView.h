#ifndef MACRO_TEXT_VIEW_H
#define MACRO_TEXT_VIEW_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <interface/Font.h>
#include <interface/GraphicsDefs.h>
#include <interface/TextView.h>
#include <support/String.h>

#include <map>

class MacroEditor;

/** Marker field on the drag message CommandReferenceListView builds - see
 * MacroTextView::MessageReceived(). */
const char* const kCommandSnippetDragMarker	= "pc:command_snippet";

/**
 * @class MacroTextView
 *
 * @brief The macro DSL text field (#55 follow-up) - no separate Apply
 * button. Edits commit themselves at natural pause points instead: every
 * time the user finishes a line (Enter) or leaves the field (e.g.
 * switching to another macro or tab), the current text is auto-applied
 * via MacroEditor::ApplyEdits() - silently on success, leaving the last
 * good version in place and reporting the error in the status line on
 * failure, never blocking further typing.
 *
 * Every "~included_node" block (a node/connection's full embedded data -
 * see MacroText.h) starts folded to a single placeholder line, since that
 * content is almost always rendering detail (font, pattern, position) no
 * one is there to edit, not the graph-editing logic a macro is actually
 * about. Double-clicking a placeholder expands it in place; double-
 * clicking an expanded block's own "~included_node" header line folds it
 * back, re-capturing whatever was edited while it was open. Folding is
 * pure presentation on top of MacroText.cpp's own serialize/parse - the
 * canonical text (what actually gets applied) is always recoverable via
 * ExpandedText(), regardless of what's currently folded on screen.
 */
class MacroTextView : public BTextView
{
public:
							MacroTextView(BRect frame, const char *name,
								BRect textRect, uint32 resizingMode, uint32 flags);
	virtual	void			KeyDown(const char *bytes, int32 numBytes);
	virtual	void			MakeFocus(bool focused = true);
	virtual	void			MouseDown(BPoint where);
	virtual	void			MessageReceived(BMessage *message);
	/** Refuses a command-snippet drop for BTextView's own insert-at-the-
	 * mouse-position handling - MessageReceived() places it itself. */
	virtual	bool			AcceptsDrop(const BMessage *message);
	/** BTextView's own internal mechanics (click, arrow-key navigation,
	 * typing, drag&drop, ...) all funnel through this - the one reliable
	 * hook for "the cursor/selection just changed", used to keep
	 * MacroEditor's line/column status display current (#55 follow-up,
	 * user report). */
	virtual	void			Select(int32 startOffset, int32 endOffset);

			void			SetEditor(MacroEditor *editor) {fEditor = editor;};

			/** Replaces the view's content with `canonicalText` (the plain
			 * output of SerializeCommands()), folding every
			 * "~included_node" block it finds to a placeholder line.
			 * Resets all previously remembered fold content - call this
			 * only when switching to a genuinely different macro, not on
			 * every keystroke. */
			void			SetMacroText(const BString &canonicalText);

			/** The view's current text with every remaining folded
			 * placeholder expanded back to its real content - what
			 * MacroEditor::ApplyEdits() should hand to ParseCommands(),
			 * never Text() directly. */
			void			ExpandedText(BString *out);

			/** True (with a human-readable count in *outWarning) if the
			 * view currently has fewer "~included_node" blocks - folded or
			 * expanded - than it had right after the last SetMacroText().
			 * A block's placeholder line looks like any other line and is
			 * just as easy to select and delete without meaning to; this
			 * catches that (or a legitimate whole-command deletion that
			 * happened to remove one) so ApplyEdits() can at least warn
			 * instead of silently storing a macro missing a node's data. */
			bool			LostFoldedBlocks(BString *outWarning);

			/** Selects and scrolls to the line at `canonicalLineNo` (1-based,
			 * as ParseCommands()'s "line N: ..." errors count them - against
			 * ExpandedText(), not what's on screen). Expands whichever
			 * folded block currently contains that line first, if any - a
			 * folded chip collapses many canonical lines into one, so the
			 * error's own line number doesn't point anywhere meaningful in
			 * the *displayed* text until that's undone. False if
			 * canonicalLineNo is out of range. */
			bool			RevealCanonicalLine(int32 canonicalLineNo);

	/** Moves the lines touched by the selection (or the caret's line) `levels`
	 * indent levels in (positive) or out (negative) - Tab / Shift+Tab. Turns
	 * a block of commands into subcommands of the line above and back. Only
	 * leading whitespace is touched, so a folded chip keeps its styling. */
			void			ShiftSelectedLines(int32 levels);

private:
			/** Inserts a dragged-in command snippet on a line boundary, not
			 * where the mouse happens to be: BTextView's own drop inserts at
			 * the exact character under the pointer, so dropping onto the
			 * word "Find" split it in two and broke that command (user
			 * report). Upper half of the target line -> before it; lower
			 * half -> after it and everything nested deeper below it. The
			 * snippet is indented to the target line's own depth. */
			void			DropSnippet(BPoint where, const char *text, int32 length);
			MacroEditor		*fEditor;
			/** This view's own starting font/color, captured once at
			 * construction - what a folded placeholder line gets styled
			 * back to once expanded, so an expanded block never keeps
			 * looking like a chip. */
			BFont			fDefaultFont;
			rgb_color		fDefaultColor;
			/** Full text of each folded-away "~included_node" block, keyed
			 * by the SAME node/connection reference id the rest of the DSL
			 * uses ("node=@1", "..."). Every embedded block carries its own
			 * id as a "this=N" field (Indexer::IndexNode()/IndexConnection()
			 * always add it) - reusing it here, shown right in the
			 * placeholder as "[@N]", is what actually answers "is this the
			 * same node as that @1 reference over there" instead of forcing
			 * the reader to guess from proximity. A block that's somehow
			 * missing "this=N" (hand-edited into that state) falls back to
			 * a negative synthetic key from fNextSyntheticId, shown as
			 * "[#N]" - never collides with a real (non-negative) id, so the
			 * lookup this map exists for stays unambiguous either way. */
			std::map<int32,BString>	fFoldedBlockText;
			/** Next negative key to hand out for a block with no "this=N"
			 * of its own - see fFoldedBlockText. */
			int32			fNextSyntheticId;
			/** How many "~included_node" blocks SetMacroText() found -
			 * see LostFoldedBlocks(). */
			int32			fOriginalBlockCount;

			/** Count of lines in the current Text() that are either a
			 * folded placeholder or a real expanded "~included_node"
			 * header - used by LostFoldedBlocks(). */
			int32			CurrentBlockCount(void);

			/** Finds the "~included_node" (or numbered placeholder) block
			 * starting at `lineStart`/ending at `lineEnd` (that single
			 * line's own range) and folds or expands it in place. No-op if
			 * that line isn't one of the two recognized forms. */
			void			ToggleFoldAtLine(int32 lineStart, int32 lineEnd);
			/** Key for a block about to be folded: its own this=N unless
			 * another folded block already holds that key, else a fresh
			 * synthetic one. */
			int32			AllocateFoldKey(int32 thisId);
			/** Carries an edited chip label ("NewName") over into the
			 * folded block's own name field. */
			void			ApplyChipLabelEdit(BString *blockText,
								const BString &placeholderTrimmed);
			/** Styles [start,end) as a folded chip (italic, muted color) -
			 * or, via ClearFoldStyle(), back to this view's own starting
			 * font/color for freshly expanded content. */
			void			StyleAsFoldedChip(int32 start, int32 end);
			void			ClearFoldStyle(int32 start, int32 end);
			/** Bolds every bare command-name line ("Select", "Group", ...)
			 * in the current text, so a subPCommand visibly stands out from
			 * the "fieldName=value"/"~fieldName" data lines sitting at the
			 * very same indentation depth - see IsCommandLine(). Run once,
			 * right after SetMacroText() builds the initial folded text;
			 * expanding/folding a "~included_node" chip never adds or
			 * removes a command line (that content is always pure node/
			 * connection data), so nothing else needs to re-run this. */
			void			StyleCommandLines(void);
};

#endif
