#ifndef MACRO_TEXT_VIEW_H
#define MACRO_TEXT_VIEW_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <interface/TextView.h>
#include <support/String.h>

#include <vector>

class MacroEditor;

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

private:
			MacroEditor		*fEditor;
			/** Full text of each folded-away "~included_node" block,
			 * indexed by the id embedded in its placeholder line
			 * ("~included_node[N] ..."). Re-folding a block that was
			 * edited while expanded appends a fresh entry rather than
			 * reusing its old index - simpler than tracking whether the
			 * old entry is still accurate, and the list only lives for as
			 * long as one macro is shown. */
			std::vector<BString>	fFoldedBlockText;

			/** Finds the "~included_node" (or numbered placeholder) block
			 * starting at `lineStart`/ending at `lineEnd` (that single
			 * line's own range) and folds or expands it in place. No-op if
			 * that line isn't one of the two recognized forms. */
			void			ToggleFoldAtLine(int32 lineStart, int32 lineEnd);
};

#endif
