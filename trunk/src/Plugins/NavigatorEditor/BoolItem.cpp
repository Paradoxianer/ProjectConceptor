#include "BoolItem.h"
#include <stdio.h>
#include <interface/InterfaceDefs.h>

#ifdef B_ZETA_VERSION_BETA
	#include <locale/Locale.h>
#else
	#define _T(a) a
#endif 

BoolItem::BoolItem(char *newLabel, bool newValue, uint32 level = 0, bool expanded = true):BaseListItem(B_BOOL_TYPE,level,expanded)
{
	svalue			= new char[24];
	value			= new BCheckBox(BRect(0,0,100,10),"checker",NULL,NULL);
	value->SetValue(newValue);
	label			= newLabel;
	background		= ui_color(B_CONTROL_BACKGROUND_COLOR);
	backgroundHi	= ui_color(B_CONTROL_HIGHLIGHT_COLOR);
	foreground		= ui_color(B_CONTROL_TEXT_COLOR);
	separated		= 100;
	ValueChange();
}



void BoolItem::Update(BView *newOwner, const BFont *font)
{
	BListItem::Update(newOwner,font);
	float	widht, height;
	value->GetPreferredSize(&widht,&height);
	SetHeight(height+3);
	font_height	fontHeight;
	font->GetHeight(&fontHeight);
	textLine=((height+3)-fontHeight.ascent)/2;
}

void BoolItem::DrawItem(BView *owner, BRect bounds, bool complete = false)
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
	value->SetViewColor(color);
	owner->FillRoundRect(bounds,3,3);
	if (IsEnabled())
		owner->SetHighColor(foreground);	
	else
		owner->SetHighColor(tint_color(foreground,B_DISABLED_LABEL_TINT));
	owner->MovePenTo(newBounds.left+4, newBounds.bottom-textLine); 
	owner->DrawString(label);
	if (IsSelected())
	{
		if (value->Parent() == NULL)
			owner->AddChild(value);
	    value->MoveTo(newBounds.right-separated+5,newBounds.top+2);
	    value->ResizeTo(newBounds.right-separated-3,newBounds.Height()-3);
	}
	else
	{
		if (value->Parent() != NULL)
		{
			owner->RemoveChild(value);
			if (value->Value())
				sprintf(svalue,_T("true"));
			else
				sprintf(svalue,_T("false"));
		}
		owner->MovePenTo(newBounds.right-separated+3, newBounds.bottom-textLine);
		owner->DrawString(svalue); 
	}
	owner->SetHighColor(205,205,205,255);
//	owner->StrokeRoundRect(newBounds,3,3);
	owner->StrokeLine(BPoint(newBounds.right-separated,newBounds.top),BPoint(newBounds.right-separated,newBounds.bottom));
	owner->SetHighColor(foreground);	
}

void BoolItem::Select(void)
{
}
void BoolItem::Deselect(void)
{
}

void BoolItem::SetExpanded(bool expande)
{
};

void	BoolItem::ValueChange(void)
{
	if (value->Value())
		sprintf(svalue,_T("true"));
	else
		sprintf(svalue,_T("false"));
}
