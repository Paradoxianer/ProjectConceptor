#include "StatusButton.h"

#include <interface/ControlLook.h>
#include <interface/PopUpMenu.h>
#include <interface/Window.h>

#include <math.h>

static const float	kHorizontalInset	= 6;
static const char	*kMenuMark			= " ▾";


StatusButton::StatusButton(const char *name, const char *label,
	BMessage *message)
	:
	BControl(BRect(0, 0, 10, 10), name, label, message,
		B_FOLLOW_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW),
	fMenu(NULL),
	fInside(false),
	fPressed(false)
{
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	SetLowUIColor(B_PANEL_BACKGROUND_COLOR);
	SetFont(be_plain_font);
	// clicked with the mouse only, the canvas keeps the keyboard
	SetFlags(Flags() & ~B_NAVIGABLE);
}


StatusButton::~StatusButton()
{
	delete fMenu;
}


void StatusButton::SetMenu(BPopUpMenu *menu)
{
	delete fMenu;
	fMenu	= menu;
	Invalidate();
}


void StatusButton::SetWidestLabel(const char *label)
{
	fWidestLabel	= label;
}


void StatusButton::SetLabel(const char *label)
{
	BControl::SetLabel(label);
	Invalidate();
}


BString StatusButton::_Text(const char *label) const
{
	BString	text(label);
	if (fMenu != NULL)
		text << kMenuMark;
	return text;
}


void StatusButton::GetPreferredSize(float *width, float *height)
{
	float	textWidth	= StringWidth(_Text(Label()).String());
	if (fWidestLabel.Length() > 0) {
		float	widest	= StringWidth(_Text(fWidestLabel.String()).String());
		if (widest > textWidth)
			textWidth	= widest;
	}
	font_height	fh;
	GetFontHeight(&fh);
	if (width != NULL)
		*width	= ceilf(textWidth) + 2 * kHorizontalInset;
	if (height != NULL)
		*height	= ceilf(fh.ascent + fh.descent) + 6;
}


void StatusButton::Draw(BRect updateRect)
{
	BRect		rect		= Bounds();
	rgb_color	base		= ui_color(B_CONTROL_BACKGROUND_COLOR);
	rgb_color	background	= ViewColor();
	rgb_color	text		= ui_color(B_PANEL_TEXT_COLOR);
	uint32		flags		= 0;
	if (!IsEnabled())
		flags	|= BControlLook::B_DISABLED;
	if (fPressed)
		flags	|= BControlLook::B_ACTIVATED;
	else
		flags	|= BControlLook::B_FLAT;
	if (fInside && IsEnabled())
		flags	|= BControlLook::B_HOVER;

	if ((flags & (BControlLook::B_HOVER | BControlLook::B_ACTIVATED)) != 0) {
		be_control_look->DrawButtonFrame(this, rect, updateRect, base,
			background, flags);
		be_control_look->DrawButtonBackground(this, rect, updateRect, base,
			flags);
	} else {
		SetLowColor(background);
		FillRect(rect, B_SOLID_LOW);
		base	= background;
	}
	be_control_look->DrawLabel(this, _Text(Label()).String(), Bounds(),
		updateRect, base, flags, BAlignment(B_ALIGN_CENTER, B_ALIGN_MIDDLE),
		&text);
}


void StatusButton::MouseDown(BPoint where)
{
	if (!IsEnabled())
		return;
	if (fMenu != NULL) {
		// async: the items deliver to their own targets
		BRect	screen	= ConvertToScreen(Bounds());
		fMenu->Go(screen.LeftBottom(), true, false, screen, true);
		return;
	}
	fPressed	= true;
	SetMouseEventMask(B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS);
	Invalidate();
}


void StatusButton::MouseMoved(BPoint where, uint32 code,
	const BMessage *dragMessage)
{
	bool	inside	= (code != B_EXITED_VIEW) && Bounds().Contains(where);
	if (inside == fInside)
		return;
	fInside	= inside;
	Invalidate();
}


void StatusButton::MouseUp(BPoint where)
{
	if (!fPressed)
		return;
	fPressed	= false;
	Invalidate();
	if (Bounds().Contains(where))
		Invoke();
}
