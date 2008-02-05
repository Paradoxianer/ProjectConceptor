#include "ToolItem.h"

#include "ToolBar.h"

ToolItem::ToolItem(const char *name, BBitmap *bmp,BMessage *msg,uint32 behave):BaseItem(name),BButton(BRect(0,0,ITEM_WIDTH,ITEM_HEIGHT),name,"",msg)
{
	Init();
	//**later create a Bitmap with the Size of the IconBitmap then scale the bmp to this size (via BView...)
	toolItemBitmap	= bmp;
	tName 			= name;
	behavior 		= behave;
//	SetMessage(msg);
}

ToolItem::ToolItem(BMessage *archive):BaseItem(""),BButton(archive)
{
	status_t err;
	Init();
	err = archive->FindString("ToolItem::tName", &tName);
	//**check if the tName ist good??
	BMessage tmpArchive;
	err = archive->FindMessage("ToolItem::toolItemBitmap",&tmpArchive);
	//**Wenn die vorher geladenen variablen nicht da waren, ist das nicht so schlimm
	//err=B_OK;
	if (err == B_OK)
		toolItemBitmap = new BBitmap(&tmpArchive);
	err = archive->FindString("ToolItem::description",description);
	err = archive->FindString("ToolItem::toolTip",toolTip);
	err = archive->FindInt32("ToolItem::toolTip",(int32 *)&behavior);
	err = archive->FindInt32("ToolItem::toolTip",(int32 *)&state);
	err = archive->FindMessage("ToolItem::Message()",&tmpArchive);
	if (err == B_OK)
		SetMessage(new BMessage(tmpArchive));
	BMessenger tmpMessenger;
	archive->FindFloat("ToolItem::shadow_offset_by",&shadow_offset_by);

	err = archive->FindMessenger("ToolItem::Messenger()",&tmpMessenger);
	//**nachtragen shadow_offset_by..
	if (err == B_OK)
		SetTarget(tmpMessenger);
}

void ToolItem::Init(void)
{
	toolItemBitmap=NULL;
	description=NULL;
	toolTip=NULL;
	state=P_M_ITEM_UP;
	shadow_offset_by=2;
	fButtonBorder=true;
}

ToolItem::~ToolItem(void)
{
//	if (tName != NULL)
	//	delete tName;
//	if (popUp!=NULL) delete popUp;
//	if (kontextMenu!=NULL) delete kontextMenu;
	if (toolItemBitmap!=NULL) delete toolItemBitmap;
	if (description!=NULL) delete description;
	if (toolTip!=NULL) delete toolTip;

}

void ToolItem::AttachedToToolBar(ToolBar *tb)
{
	//**check if parentToolBar==NULL or any other error
	BaseItem::AttachedToToolBar(tb);
	parentToolBar->AddChild(this);
}

void ToolItem::DetachedFromToolBar(ToolBar *tb)
{
	tb->RemoveChild(this);
	BaseItem::DetachedFromToolBar(tb);
}

status_t ToolItem::Archive(BMessage *archive, bool deep) const
{
	status_t err;
	err = BaseItem::Archive(archive,deep);
	err = archive->AddString("class", "ToolItem");
	err = archive->AddString("ToolItem::tName",tName);
	BMessage tmpArchive;
	//**is the NULL - pointer test OK?
/*	if ((popUp!=NULL)&&( popUp->Archive(&tmpArchive, deep) == B_OK))
		err = archive->AddMessage("ToolItem::popUp",&tmpArchive);
	if ((kontextMenu!=NULL)&&(kontextMenu->Archive(&tmpArchive, deep) == B_OK))
		err = archive->AddMessage("ToolItem::kontextMenu",&tmpArchive);*/
	if ((toolItemBitmap!=NULL)&&(toolItemBitmap->Archive(&tmpArchive, deep) == B_OK))
		err = archive->AddMessage("ToolItem::toolItemBitmap",&tmpArchive);
	if (description!=NULL)
		archive->AddString("ToolItem::description",*description);
	if (toolTip!=NULL)
		archive->AddString("ToolItem::toolTip",*toolTip);
	err = archive->AddInt32("ToolItem::behavior",(int32)behavior);
	err = archive->AddInt32("ToolItem::state",(int32)state);
	err = archive->AddFloat("ToolItem::shadow_offset_by",shadow_offset_by);
	//**shoud we test  if Message or Messenger==NULL???
	err = archive->AddMessage("ToolItem::Message()",Message());
	err = archive->AddMessenger("ToolItem::Messenger()",Messenger());
	return err;
}

BArchivable* ToolItem::Instantiate(BMessage *archive)
{
	if ( !validate_instantiation(archive, "ToolItem") )
		return NULL;
	return new ToolItem(archive);
}

/*void ToolItem::Draw(ToolBar *tp,BRect updateRect)
{
	ToolBar *toolbar=(ToolBar *)tp;
	rgb_color old=toolbar->HighColor();
	toolbar->SetHighColor(shadowGray);
	BRect shadowFrame=frame;
	shadowFrame.InsetBy(2.0,2.0);
	shadowFrame.OffsetBy(4.0,4.0);
	toolbar->FillRoundRect(shadowFrame,5,5);
	BRect buttonFrame=shadowFrame;
	if ((state&P_M_ITEM_DOWN) == P_M_ITEM_DOWN)
	{
		buttonFrame.OffsetBy(-4.0,-4.0);
		toolbar->SetHighColor(itemDarkGray);
	}
	else
		toolbar->SetHighColor(itemGray);
	toolbar->FillRoundRect(buttonFrame,5,5);
	toolbar->SetHighColor(black);
	toolbar->StrokeRoundRect(buttonFrame,5,5);
	if (toolItemBitmap!=NULL)
		toolbar->DrawBitmap(toolItemBitmap, buttonFrame);
	else
		toolbar->DrawString(tName);
	if (popUp != NULL)
	{
		BPoint p1=BPoint(buttonFrame.right-1,buttonFrame.bottom-1);
		BPoint p2=BPoint(buttonFrame.right-1,buttonFrame.bottom-POPUP_SIGN_HEIGHT-1);
		BPoint p3=BPoint(buttonFrame.right-POPUP_SIGN_WIDTH-1,buttonFrame.bottom-1);
		toolbar->SetHighColor(triangleBlue);
		toolbar->FillTriangle(p1,p2,p3);
	}
	toolbar->SetHighColor(old);
}*/

void ToolItem::Draw(BRect updateRect)
{
	if (fButtonBorder)
		BButton::Draw(updateRect); //enable button border
	SetDrawingMode(B_OP_ALPHA);
	BRect buttonFrame=BRect(0,0,18,18);

	if (Value() != B_CONTROL_ON)
		buttonFrame.OffsetTo(4,4);
	else
	{
		buttonFrame.InsetBy(1,1);
		buttonFrame.OffsetTo(5,5);
	}
	if ((toolItemBitmap) && (toolItemBitmap->IsValid()) )
		DrawBitmap(toolItemBitmap,buttonFrame);

}


void ToolItem::MouseDown(BPoint point)
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
void ToolItem::MouseUp(BPoint point)
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
	if (behavior != P_M_ONE_STATE_ITEM)
		BButton::SetValue(B_CONTROL_OFF);
}
