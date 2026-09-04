#include "ChoiceToolItem.h"

#include <string.h>

#include <interface/PopUpMenu.h>
#include <support/String.h>

#include "IconMenuItem.h"
#include "ToolBar.h"


ChoiceToolItem::ChoiceToolItem(const char *name, BMessage *msg, float width)
	:BMenuField(BRect(0,0,width,ITEM_HEIGHT),name,NULL,
		menu = new BPopUpMenu(name),true),
	BaseItem(name)
{
	baseMessage	= msg;
	SetDivider(0);
}


ChoiceToolItem::~ChoiceToolItem(void)
{
	delete baseMessage;
}


void ChoiceToolItem::AddChoice(const char *label, const char *value, BBitmap *icon)
{
	BMessage	*itemMessage	= new BMessage(*baseMessage);
	itemMessage->AddString("value",value);
	IconMenuItem	*item	= new IconMenuItem(icon,label,itemMessage);
	menu->AddItem(item);
	if (menu->CountItems() == 1)
		item->SetMarked(true);
}


void ChoiceToolItem::SetValue(const char *value)
{
	for (int32 i=0; i<menu->CountItems(); i++) {
		BMenuItem	*item	= menu->ItemAt(i);
		const char	*itemValue	= NULL;
		if ((item->Message()->FindString("value",&itemValue) == B_OK)
				&& (strcmp(itemValue,value) == 0)) {
			item->SetMarked(true);
			break;
		}
	}
}


status_t ChoiceToolItem::SetTarget(BMessenger messenger)
{
	return menu->SetTargetForItems(messenger);
}


void ChoiceToolItem::AttachedToToolBar(ToolBar *tb)
{
	BaseItem::AttachedToToolBar(tb);
	parentToolBar->AddChild(this);
}


void ChoiceToolItem::DetachedFromToolBar(ToolBar *tb)
{
	tb->RemoveChild(this);
	BaseItem::DetachedFromToolBar(tb);
}


status_t ChoiceToolItem::Archive(BMessage *archive, bool deep) const
{
	return BMenuField::Archive(archive,deep);
}


BArchivable* ChoiceToolItem::Instantiate(BMessage *archive)
{
	return NULL;
}
