#include "PointItem.h"
#include <stdio.h>
#include <stdlib.h>
#include <interface/InterfaceDefs.h>
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PointItem"
/*
 * @author Paradoxon
 */

PointItem::PointItem(char *newLabel,
		BPoint newPoint,
		uint32 level,
		bool expanded)
	: BaseListItem(B_POINT_TYPE, level, expanded)
{
	sx				= new char[24];
	sy				= new char[24];
	sprintf(sx,"%.2f",newPoint.x);
	sprintf(sy,"%.2f",newPoint.y);

	BMessage		*inputChanged = new BMessage(ITEM_CHANDED);
	inputChanged->AddPointer("item",this);

	x				= new BTextControl(BRect(0,0,100,10),"x","x",sx,inputChanged);
	y				= new BTextControl(BRect(0,0,100,10),"y","y",sy,inputChanged);
	x->SetDivider(20);
	y->SetDivider(20);
	label			= newLabel;
	background		= ui_color(B_CONTROL_BACKGROUND_COLOR);
	backgroundHi	= ui_color(B_CONTROL_HIGHLIGHT_COLOR);
	foreground		= ui_color(B_CONTROL_TEXT_COLOR);
}

void PointItem::Update(BView *newOwner, const BFont *font)
{
	BListItem::Update(newOwner,font);
	float	widht;
	x->GetPreferredSize(&widht,&textControlHeight);
	SetHeight(textControlHeight*2+3);
	font_height	fontHeight;
	font->GetHeight(&fontHeight);
	textLine=((textControlHeight+3)-fontHeight.ascent)/2;
}

void PointItem::DrawItem(BView *owner, BRect bounds, bool complete)
{
	BRect	newBounds=bounds;
	newBounds.InsetBy(1,1);
	owner->SetDrawingMode(B_OP_OVER);
	owner->MovePenTo(newBounds.left+4, newBounds.bottom-2);
	rgb_color color;
	if (IsSelected())
		color = backgroundHi;
	else
		color = background;
	owner->SetHighColor(color);
	x->SetViewColor(color);
	y->SetViewColor(color);
	owner->FillRoundRect(bounds,3,3);
	if (IsEnabled())
		owner->SetHighColor(foreground);
	else
		owner->SetHighColor(tint_color(foreground,B_DISABLED_LABEL_TINT));
	owner->MovePenTo(newBounds.left+4, newBounds.bottom-(textControlHeight)-textLine);
	owner->DrawString(label);
	if (IsSelected())
	{
		if (x->Parent() == NULL)
		{
			owner->AddChild(x);
			owner->AddChild(y);
			x->ResizeTo(newBounds.Width()-SEPERATOR-3,textControlHeight);
			y->ResizeTo(newBounds.Width()-SEPERATOR-3,textControlHeight);
			x->SetTarget(owner);
			y->SetTarget(owner);
		}
	    x->MoveTo(newBounds.right-SEPERATOR+1,newBounds.top+2);
	    y->MoveTo(newBounds.right-SEPERATOR+1,x->Frame().bottom);
	}
	else
	{
		if (x->Parent() != NULL)
		{
			sx	= (char *)x->Text();
			sy	= (char *)y->Text();
			owner->RemoveChild(x);
			owner->RemoveChild(y);
		}
		owner->MovePenTo(newBounds.right-SEPERATOR+3, newBounds.bottom-(textControlHeight)-textLine);
		owner->DrawString("x: ");
		owner->DrawString(sx);
		owner->MovePenTo(newBounds.right-SEPERATOR+3, newBounds.bottom-textLine);
		owner->DrawString("y: ");
		owner->DrawString(sy);
	}
	owner->SetHighColor(205,205,205,255);
	owner->StrokeLine(BPoint(newBounds.right-SEPERATOR,newBounds.top),BPoint(newBounds.right-SEPERATOR,newBounds.bottom));
	owner->SetHighColor(foreground);
}

void PointItem::Select(void)
{
}
void PointItem::Deselect(void)
{
}

void PointItem::SetExpanded(bool /*expande*/)
{
}

BPoint PointItem::GetPoint(void)
{
	return BPoint(atof(x->Text()),atof(y->Text()));
}

status_t PointItem::Invoke(BMessage *message)
{
	BMessage	*sendMessage	= NULL;
	BMessage	*valueContainer	= new BMessage();
	if (message==NULL)
		sendMessage = new BMessage(*Message());
	else
		sendMessage = new BMessage(*message);
	if (sendMessage != NULL)
	{
		sendMessage->FindMessage("valueContainer",valueContainer);
		valueContainer->AddInt32("type",B_POINT_TYPE);
		valueContainer->AddString("name",label);
		valueContainer->AddPoint("newValue", GetPoint());
		sendMessage->ReplaceMessage("valueContainer",valueContainer);
		return BInvoker::Invoke(sendMessage);
	}
	else
		return B_ERROR;
}
