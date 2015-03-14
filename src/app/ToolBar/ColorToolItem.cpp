#include <interface/Point.h>
#include <interface/Screen.h>
#include "ColorToolItem.h"

#include "ToolBar.h"

ColorToolItem::ColorToolItem(const char *name, rgb_color newValue,BMessage *msg):BaseItem(name),BButton(BRect(0,0,ITEM_WIDTH,ITEM_HEIGHT),name,"",msg)
{
	Init();
	//**later create a Bitmap with the Size of the IconBitmap then scale the bmp to this size (via BView...)
	value			= newValue;
	tName 			= name;
	BPoint	lefttop(1,1);
	colorPicker =new  BColorControl(lefttop,B_CELLS_32x8 ,1.0,"ColorPicker",new BMessage(COLOR_CHANGED));
	colorPicker->SetValue(newValue);
//	SetMessage(msg);
}

ColorToolItem::ColorToolItem(BMessage *archive):BaseItem(""),BButton(archive)
{
	status_t	err;
	ssize_t		size;
	Init();
	err = archive->FindString("ColorToolItem::tName", &tName); 
	//**check if the tName ist good??
	void *pointer=&value;
	err = archive->FindData("ColorToolItem::value",B_PATTERN_TYPE,(const void **)&pointer, &size);
	//**Wenn die vorher geladenen variablen nicht da waren, ist das nicht so schlimm
	//err=B_OK;
	err = archive->FindString("ColorToolItem::description",description);
	err = archive->FindString("ColorToolItem::toolTip",toolTip);
	err = archive->FindInt32("ColorToolItem::toolTip",(int32 *)&behavior);
	err = archive->FindInt32("ColorToolItem::toolTip",(int32 *)&state);
	BMessenger tmpMessenger;
	err = archive->FindMessenger("ColorToolItem::Messenger()",&tmpMessenger);	//**nachtragen shadow_offset_by..
	if (err == B_OK)
		SetTarget(tmpMessenger);	
	BPoint	lefttop(1,1);
	colorPicker =new  BColorControl(lefttop,B_CELLS_32x8,1.0,"ColorPicker",new BMessage(COLOR_CHANGED));
}
void ColorToolItem::Init(void)
{
	description			= NULL;
	toolTip				= NULL;
	state				= P_M_ITEM_UP;
	colorWindow			= NULL;

}

ColorToolItem::~ColorToolItem(void)
{
	if (description!=NULL) delete description;
	if (toolTip!=NULL) delete toolTip;
}

void ColorToolItem::AttachedToToolBar(ToolBar *tb)
{
	//**check if parentToolBar==NULL or any other error
	BaseItem::AttachedToToolBar(tb);
	parentToolBar->AddChild(this);
	colorPicker->SetTarget(this);
}

void ColorToolItem::DetachedFromToolBar(ToolBar *tb)
{
	tb->RemoveChild(this);
	BaseItem::DetachedFromToolBar(tb);
}

status_t ColorToolItem::Archive(BMessage *archive, bool deep) const
{
	status_t err;
	err = BaseItem::Archive(archive,deep);
	err = archive->AddString("class", "ColorToolItem"); 
	err = archive->AddString("ColorToolItem::tName",tName);
	BMessage tmpArchive;
	//**is the NULL - pointer test OK?
	err = archive->AddData("ColorToolItem::value",B_PATTERN_TYPE,&value,sizeof(value));
	if (description!=NULL)
		archive->AddString("ColorToolItem::description",*description);
	if (toolTip!=NULL)
		archive->AddString("ColorToolItem::toolTip",*toolTip);
	err = archive->AddInt32("ColorToolItem::behavior",(int32)behavior);
	err = archive->AddInt32("ColorToolItem::state",(int32)state);

	//**shoud we test  if Message or Messenger==NULL???
	err = archive->AddMessage("ColorToolItem::Message()",Message());
	err = archive->AddMessenger("ColorToolItem::Messenger()",Messenger());
	return err;
}

BArchivable* ColorToolItem::Instantiate(BMessage *archive)
{
	if ( !validate_instantiation(archive, "ColorToolItem") )
		return NULL;
	return new ColorToolItem(archive); 
}

