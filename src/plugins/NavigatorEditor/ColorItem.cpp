#include "ColorItem.h"
#include "ColorPickerWindow.h"
#include "ColorSwatchView.h"

#include <interface/InterfaceDefs.h>
#include <interface/Screen.h>
#include <interface/Window.h>

ColorItem::ColorItem(char *newLabel,
		rgb_color newValue,
		uint32 level,
		bool expanded)
	: BaseListItem(B_INT32_TYPE, level, expanded)
{
	svalue			= new char[24];
	colorValue		= newValue;
	picker			= NULL;
	sprintf(svalue,"#%02X%02X%02X",colorValue.red,colorValue.green,colorValue.blue);
	BMessage		*openMessage	= new BMessage(COLOR_ITEM_OPEN);
	openMessage->AddPointer("item",this);
	value			= new BButton(BRect(0,0,100,10),"colorButton",svalue,openMessage);
	label			= newLabel;
	background		= ui_color(B_CONTROL_BACKGROUND_COLOR);
	backgroundHi	= ui_color(B_CONTROL_HIGHLIGHT_COLOR);
	foreground		= ui_color(B_CONTROL_TEXT_COLOR);
}

ColorItem::~ColorItem()
{
	if (picker != NULL) {
		picker->Lock();
		picker->Quit();
	}
}

void ColorItem::SetColor(rgb_color newColor)
{
	colorValue	= newColor;
	sprintf(svalue,"#%02X%02X%02X",colorValue.red,colorValue.green,colorValue.blue);
	value->SetLabel(svalue);
}

void ColorItem::Update(BView *newOwner, const BFont *font)
{
	BListItem::Update(newOwner,font);
	float	widht, height;
	value->GetPreferredSize(&widht,&height);
	SetHeight(height+3);
	font_height	fontHeight;
	font->GetHeight(&fontHeight);
	textLine=((height+3)-fontHeight.ascent)/2;
}

void ColorItem::DrawItem(BView *owner, BRect bounds, bool complete)
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
	// A small swatch in the value column, always visible - unlike the
	// embedded BButton (only meaningful while selected, same as every
	// other item type here), the actual color is worth seeing at a
	// glance without selecting the row first.
	BRect	swatchRect(newBounds.right-SEPERATOR+4,newBounds.top+3,
		newBounds.right-SEPERATOR+20,newBounds.bottom-3);
	owner->SetHighColor(colorValue);
	owner->FillRect(swatchRect);
	owner->SetHighColor(0,0,0,255);
	owner->StrokeRect(swatchRect);
	if (IsSelected())
	{
		if (value->Parent() == NULL)
		{
			owner->AddChild(value);
			value->SetTarget(owner);
		}
	    value->MoveTo(swatchRect.right+4,newBounds.top+2);
	    value->ResizeTo(newBounds.right-(swatchRect.right+4)-2,newBounds.Height()-3);
	}
	else
	{
		if (value->Parent() != NULL)
			owner->RemoveChild(value);
		owner->SetHighColor(foreground);
		owner->MovePenTo(swatchRect.right+4, newBounds.bottom-textLine);
		owner->DrawString(svalue);
	}
	owner->SetHighColor(205,205,205,255);
	owner->StrokeLine(BPoint(newBounds.right-SEPERATOR,newBounds.top),BPoint(newBounds.right-SEPERATOR,newBounds.bottom));
	owner->SetHighColor(foreground);
}

void ColorItem::Select(void)
{
}
void ColorItem::Deselect(void)
{
}

void ColorItem::SetExpanded(bool /*expande*/)
{
};

status_t ColorItem::Invoke(BMessage *message)
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
		valueContainer->AddInt32("type",B_INT32_TYPE);
		valueContainer->AddString("name",label);
		valueContainer->AddInt32("newValue",*(int32 *)&colorValue);
		sendMessage->ReplaceMessage("valueContainer",valueContainer);
		return BInvoker::Invoke(sendMessage);
	}
	else
		return B_ERROR;
}

void ColorItem::OpenPicker(BHandler *target)
{
	if (picker != NULL)
		return;
	originalValue	= colorValue;
	BPoint	startPoint	= value->ConvertToScreen(value->Bounds().LeftBottom());
	startPoint.y++;
	BRect	frame(startPoint.x,startPoint.y,startPoint.x+1,startPoint.y+1);
	BMessage	*reportMessage	= new BMessage(COLOR_ITEM_REPORT);
	reportMessage->AddPointer("item",this);
	picker	= new ColorPickerWindow(frame,colorValue,reportMessage,target);
	picker->Lock();
	BScreen	screen(B_MAIN_SCREEN_ID);
	BRect	screenFrame	= screen.Frame();
	BRect	pickerFrame	= picker->Frame();
	if (screenFrame.right < pickerFrame.right)
		startPoint.x -= pickerFrame.Width();
	if (screenFrame.bottom < pickerFrame.bottom)
		startPoint.y -= pickerFrame.Height()+2;
	picker->MoveTo(startPoint);
	picker->Show();
	picker->Activate();
	picker->Unlock();
}

void ColorItem::ApplyReport(BMessage *message)
{
	rgb_color	newColor;
	if (ColorFromMessage(message,newColor))
		SetColor(newColor);
}

void ColorItem::ClosePicker(BMessage *message)
{
	picker	= NULL;
	bool	cancel	= false;
	message->FindBool("cancel",&cancel);
	if (cancel)
		SetColor(originalValue);
	else
		Invoke();
}
