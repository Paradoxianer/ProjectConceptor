#ifndef MACRO_TEXT_VIEW_H
#define MACRO_TEXT_VIEW_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <interface/TextView.h>

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
 */
class MacroTextView : public BTextView
{
public:
							MacroTextView(BRect frame, const char *name,
								BRect textRect, uint32 resizingMode, uint32 flags);
	virtual	void			KeyDown(const char *bytes, int32 numBytes);
	virtual	void			MakeFocus(bool focused = true);

			void			SetEditor(MacroEditor *editor) {fEditor = editor;};

private:
			MacroEditor		*fEditor;
};

#endif
