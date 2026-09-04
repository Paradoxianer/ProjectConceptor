#include "ChoiceToolItem.h"

#include <interface/MenuItem.h>
#include <interface/PopUpMenu.h>
#include <support/String.h>


ChoiceToolItem::ChoiceToolItem(const char *name, BBitmap *bmp, BMessage *baseMessage)
	:ToolItem(name,bmp,baseMessage)
{
}


ChoiceToolItem::~ChoiceToolItem(void)
{
	for (int32 i=0; i<labels.CountItems(); i++)
		delete (BString *)labels.ItemAt(i);
	for (int32 i=0; i<values.CountItems(); i++)
		delete (BString *)values.ItemAt(i);
}


void ChoiceToolItem::AddChoice(const char *label, const char *value)
{
	labels.AddItem(new BString(label));
	values.AddItem(new BString(value));
}


void ChoiceToolItem::MouseDown(BPoint point)
{
	BPopUpMenu	*menu	= new BPopUpMenu("",false,false);
	for (int32 i=0; i<labels.CountItems(); i++)
		menu->AddItem(new BMenuItem(((BString *)labels.ItemAt(i))->String(),NULL));

	BMenuItem	*picked	= menu->Go(ConvertToScreen(point));
	if (picked != NULL) {
		int32	index	= menu->IndexOf(picked);
		BMessage	msg(*Message());
		msg.AddString("value",((BString *)values.ItemAt(index))->String());
		Messenger().SendMessage(&msg);
		BButton::SetToolTip(((BString *)labels.ItemAt(index))->String());
	}
	delete menu;
}
