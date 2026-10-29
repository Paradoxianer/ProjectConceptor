#include "StatusField.h"

#include <interface/ControlLook.h>
#include <support/Beep.h>

#include <math.h>
#include <stdlib.h>

static const float	kInset	= 3;


StatusField::StatusField(const char *name, BMessage *message, int32 digits)
	:
	BTextView(BRect(0, 0, 10, 10), name, BRect(0, 0, 10, 10),
		B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS),
	BInvoker(message, NULL),
	fDigits(digits),
	fInside(false)
{
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	SetLowUIColor(B_PANEL_BACKGROUND_COLOR);
	rgb_color	text	= ui_color(B_PANEL_TEXT_COLOR);
	SetFontAndColor(be_plain_font, B_FONT_ALL, &text);
	SetWordWrap(false);
	SetMaxBytes(digits + 2);
	SetAlignment(B_ALIGN_LEFT);
}


void StatusField::SetValue(float value)
{
	BString	text;
	text << (int32)floorf(value + 0.5f);
	_Show(text.String());
}


void StatusField::SetMixed(void)
{
	_Show("");
}


void StatusField::_Show(const char *text)
{
	fShown	= text;
	if (!IsFocus())
		SetText(text);
}


void StatusField::_Commit(void)
{
	BString	typed(Text());
	if (typed == fShown)
		return;
	typed.ReplaceAll(',', '.');
	char	*end	= NULL;
	double	value	= strtod(typed.String(), &end);
	if ((typed.Length() == 0) || (end == NULL) || (*end != '\0')) {
		beep();
		SetText(fShown.String());
		return;
	}
	if (Message() == NULL)
		return;
	BMessage	message(*Message());
	message.AddFloat("value", (float)value);
	Invoke(&message);
}


void StatusField::KeyDown(const char *bytes, int32 numBytes)
{
	switch (bytes[0]) {
		case B_ENTER:
			_Commit();
			SelectAll();
			return;
		case B_ESCAPE:
			SetText(fShown.String());
			SelectAll();
			return;
		case B_TAB:
			// to the next field, not into the text
			BView::KeyDown(bytes, numBytes);
			return;
	}
	if ((numBytes == 1) && (bytes[0] >= ' ') && (bytes[0] < 127)
		&& ((bytes[0] < '0') || (bytes[0] > '9'))
		&& (bytes[0] != '-') && (bytes[0] != '.') && (bytes[0] != ',')) {
		beep();
		return;
	}
	BTextView::KeyDown(bytes, numBytes);
}


void StatusField::MakeFocus(bool focus)
{
	if (focus == IsFocus())
		return;
	if (!focus) {
		_Commit();
		// a value that came in while editing
		SetText(fShown.String());
	}
	BTextView::MakeFocus(focus);
	if (focus)
		SelectAll();
	Invalidate();
}


void StatusField::MouseMoved(BPoint where, uint32 code,
	const BMessage *dragMessage)
{
	bool	inside	= (code != B_EXITED_VIEW) && Bounds().Contains(where);
	if (inside != fInside) {
		fInside	= inside;
		Invalidate();
	}
	BTextView::MouseMoved(where, code, dragMessage);
}


void StatusField::Draw(BRect updateRect)
{
	BTextView::Draw(updateRect);
	if (!fInside && !IsFocus())
		return;
	BRect	frame	= Bounds();
	uint32	flags	= IsFocus() ? BControlLook::B_FOCUSED : 0;
	be_control_look->DrawTextControlBorder(this, frame, updateRect,
		ViewColor(), flags);
}


void StatusField::GetPreferredSize(float *width, float *height)
{
	font_height	fh;
	be_plain_font->GetHeight(&fh);
	if (width != NULL)
		*width	= ceilf(be_plain_font->StringWidth("0") * fDigits) + 2 * kInset + 4;
	if (height != NULL)
		*height	= ceilf(fh.ascent + fh.descent) + 6;
}


void StatusField::FrameResized(float width, float height)
{
	BTextView::FrameResized(width, height);
	font_height	fh;
	be_plain_font->GetHeight(&fh);
	float	textHeight	= ceilf(fh.ascent + fh.descent);
	float	top			= floorf((height - textHeight) / 2);
	SetTextRect(BRect(kInset + 2, top, width - kInset - 2, top + textHeight));
}
