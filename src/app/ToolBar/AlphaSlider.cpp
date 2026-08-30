/*
 * Adapted for ProjectConceptor from Haiku's own Icon-O-Matic app
 * (~/repos/haiku/src/apps/icon-o-matic/generic/gui/panel/color_picker/
 * AlphaSlider.{h,cpp}).
 *
 * Copyright 2006-2012 Stephan Assmus <superstippi@gmx.de>
 * Distributed under the terms of the MIT License.
 */

#include "AlphaSlider.h"

#include <stdio.h>
#include <string.h>

#include <app/AppDefs.h>
#include <interface/Bitmap.h>
#include <interface/ControlLook.h>
#include <interface/LayoutUtils.h>
#include <app/Message.h>
#include <interface/Window.h>

// Just the constants this file needs from Icon-O-Matic's own
// (Icon-O-Matic specific) ui_defines.h, plus stock kBlack.
static const rgb_color kBlack		= {   0,   0,   0, 255 };
static const rgb_color kAlphaLow	= { 0xbb, 0xbb, 0xbb, 0xff };
static const rgb_color kAlphaHigh	= { 0xe0, 0xe0, 0xe0, 0xff };

// End-cap swatch (ProjectConceptor addition, see the class comment in the
// header) - same checkerboard pattern and colors ColorSwatchView uses, so
// the two read as one visual language.
static const pattern	kDottedBig	= { { 0x0f, 0x0f, 0x0f, 0x0f, 0xf0, 0xf0, 0xf0, 0xf0 } };
static const float		kEndCapSize	= 24.0;
static const float		kEndCapGap	= 3.0;

// Thumb (ProjectConceptor addition, replacing the original ported code's
// plain white/black marker lines) - mirrors BColorControl's own round
// selector knob exactly, same constants and the same two-stroke technique
// (a thick white circle, then a thinner black ring on top - see
// BColorControl::_DrawSelectors() in
// ~/repos/haiku/src/kits/interface/ColorControl.cpp), so the alpha
// control's handle matches the R/G/B ramps right above it in
// ColorPickerWindow instead of looking like a different kind of control.
static const float		kSelectorSize		= 4.0;
static const float		kSelectorPenSize	= 2.0;


AlphaSlider::AlphaSlider(orientation dir, BMessage *message,
		border_style border)
	: BControl("alpha slider", NULL, message,
		B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE),
	fBitmap(NULL),
	fColor(kBlack),
	fDragging(false),
	fOrientation(dir),
	fBorderStyle(border)
{
	FrameResized(Bounds().Width(), Bounds().Height());

	// was B_TRANSPARENT_COLOR in the original ported code, which disables
	// Haiku's automatic erase-to-view-color before each Draw() call - the
	// end cap/thumb additions left a real, uncovered gap between the
	// gradient bar and the end cap that never got painted by anything,
	// and any stale pixels from a previous frame had no guarantee of
	// being cleared either. A real background color lets Haiku erase the
	// whole view correctly before every redraw, matching the panel-gray
	// ColorPickerWindow's own background now explicitly uses too.
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	SetValue(255);
}


AlphaSlider::~AlphaSlider()
{
	delete fBitmap;
}


void
AlphaSlider::WindowActivated(bool active)
{
	if (IsFocus())
		Invalidate();
}


void
AlphaSlider::MakeFocus(bool focus)
{
	if (focus != IsFocus()) {
		BControl::MakeFocus(focus);
		Invalidate();
	}
}


BSize
AlphaSlider::MinSize()
{
	BSize	minSize;
	// +kEndCapSize+kEndCapGap so the gradient itself still gets the full
	// 255 pixels of 1:1 resolution the original design assumed, with the
	// end cap occupying genuinely extra space rather than eating into it
	if (fOrientation == B_HORIZONTAL)
		minSize = BSize(255 + 4 + kEndCapSize + kEndCapGap, 7 + 4);
	else
		minSize = BSize(7 + 4, 255 + 4 + kEndCapSize + kEndCapGap);
	return BLayoutUtils::ComposeSize(ExplicitMinSize(), minSize);
}


BSize
AlphaSlider::PreferredSize()
{
	return BLayoutUtils::ComposeSize(ExplicitPreferredSize(), MinSize());
}


BSize
AlphaSlider::MaxSize()
{
	BSize	minSize;
	if (fOrientation == B_HORIZONTAL)
		minSize = BSize(B_SIZE_UNLIMITED, 16 + 4);
	else
		minSize = BSize(16 + 4, B_SIZE_UNLIMITED);
	return BLayoutUtils::ComposeSize(ExplicitMinSize(), minSize);
}


void
AlphaSlider::MouseDown(BPoint where)
{
	if (!IsEnabled())
		return;

	fDragging = true;
	SetValue(_ValueFor(where));

	SetMouseEventMask(B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS);
}


