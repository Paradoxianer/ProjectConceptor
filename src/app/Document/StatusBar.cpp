#include "StatusBar.h"

#include <interface/ControlLook.h>
#include <interface/Font.h>

#include <math.h>

static const float	kPadding	= 8;
static const float	kSlotGap	= 18;


StatusBar::StatusBar(BRect frame, const char *name)
	:
	BView(frame, name, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM,
		B_WILL_DRAW | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE)
{
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	SetLowUIColor(B_PANEL_BACKGROUND_COLOR);
	SetFont(be_plain_font);
}


float StatusBar::PreferredHeight(void)
{
	font_height	fh;
	be_plain_font->GetHeight(&fh);
	return ceilf(fh.ascent + fh.descent + fh.leading) + 8;
}


void StatusBar::SetText(const char *slot, const char *text)
{
	bool	remove	= (text == NULL);
	for (size_t i = 0; i < fSlots.size(); i++) {
		if (fSlots[i].name != slot)
			continue;
		if (remove)
			fSlots.erase(fSlots.begin() + i);
		else if (fSlots[i].text == text)
			return;
		else
			fSlots[i].text	= text;
		Invalidate();
		return;
	}
	if (remove)
		return;
	Slot	added;
	added.name	= slot;
	added.text	= text;
	fSlots.push_back(added);
	Invalidate();
}


void StatusBar::AddRightView(BView *view)
{
	if ((view == NULL) || fRightViews.HasItem(view))
		return;
	fRightViews.AddItem(view);
	AddChild(view);
	_LayoutRightViews();
}


void StatusBar::RemoveRightView(BView *view)
{
	if (!fRightViews.RemoveItem(view))
		return;
	RemoveChild(view);
	_LayoutRightViews();
}


void StatusBar::_LayoutRightViews(void)
{
	float	right	= Bounds().right;
	for (int32 i = fRightViews.CountItems() - 1; i >= 0; i--) {
		BView	*view	= (BView *)fRightViews.ItemAt(i);
		float	width, height;
		view->GetPreferredSize(&width, &height);
		if (height > Bounds().Height())
			height	= Bounds().Height();
		view->ResizeTo(width, height);
		view->MoveTo(right - width, (Bounds().Height() - height) / 2);
		right	-= width + kPadding;
	}
	Invalidate();
}


void StatusBar::FrameResized(float width, float height)
{
	_LayoutRightViews();
}


void StatusBar::Draw(BRect updateRect)
{
	BRect	bounds	= Bounds();
	rgb_color	base	= ui_color(B_PANEL_BACKGROUND_COLOR);
	SetHighColor(tint_color(base, B_DARKEN_2_TINT));
	StrokeLine(bounds.LeftTop(), bounds.RightTop());

	font_height	fh;
	GetFontHeight(&fh);
	float	baseline	= floorf((bounds.Height() + fh.ascent - fh.descent) / 2) + 1;
	float	x			= kPadding;
	rgb_color	text	= ui_color(B_PANEL_TEXT_COLOR);
	bool	first	= true;
	for (size_t i = 0; i < fSlots.size(); i++) {
		if (fSlots[i].text.Length() == 0)
			continue;
		if (!first) {
			// a quiet divider between texts
			SetHighColor(tint_color(base, B_DARKEN_1_TINT));
			float	middle	= x - kSlotGap / 2;
			StrokeLine(BPoint(middle, bounds.top + 5), BPoint(middle, bounds.bottom - 4));
		}
		SetHighColor(text);
		DrawString(fSlots[i].text.String(), BPoint(x, baseline));
		x	+= StringWidth(fSlots[i].text.String()) + kSlotGap;
		first	= false;
	}
}