void ColorToolItem::Draw(BRect updateRect)
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
	SetHighColor(value);
	FillRoundRect(buttonFrame,4,4);
	SetHighColor(tint_color(value,0));
	StrokeLine(BPoint(buttonFrame.left,buttonFrame.top+1),BPoint(buttonFrame.right,buttonFrame.top+1));
	SetHighColor(tint_color(value,0.2));
	StrokeLine(BPoint(buttonFrame.left,buttonFrame.top+2),BPoint(buttonFrame.right,buttonFrame.top+2));
	SetHighColor(tint_color(value,0.4));
	StrokeLine(BPoint(buttonFrame.left,buttonFrame.top+3),BPoint(buttonFrame.right,buttonFrame.top+3));
	SetHighColor(tint_color(value,0.6));
	StrokeLine(BPoint(buttonFrame.left,buttonFrame.top+4),BPoint(buttonFrame.right,buttonFrame.top+4));
	SetHighColor(tint_color(value,0.8));
	StrokeLine(BPoint(buttonFrame.left,buttonFrame.top+5),BPoint(buttonFrame.right,buttonFrame.top+5));


	SetHighColor(ui_color(B_KEYBOARD_NAVIGATION_COLOR));
	StrokeRoundRect(buttonFrame,4,4);
}

void ColorToolItem::MouseDown(BPoint point)
{
	if ((colorWindow == NULL) || (colorWindow->IsHidden()))
	{
		//** Unterschied zwischen Vertikal und Horizontal beachten
		BScreen *screenchecker=new BScreen(B_MAIN_SCREEN_ID);
		BPoint startpoint=Frame().RightTop();
		startpoint=parentToolBar->ConvertToScreen(startpoint);
		startpoint.x++;
		if (colorWindow == NULL)
		{
			colorWindow=new BWindow(colorPicker->Bounds(),tName,B_BORDERED_WINDOW,B_NOT_CLOSABLE);
			colorWindow->SetFeel(B_FLOATING_APP_WINDOW_FEEL);
			parentToolBar->Window()->AddToSubset(colorWindow);
			colorWindow->AddChild(colorPicker);
		}
		colorWindow->Lock();
		float width, height;
		colorPicker->GetPreferredSize(&width,&height);
		colorWindow->ResizeTo(width,height);
		colorWindow->Unlock();

		colorWindow->MoveTo(startpoint);
		BRect colorWindowFrame=colorWindow->Frame();
		colorWindow->Lock();	
		BRect screenFrame=screenchecker->Frame();
		if (screenFrame.right<colorWindowFrame.right)
		{
			//move Menu Window to the left
			startpoint=Frame().LeftTop();
			startpoint=parentToolBar->ConvertToScreen(startpoint);
			startpoint.x-=colorWindowFrame.Width();
			startpoint.y=colorWindowFrame.top;
			startpoint.x--;
			colorWindow->MoveTo(startpoint);
		}
		if (screenFrame.bottom<colorWindowFrame.bottom)
		{
			//reload colorWindowFrame because it coud have changed during the if bevore
			colorWindowFrame=colorWindow->Frame();
			startpoint=Frame().RightBottom();
			startpoint=parentToolBar->ConvertToScreen(startpoint);
			startpoint.y-=colorWindowFrame.Height();
			startpoint.x=colorWindowFrame.left;
			//move Menu Window to top
			colorWindow->MoveTo(startpoint);
		}
		colorWindow->Show();
		colorWindow->Unlock();
		oldEventMask=EventMask();
		SetEventMask(B_POINTER_EVENTS,B_LOCK_WINDOW_FOCUS | B_NO_POINTER_HISTORY);
	}
	BButton::MouseDown(point);
}
void ColorToolItem::MouseUp(BPoint point)
{
	//***check if the point is in the Color Window coordinates
	if (Bounds().Contains(point)) 
	{
		//parentToolBar->SetEventReciver(this);
		//parentToolBar->
		//SetEventMask(B_POINTER_EVENTS,B_LOCK_WINDOW_FOCUS | B_NO_POINTER_HISTORY);
		//SetEventMask(oldEventMask);
		SetEventMask(oldEventMask);
		colorWindow->Hide();
		Invoke();

	}
	else
	{
		if (!colorWindow-> IsHidden())
		{
			BPoint screenPoint=ConvertToScreen(point);
			if (!colorWindow->Frame().Contains(screenPoint))
			{
				colorWindow->Hide();
				SetEventMask(oldEventMask);

			}
		}
		//SetEventMask(0,B_LOCK_WINDOW_FOCUS | B_NO_POINTER_HISTORY);
	
	}
	BButton::MouseUp(point);
}

void ColorToolItem::MessageReceived(BMessage *message)
{
	if (message->what == COLOR_CHANGED)
	{
		value=colorPicker->ValueAsColor();
		Invalidate();
	}
	BButton::MessageReceived(message);
}
/*status_t ColorToolItem::Invoke(BMessage *message)
{
	status_t err = B_OK;
	if (colorWindow)
	{
		if (colorWindow->Lock())
		{
			if (!colorWindow->IsHidden())
				err = BButton::Invoke(message);
			colorWindow->Unlock();
		}
		else
			err = B_ERROR;
	}
	return err;
}*/
