#include "ChoiceToolItem.h"

#include <string.h>

#include <interface/Bitmap.h>
#include <interface/MenuBar.h>
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
	iconOnly	= false;
	SetDivider(0);
}


void ChoiceToolItem::SetIconOnly(bool _iconOnly)
{
	iconOnly	= _iconOnly;
	// with this off BMenuField paints the marked choice's label into the
	// field; Draw() puts the icon there instead
	menu->SetLabelFromMarked(!iconOnly);
	if (iconOnly) {
		menu->Superitem()->SetLabel("");
		SetFlags(Flags()|B_DRAW_ON_CHILDREN);
	}
	Invalidate();
}


void ChoiceToolItem::Draw(BRect updateRect)
{
	BMenuField::Draw(updateRect);
}


// BMenuField paints its content through a real BMenuBar child view, which
// covers anything the field itself drew - so the icon has to go on after
// the children (B_DRAW_ON_CHILDREN, set in SetIconOnly()) rather than in
// Draw().
void ChoiceToolItem::DrawAfterChildren(BRect updateRect)
{
	BMenuField::DrawAfterChildren(updateRect);
	if (!iconOnly)
		return;
	IconMenuItem	*marked	= dynamic_cast<IconMenuItem *>(menu->FindMarked());
	if (marked == NULL)
		return;
	BBitmap			*icon	= marked->Icon();
	if (icon == NULL)
		return;
	// left-align in the field's own box, clear of the pop-up marker
	// BMenuField draws down the right-hand side
	BRect	bounds	= MenuBar()->Frame();
	BRect	iconRect	= icon->Bounds();
	BPoint	where(bounds.left+4,
		bounds.top+((bounds.Height()-iconRect.Height())/2));
	SetDrawingMode(B_OP_ALPHA);
	DrawBitmap(icon,where);
	SetDrawingMode(B_OP_COPY);
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
	if (iconOnly)
		menu->Superitem()->SetLabel("");
}


void ChoiceToolItem::SetValue(const char *value)
{
	for (int32 i=0; i<menu->CountItems(); i++) {
		BMenuItem	*item	= menu->ItemAt(i);
		const char	*itemValue	= NULL;
		if ((item->Message()->FindString("value",&itemValue) == B_OK)
				&& (strcmp(itemValue,value) == 0)) {
			item->SetMarked(true);
			if (iconOnly)
				menu->Superitem()->SetLabel("");
			Invalidate();
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
