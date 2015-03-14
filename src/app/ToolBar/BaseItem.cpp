#include "BaseItem.h"

BaseItem::BaseItem(const char* /*name*/)
{
}

BaseItem::BaseItem(BMessage* /*archive*/)
{
	
	//+*classe überprüfen????
//	err = archive->FindRect("BaseItem::base_item_frame", &frame); 
	//testen ob err OK und ob gültiges Frame...
/*	if (err!=B_OK)
			frame=BRect(0,0,ITEM_WIDTH,ITEM_HEIGHT);*/
}
void BaseItem::AttachedToToolBar(ToolBar *tb)
{
	parentToolBar=tb;
}

void BaseItem::DetachedFromToolBar(ToolBar *tb)
{
	if (parentToolBar==tb)
		parentToolBar=NULL;

}
/**
 *@todo Handle Errors 
 */
status_t BaseItem::Archive(BMessage *archive,bool /*deep=true*/) const
{
	status_t err;
//	err = BArchivable::Archive(archive, deep);
	err = archive->AddString("class", "BaseItem"); 
	return err;
}

BArchivable* BaseItem::Instantiate(BMessage* /*archive*/) 
{
/*	if ( !validate_instantiation(archive, "BaseItem") )
		return NULL;
	return new BaseItem(archive); */
	return NULL;
}
