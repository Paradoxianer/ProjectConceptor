#include <interface/Point.h>
#include <interface/Screen.h>

#include <stdlib.h>

#include "FloatToolItem.h"
#include "ToolBar.h"

FloatToolItem::FloatToolItem(const char *name, float newValue,BMessage *msg):BButton(BRect(0,0,ITEM_WIDTH*4,ITEM_HEIGHT),name,"",msg), BaseItem(name)
{
	Init();
	value			= newValue;
	tName 			= name;
	BRect	textControlRect	= Bounds();
	textControlRect.InsetBy(5,1);
	char*	floatToText	= new char[24];
	sprintf(floatToText,"%.2f",newValue);
	textValue		= new BTextControl(textControlRect,name,name,"",new BMessage(*msg));
	float moveToY=(Bounds().Height()-textValue->Bounds().Height())/2;
	textValue->MoveTo(textValue->Frame().left,moveToY);
	AddChild(textValue);
//	SetMessage(msg);
}
FloatToolItem::FloatToolItem(BMessage *archive):BButton(archive),BaseItem("")
{
	status_t	err;
	ssize_t		size;
	Init();
	err = archive->FindString("FloatToolItem::tName", &tName); 
	//**check if the tName ist good??
	void *pointer=&value;
	err = archive->FindData("FloatToolItem::value",B_PATTERN_TYPE,(const void **)&pointer, &size);
	//**Wenn die vorher geladenen variablen nicht da waren, ist das nicht so schlimm
	//err=B_OK;
	err = archive->FindString("FloatToolItem::description",description);
	err = archive->FindString("FloatToolItem::toolTip",toolTip);
	BMessenger tmpMessenger;
	err = archive->FindMessenger("FloatToolItem::Messenger()",&tmpMessenger);	//**nachtragen shadow_offset_by..
	if (err == B_OK)
		SetTarget(tmpMessenger);	
}
void FloatToolItem::Init(void)
{
	description			= NULL;
	toolTip				= NULL;

}

FloatToolItem::~FloatToolItem(void)
{
	if (description!=NULL) delete description;
	if (toolTip!=NULL) delete toolTip;
}

void FloatToolItem::AttachedToToolBar(ToolBar *tb)
{
	//**check if parentToolBar==NULL or any other error
	BaseItem::AttachedToToolBar(tb);
	parentToolBar->AddChild(this);
}

void FloatToolItem::DetachedFromToolBar(ToolBar *tb)
{
	tb->RemoveChild(this);
	BaseItem::DetachedFromToolBar(tb);
}

status_t FloatToolItem::Archive(BMessage *archive, bool deep=true) const
{
	status_t err;
	err = BaseItem::Archive(archive,deep);
	err = archive->AddString("class", "FloatToolItem"); 
	err = archive->AddString("FloatToolItem::tName",tName);
	BMessage tmpArchive;
	//**is the NULL - pointer test OK?
	err = archive->AddData("FloatToolItem::value",B_PATTERN_TYPE,&value,sizeof(value));
	if (description!=NULL)
		archive->AddString("FloatToolItem::description",*description);
	if (toolTip!=NULL)
		archive->AddString("FloatToolItem::toolTip",*toolTip);
	//**shoud we test  if Message or Messenger==NULL???
	err = archive->AddMessage("FloatToolItem::Message()",Message());
	err = archive->AddMessenger("FloatToolItem::Messenger()",Messenger());
	return err;
}

BArchivable* FloatToolItem::Instantiate(BMessage *archive)
{
	if ( !validate_instantiation(archive, "FloatToolItem") )
		return NULL;
	return new FloatToolItem(archive); 
}

void FloatToolItem::Draw(BRect updateRect)
{
	BButton::Draw(updateRect);
}

void FloatToolItem::MessageReceived(BMessage *message)
{
	BButton::MessageReceived(message);
}

void FloatToolItem::SetValue(float newValue)
{
	value=newValue;
	char*	floatToText	= new char[24];
	sprintf(floatToText,"%.2f",newValue);
	textValue->SetText(floatToText);
}

float FloatToolItem::GetValue(void)
{
	float returnVal =atof(textValue->Text());
	char*	floatToText	= new char[24];
	sprintf(floatToText,"%.2f",returnVal);
	textValue->SetText(floatToText);
	return returnVal;
}

