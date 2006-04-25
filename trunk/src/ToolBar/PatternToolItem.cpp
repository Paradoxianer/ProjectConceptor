#include "PatternToolItem.h"

#include "ToolBar.h"

PatternToolItem::PatternToolItem(const char *name, pattern newValue,BMessage *msg,uint32 behave = P_M_ONE_STATE_ITEM):BaseItem(name),BButton(BRect(0,0,ITEM_WIDTH,ITEM_HEIGHT),name,"",msg)
{
	Init();
	//**later create a Bitmap with the Size of the IconBitmap then scale the bmp to this size (via BView...)
	value			= newValue;
	tName 			= name;
	behavior 		= behave;
//	SetMessage(msg);
}
PatternToolItem::PatternToolItem(BMessage *archive):BaseItem(""),BButton(archive)
{
	status_t	err;
	ssize_t		size;
	Init();
	err = archive->FindString("PatternToolItem::tName", &tName); 
	//**check if the tName ist good??
	void *pointer=&value;
	err = archive->FindData("PatternToolItem::value",B_PATTERN_TYPE,(const void **)&pointer, &size);
	//**Wenn die vorher geladenen variablen nicht da waren, ist das nicht so schlimm
	//err=B_OK;
	err = archive->FindString("PatternToolItem::description",description);
	err = archive->FindString("PatternToolItem::toolTip",toolTip);
	err = archive->FindInt32("PatternToolItem::toolTip",(int32 *)&behavior);
	err = archive->FindInt32("PatternToolItem::toolTip",(int32 *)&state);
	archive->FindFloat("PatternToolItem::shadow_offset_by",&shadow_offset_by);
	BMessenger tmpMessenger;
	err = archive->FindMessenger("PatternToolItem::Messenger()",&tmpMessenger);	//**nachtragen shadow_offset_by..
	if (err == B_OK)
		SetTarget(tmpMessenger);	
			
}
void PatternToolItem::Init(void)
{
	description=NULL;
	toolTip=NULL;
	state=P_M_ITEM_UP;
	shadow_offset_by=2;
}

PatternToolItem::~PatternToolItem(void)
{
	if (description!=NULL) delete description;
	if (toolTip!=NULL) delete toolTip;
	
}

void PatternToolItem::AttachedToToolBar(ToolBar *tb)
{
	//**check if parentToolBar==NULL or any other error
	BaseItem::AttachedToToolBar(tb);
	parentToolBar->AddChild(this);
}

void PatternToolItem::DetachedFromToolBar(ToolBar *tb)
{
	tb->RemoveChild(this);
	BaseItem::DetachedFromToolBar(tb);
}

status_t PatternToolItem::Archive(BMessage *archive, bool deep=true) const
{
	status_t err;
	err = BaseItem::Archive(archive,deep);
	err = archive->AddString("class", "PatternToolItem"); 
	err = archive->AddString("PatternToolItem::tName",tName);
	BMessage tmpArchive;
	//**is the NULL - pointer test OK?
/*	if ((popUp!=NULL)&&( popUp->Archive(&tmpArchive, deep) == B_OK))
		err = archive->AddMessage("PatternToolItem::popUp",&tmpArchive);
	if ((kontextMenu!=NULL)&&(kontextMenu->Archive(&tmpArchive, deep) == B_OK))
		err = archive->AddMessage("PatternToolItem::kontextMenu",&tmpArchive);*/
	err = archive->AddData("PatternToolItem::value",B_PATTERN_TYPE,&value,sizeof(value));
	if (description!=NULL)
		archive->AddString("PatternToolItem::description",*description);
	if (toolTip!=NULL)
		archive->AddString("PatternToolItem::toolTip",*toolTip);
	err = archive->AddInt32("PatternToolItem::behavior",(int32)behavior);
	err = archive->AddInt32("PatternToolItem::state",(int32)state);
	err = archive->AddFloat("PatternToolItem::shadow_offset_by",shadow_offset_by);
	//**shoud we test  if Message or Messenger==NULL???
	err = archive->AddMessage("PatternToolItem::Message()",Message());
	err = archive->AddMessenger("PatternToolItem::Messenger()",Messenger());
	return err;
}

BArchivable* PatternToolItem::Instantiate(BMessage *archive)
{
	if ( !validate_instantiation(archive, "PatternToolItem") )
		return NULL;
	return new PatternToolItem(archive); 
}

void PatternToolItem::Draw(BRect updateRect)
{
	BButton::Draw(updateRect);
	SetDrawingMode(B_OP_OVER);
	BRect buttonFrame=BRect(0,0,17,17);
	if (Value() != B_CONTROL_ON)
	{
		buttonFrame.OffsetTo(4,4);
	}
	else
	{
		buttonFrame.OffsetTo(5,5);
		buttonFrame.bottom -=2;
		buttonFrame.right -=2;
	}
	SetLowColor(255,255,255,255);
	SetHighColor(0,0,0,255);
	FillRoundRect(buttonFrame,4,4,value);
	SetHighColor(ui_color(B_UI_CONTROL_BORDER_COLOR));
	StrokeRoundRect(buttonFrame,4,4);
}

void PatternToolItem::MouseDown(BPoint point)
{
/*	if (Bounds().Contains(point))
	{
		if (behavior == P_M_ONE_STATE_ITEM)
		{
			state = P_M_ITEM_DOWN;
		}
		else
		{
			if ((state&P_M_ITEM_DOWN) == P_M_ITEM_DOWN)
				//Markieren dass der Button beim nächsten mal loslassen wieder hoch muss
				state=state|P_M_ITEM_UP;
			else
			{
				//ansonsten ein "normale" down
				state=P_M_ITEM_DOWN;
				//aber schon die Naricht weiterleiten
				Invoke();
				//the Modifikation Message
				BMessage *msg=parentToolBar->Message();
				if (msg!=NULL)
				{
					msg->AddPointer("source",(const void *)this);
					msg->AddInt32("state",state);
					parentToolBar->SetMessage(msg);
					parentToolBar->Invoke();
				}
			}
		}
	}*/
	BButton::MouseDown(point);
}
void PatternToolItem::MouseUp(BPoint point)
{
/*	if (behavior == P_M_ONE_STATE_ITEM)
	{
		state = P_M_ITEM_UP;
		if (Bounds().Contains(point))
		{
			Invoke();
			//the Modifikation Message
			BMessage *msg=parentToolBar->Message();
			if (msg!=NULL)
			{
				msg->AddPointer("source",(const void *)this);
				msg->AddInt32("state",state);
				parentToolBar->SetMessage(msg);
				parentToolBar->Invoke();
			}
		}
	}
	else
	{
		if (((state&P_M_ITEM_DOWN) == P_M_ITEM_DOWN)&&(Bounds().Contains(point)))
		{
			if ((state&P_M_ITEM_UP) == P_M_ITEM_UP)
			{
				state = P_M_ITEM_UP;
				Invoke();
				//the Modifikation Message
				BMessage *msg=parentToolBar->Message();
				if (msg!=NULL)
				{
					msg->AddPointer("source",(const void *)this);
					msg->AddInt32("state",state);
					parentToolBar->SetMessage(msg);
					parentToolBar->Invoke();
				}
			}
		}
	}*/
	BButton::MouseUp(point);
}