void
AlphaSlider::MouseUp(BPoint where)
{
	fDragging = false;
}


void
AlphaSlider::MouseMoved(BPoint where, uint32 transit,
	const BMessage *dragMessage)
{
	if (!IsEnabled() || !fDragging)
		return;

	SetValue(_ValueFor(where));
}


void
AlphaSlider::KeyDown(const char *bytes, int32 numBytes)
{
	if (!IsEnabled() || numBytes <= 0) {
		BControl::KeyDown(bytes, numBytes);
		return;
	}

	switch (bytes[0]) {
		case B_HOME:
			SetValue(0);
			break;
		case B_END:
			SetValue(255);
			break;

		case B_LEFT_ARROW:
		case B_UP_ARROW:
			SetValue(max_c(0, Value() - 1));
			break;
		case B_RIGHT_ARROW:
		case B_DOWN_ARROW:
			SetValue(min_c(255, Value() + 1));
			break;

		default:
			BControl::KeyDown(bytes, numBytes);
			break;
	}
}


void
AlphaSlider::Draw(BRect updateRect)
{
	BRect	b = Bounds();
	bool	isFocus = IsFocus() && Window()->IsActive();

	if (fBorderStyle == B_FANCY_BORDER) {
		rgb_color	bg = LowColor();
		uint32		flags = 0;

		if (!IsEnabled())
			flags |= BControlLook::B_DISABLED;
		if (isFocus)
			flags |= BControlLook::B_FOCUSED;

		be_control_look->DrawTextControlBorder(this, b, updateRect, bg, flags);
	}

	BRect	barRect = _BitmapRect();
	DrawBitmap(fBitmap, barRect.LeftTop());

	_DrawThumb(barRect, isFocus);
	_DrawEndCap();
}


void
AlphaSlider::FrameResized(float width, float height)
{
	BRect	r = _BitmapRect();
	_AllocBitmap(r.IntegerWidth() + 1, r.IntegerHeight() + 1);
	_UpdateColors();
	Invalidate();
}


void
AlphaSlider::SetValue(int32 value)
{
	if (value == Value())
		return;

	Invoke(Message());
	BControl::SetValue(value);
}


void
AlphaSlider::SetEnabled(bool enabled)
{
	if (enabled == IsEnabled())
		return;

	BControl::SetEnabled(enabled);

	_UpdateColors();
	Invalidate();
}


void
AlphaSlider::SetColor(const rgb_color &color)
{
	if ((uint32&)fColor == (uint32&)color)
		return;

	fColor = color;

	_UpdateColors();
	Invalidate();
}


static inline void
blend_colors(uint8 *d, uint8 alpha, uint8 c1, uint8 c2, uint8 c3)
{
	if (alpha > 0) {
		if (alpha == 255) {
			d[0] = c1;
			d[1] = c2;
			d[2] = c3;
		} else {
			d[0] += (uint8)(((c1 - d[0]) * alpha) >> 8);
			d[1] += (uint8)(((c2 - d[1]) * alpha) >> 8);
			d[2] += (uint8)(((c3 - d[2]) * alpha) >> 8);
		}
	}
}


void
AlphaSlider::_UpdateColors()
{
	if (fBitmap == NULL || !fBitmap->IsValid())
		return;

	// fill in top row with alpha gradient
	uint8	*topRow = (uint8*)fBitmap->Bits();
	uint32	width = fBitmap->Bounds().IntegerWidth() + 1;

	uint8		*p = topRow;
	rgb_color	color = fColor;
	if (!IsEnabled()) {
		// blend low color and color to give disabled look
		rgb_color	bg = LowColor();
		color.red	= (uint8)(((uint32)bg.red + color.red) / 2);
		color.green	= (uint8)(((uint32)bg.green + color.green) / 2);
		color.blue	= (uint8)(((uint32)bg.blue + color.blue) / 2);
	}
	for (uint32 x = 0; x < width; x++) {
		p[0] = color.blue;
		p[1] = color.green;
		p[2] = color.red;
		p[3] = x * 255 / width;
		p += 4;
	}
	// copy top row to rest of bitmap
	uint32	height = fBitmap->Bounds().IntegerHeight() + 1;
	uint32	bpr = fBitmap->BytesPerRow();
	uint8	*dstRow = topRow + bpr;
	for (uint32 i = 1; i < height; i++) {
		memcpy(dstRow, topRow, bpr);
		dstRow += bpr;
	}
	// post process bitmap to underlay it with a pattern to visualize alpha
	uint8	*row = topRow;
	for (uint32 i = 0; i < height; i++) {
		uint8	*p = row;
		for (uint32 x = 0; x < width; x++) {
			uint8	alpha = p[3];
			if (alpha < 255) {
				p[3] = 255;
				alpha = 255 - alpha;
				if (x % 8 >= 4) {
					if (i % 8 >= 4) {
						blend_colors(p, alpha,
									 kAlphaLow.blue,
									 kAlphaLow.green,
									 kAlphaLow.red);
					} else {
						blend_colors(p, alpha,
									 kAlphaHigh.blue,
									 kAlphaHigh.green,
									 kAlphaHigh.red);
					}
				} else {
					if (i % 8 >= 4) {
						blend_colors(p, alpha,
									 kAlphaHigh.blue,
									 kAlphaHigh.green,
									 kAlphaHigh.red);
					} else {
						blend_colors(p, alpha,
									 kAlphaLow.blue,
									 kAlphaLow.green,
									 kAlphaLow.red);
					}
				}
			}
			p += 4;
		}
		row += bpr;
	}
}


