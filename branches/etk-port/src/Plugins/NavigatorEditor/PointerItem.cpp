#include "PointerItem.h"
#include <stdio.h>
#include <interface/InterfaceDefs.h>


PointerItem::PointerItem(char *newLabel, void *newValue, uint32 level = 0, bool expanded = true):BaseListItem(B_POINTER_TYPE,level,expanded)
{
	label			= newLabel;
	value			= newValue;
	background		= ui_color(B_CONTROL_BACKGROUND_COLOR);
	backgroundHi	= ui_color(B_CONTROL_HIGHLIGHT_COLOR);
	foreground		= ui_color(B_CONTROL_TEXT_COLOR);
	separated		= 100;
}



void PointerItem::Update(BView *newOwner, const BFont *font)
{
	BListItem::Update(newOwner,font);
	owner = newOwner;
	float	widht,height;
	SetHeight(height+3);
	font_height	fontHeight;
	font->GetHeight(&fontHeight);
	textLine=((height+3)-fontHeight.ascent)/2;
}

void PointerItem::DrawItem(BView *owner, BRect bounds, bool complete = false)
{
	BRect	newBounds=bounds;
	newBounds.InsetBy(1,1);
	owner->SetDrawingMode(B_OP_OVER);
	owner->MovePenTo(bounds.left+4, bounds.bottom-2);
	rgb_color color;
	if (IsSelected())
		color = backgroundHi;
	else
		color = background;
	owner->SetHighColor(color);
	owner->FillRoundRect(bounds,3,3);
	if (IsEnabled())
		owner->SetHighColor(foreground);	
	else
		owner->SetHighColor(tint_color(foreground,B_DISABLED_LABEL_TINT));
	owner->MovePenTo(newBounds.left+4, newBounds.bottom-textLine); 
	owner->DrawString(label);
	owner->SetHighColor(205,205,205,255);
//	owner->StrokeRoundRect(newBounds,3,3);
	owner->StrokeLine(BPoint(newBounds.right-separated,newBounds.top),BPoint(newBounds.right-separated,newBounds.bottom));
	owner->SetHighColor(foreground);	
}

void PointerItem::Select(void)
{
}
void PointerItem::Deselect(void)
{
}

void PointerItem::SetExpanded(bool expande)
{
	printf("SetExpanded\n");
};
