#include "IconMenuItem.h"

#include <interface/Bitmap.h>
#include <interface/ControlLook.h>
#include <interface/Menu.h>


IconMenuItem::IconMenuItem(BBitmap *bitmap, const char *title, BMessage *message)
	:BMenuItem(title,message),
	icon(bitmap)
{
}


void IconMenuItem::DrawContent(void)
{
	DrawIcon();

	BPoint	loc	= ContentLocation();
	if (icon != NULL)
		loc.x	+= ceilf(be_control_look->DefaultLabelSpacing()*3.3f);
	Menu()->MovePenTo(loc);
	BMenuItem::DrawContent();
}


void IconMenuItem::Highlight(bool isHighlighted)
{
	BMenuItem::Highlight(isHighlighted);
	DrawIcon();
}


void IconMenuItem::DrawIcon(void)
{
	if (icon == NULL)
		return;

	BPoint	loc		= ContentLocation();
	BRect	frame	= Frame();
	loc.y	= frame.top + (frame.bottom-frame.top-icon->Bounds().Height())/2;

	BMenu	*menu	= Menu();
	if (icon->ColorSpace() == B_RGBA32) {
		menu->SetDrawingMode(B_OP_ALPHA);
		menu->SetBlendingMode(B_PIXEL_ALPHA,B_ALPHA_OVERLAY);
	}
	else
		menu->SetDrawingMode(B_OP_OVER);
	menu->DrawBitmap(icon,loc);
	menu->SetDrawingMode(B_OP_COPY);
}


void IconMenuItem::GetContentSize(float *width, float *height)
{
	BMenuItem::GetContentSize(width,height);
	if (icon == NULL)
		return;

	const float	limit	= ceilf(icon->Bounds().Height()
		+(be_control_look->DefaultLabelSpacing()/3.0f));
	if (*height < limit)
		*height	= limit;
	*width	+= icon->Bounds().Width()+be_control_look->DefaultLabelSpacing();
}