void
AlphaSlider::_AllocBitmap(int32 width, int32 height)
{
	if (width < 2 || height < 2)
		return;

	delete fBitmap;
	fBitmap = new BBitmap(BRect(0, 0, width - 1, height - 1), 0, B_RGB32);
}


BRect
AlphaSlider::_BitmapRect() const
{
	BRect	r = Bounds();
	if (fBorderStyle == B_FANCY_BORDER)
		r.InsetBy(2, 2);
	// leave room for the end cap (_EndCapRect() below) at the far end,
	// so the gradient bar doesn't draw underneath it
	if (fOrientation == B_HORIZONTAL)
		r.right		-= (kEndCapSize + kEndCapGap);
	else
		r.bottom	-= (kEndCapSize + kEndCapGap);
	return r;
}


void
AlphaSlider::_DrawThumb(BRect barRect, bool isFocus)
{
	BPoint	center;
	if (fOrientation == B_HORIZONTAL) {
		center.x	= floorf(barRect.left + Value() * barRect.Width() / 255.0 + 0.5);
		center.y	= (barRect.top + barRect.bottom) / 2;
	} else {
		center.x	= (barRect.left + barRect.right) / 2;
		center.y	= floorf(barRect.top + Value() * barRect.Height() / 255.0 + 0.5);
	}

	// exact technique/constants BColorControl::_DrawSelectors() uses -
	// SetLowColor(black) locally for the B_SOLID_LOW ring stroke, then
	// restored, since the view's real LowColor (panel background) is
	// relied on elsewhere (e.g. the border in Draw())
	SetHighColor(255, 255, 255);
	SetLowColor(0, 0, 0);
	SetPenSize(kSelectorPenSize);
	StrokeEllipse(center, kSelectorSize / 2, kSelectorSize / 2);
	SetPenSize(kSelectorPenSize / 2);
	StrokeEllipse(center, kSelectorSize, kSelectorSize, B_SOLID_LOW);
	if (isFocus)
		StrokeEllipse(center, kSelectorSize / 2, kSelectorSize / 2, B_SOLID_LOW);
	SetPenSize(1.0);
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}


BRect
AlphaSlider::_EndCapRect() const
{
	BRect	r = Bounds();
	if (fBorderStyle == B_FANCY_BORDER)
		r.InsetBy(2, 2);
	if (fOrientation == B_HORIZONTAL)
		r.left	= r.right - kEndCapSize + 1;
	else
		r.top	= r.bottom - kEndCapSize + 1;
	return r;
}


static inline void
blend_end_cap_color(rgb_color &a, const rgb_color &b, float alpha)
{
	// mirrors ColorSwatchView::blend_color() exactly - see the class
	// comment in AlphaSlider.h for why
	float alphaInv = 1.0 - alpha;
	a.red	= (uint8)(b.red * alphaInv + a.red * alpha);
	a.green	= (uint8)(b.green * alphaInv + a.green * alpha);
	a.blue	= (uint8)(b.blue * alphaInv + a.blue * alpha);
}


void
AlphaSlider::_DrawEndCap()
{
	rgb_color	color	= fColor;
	color.alpha			= (uint8)Value();

	BRect	r = _EndCapRect();
	if (color.alpha < 255) {
		float		alpha	= color.alpha / 255.0;
		rgb_color	h		= color;
		blend_end_cap_color(h, kAlphaHigh, alpha);
		rgb_color	l		= color;
		blend_end_cap_color(l, kAlphaLow, alpha);

		SetHighColor(h);
		SetLowColor(l);
		FillRect(r, kDottedBig);
	} else {
		SetHighColor(color);
		FillRect(r);
	}
}


int32
AlphaSlider::_ValueFor(BPoint where) const
{
	BRect	r = _BitmapRect();

	int32	value;
	if (fOrientation == B_HORIZONTAL)
		value = (int32)(255 * (where.x - r.left) / r.Width() + 0.5);
	else
		value = (int32)(255 * (where.y - r.top) / r.Height() + 0.5);

	value = max_c(0, value);
	value = min_c(255, value);

	return value;
}
