#include "ColorSwatchView.h"

#include <app/Looper.h>
#include <app/Message.h>
#include <interface/Window.h>


// Checkerboard colors for the alpha-blend preview, matching Icon-O-Matic's
// own ui_defines.h - just the two constants this file actually needs,
// rather than pulling in the whole (Icon-O-Matic specific) header.
static const rgb_color kAlphaLow	= { 0xbb, 0xbb, 0xbb, 0xff };
static const rgb_color kAlphaHigh	= { 0xe0, 0xe0, 0xe0, 0xff };
static const pattern kDottedBig	= { { 0x0f, 0x0f, 0x0f, 0x0f, 0xf0, 0xf0, 0xf0, 0xf0 } };


void
AddColorToMessage(BMessage *message, rgb_color color)
{
	message->AddData("color", B_RGB_COLOR_TYPE, &color, sizeof(rgb_color));
}


bool
ColorFromMessage(const BMessage *message, rgb_color &color)
{
	const void	*data	= NULL;
	ssize_t		size	= 0;
	if ((message->FindData("color", B_RGB_COLOR_TYPE, &data, &size) != B_OK)
		|| (size != sizeof(rgb_color)))
		return false;
	color = *(const rgb_color *)data;
	return true;
}


ColorSwatchView::ColorSwatchView(const char *name, BMessage *message,
		BHandler *target, rgb_color color, float width, float height)
	: BView(BRect(0.0, 0.0, width, height), name, B_FOLLOW_NONE, B_WILL_DRAW),
	fColor(color),
	fTrackingStart(-1.0, -1.0),
	fClickMessage(message),
	fTarget(target)
{
	SetViewColor(B_TRANSPARENT_32_BIT);
	SetHighColor(fColor);
}


ColorSwatchView::~ColorSwatchView()
{
	delete fClickMessage;
}


static inline void
blend_color(rgb_color &a, const rgb_color &b, float alpha)
{
	float alphaInv = 1.0 - alpha;
	a.red	= (uint8)(b.red * alphaInv + a.red * alpha);
	a.green	= (uint8)(b.green * alphaInv + a.green * alpha);
	a.blue	= (uint8)(b.blue * alphaInv + a.blue * alpha);
}


void
ColorSwatchView::Draw(BRect updateRect)
{
	BRect	r(Bounds());

	if (fColor.alpha < 255) {
		float	alpha	= fColor.alpha / 255.0;
		rgb_color	h	= fColor;
		blend_color(h, kAlphaHigh, alpha);
		rgb_color	l	= fColor;
		blend_color(l, kAlphaLow, alpha);

		SetHighColor(h);
		SetLowColor(l);
		FillRect(r, kDottedBig);
	} else {
		SetHighColor(fColor);
		FillRect(r);
	}
}


void
ColorSwatchView::MouseDown(BPoint where)
{
	if (Bounds().Contains(where))
		fTrackingStart = where;
}


void
ColorSwatchView::MouseUp(BPoint where)
{
	if (Bounds().Contains(where) && Bounds().Contains(fTrackingStart))
		_Invoke();

	fTrackingStart.x = -1.0;
	fTrackingStart.y = -1.0;
}


void
ColorSwatchView::SetColor(rgb_color color)
{
	fColor = color;
	SetHighColor(fColor);
	Invalidate();
}


void
ColorSwatchView::_Invoke()
{
	if ((fClickMessage == NULL) || (fTarget == NULL))
		return;

	BLooper *looper = fTarget->Looper();
	if (looper == NULL)
		return;

	BMessage	message(*fClickMessage);
	AddColorToMessage(&message, fColor);
	looper->PostMessage(&message, fTarget);
}
