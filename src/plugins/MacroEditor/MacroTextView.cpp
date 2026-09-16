#include "MacroTextView.h"

#include <interface/InterfaceDefs.h>

#include "MacroEditor.h"

MacroTextView::MacroTextView(BRect frame, const char *name, BRect textRect,
	uint32 resizingMode, uint32 flags)
	:BTextView(frame,name,textRect,resizingMode,flags)
{
	fEditor	= NULL;
}


void MacroTextView::KeyDown(const char *bytes, int32 numBytes)
{
	BTextView::KeyDown(bytes,numBytes);
	// B_RETURN and B_ENTER are the same byte (0x0a) in Haiku - covers both
	// the main Return key and the numpad Enter key uniformly.
	if ((fEditor != NULL) && (numBytes == 1) && (bytes[0] == B_ENTER))
		fEditor->ApplyEdits();
}


void MacroTextView::MakeFocus(bool focused)
{
	bool	wasFocused	= IsFocus();
	BTextView::MakeFocus(focused);
	// losing focus (switching macros, tabs, or clicking elsewhere) is the
	// other natural pause point - without this, a last line typed but
	// never terminated with Enter would silently be lost the moment
	// ShowSelectedMacro() overwrites this view's text with the last
	// applied version.
	if (wasFocused && (!focused) && (fEditor != NULL))
		fEditor->ApplyEdits();
}
