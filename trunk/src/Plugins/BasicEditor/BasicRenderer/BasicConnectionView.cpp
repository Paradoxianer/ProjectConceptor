/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "BasicConnectionView.h"
#include <interface/Window.h>
#include <stdio.h>


BasicConnectionView::BasicConnectionView(BMessage *forMessage):BView(BRect(0,0,100,100),"ConnectionView",B_FOLLOW_LEFT|B_FOLLOW_TOP,B_WILL_DRAW)
{
	viewMessage	= forMessage;
	Init();
}
void BasicConnectionView::Init()
{	
	from			= NULL;
	to				= NULL;
	fromPoint		= new BPoint(0,0);
	toPoint			= new BPoint(0,0);
	mirrorX			= false;
	mirrorY 		= false;
	connectionName	= new BTextControl(BRect(0,0,100,55),"Name",NULL,"Unbenannt",new BMessage(B_C_NAME_CHANGED));
	AddChild(connectionName);
	BList		*outgoing	= NULL;
	BList		*incoming	= NULL;
	BMessage	*data		= new BMessage();
	viewMessage->FindPointer("From",(void **)&from);
	viewMessage->FindPointer("To",(void **)&to);
	if (from->FindPointer("Outgoing",(void **)&outgoing) != B_OK)
	{
		outgoing = new BList();
		from->AddPointer("Outgoing",outgoing);
	}
	outgoing->AddItem(viewMessage);
	if (to->FindPointer("Incoming",(void **)&incoming) != B_OK)
	{
		incoming = new BList();
		to->AddPointer("Incoming",incoming);
	}
	incoming->AddItem(viewMessage);
	if (viewMessage->FindMessage("Data",data) != B_OK)
	{
		data->AddString("Name","Unbenannt");
		viewMessage->AddMessage("Data",data);
	}
	const char* name;
	if (data->FindString("Name",&name)==B_OK)
		connectionName->SetText(name);
}
void BasicConnectionView::AttachedToWindow()
{
	BRect	*fromRect	= new BRect();
	BRect	*toRect		= new BRect();
	SetViewColor(B_TRANSPARENT_COLOR);
	/*tv->SetAlignment(B_ALIGN_CENTER);
	tv->SetFontAndColor(be_bold_font);*/
	//tv->MakeResizable(true,this);
	from->FindRect("Frame",fromRect);
	to->FindRect("Frame",toRect);
	BPoint	fromPt		= BPoint(fromRect->right,fromRect->top+(fromRect->Height()/2));
	BPoint	toPt		= BPoint(toRect->left,toRect->top+(toRect->Height()/2));

	BRect ownRect= BRect(fromPt,toPt);
	if (ownRect.left>ownRect.right)
	{

		mirrorX = true;
		float c = ownRect.left;
		ownRect.left=ownRect.right;
		ownRect.right=c;
	}
	if (ownRect.top>ownRect.bottom)
	{
		mirrorY = true;
		float c = ownRect.top;
		ownRect.top=ownRect.bottom;
		ownRect.bottom=c;
	}
	MoveTo(ownRect.left,ownRect.top);
	ResizeTo(ownRect.Width(),ownRect.Height());

	*fromPoint	= ConvertFromParent(fromPt);
	*toPoint		= ConvertFromParent(toPt);	

	float dx=Bounds().Width()-connectionName->Bounds().Width();
	float dy=Bounds().Height()-connectionName->Bounds().Height();
	connectionName->MoveTo(dx/2,dy/2);
	connectionName->SetTarget(this);
}
	
void BasicConnectionView::MouseDown(BPoint where)
{
	BView::MouseDown(where);
}
void BasicConnectionView::MouseMoved(BPoint pt, uint32 code, const BMessage *msg)
{
	BView::MouseMoved(pt,code,msg);
}
void BasicConnectionView::MouseUp(BPoint where)
{
	BView::MouseUp(where);
}

void BasicConnectionView::LanguageChanged()
{
}

void BasicConnectionView::Draw(BRect updateRect)
{	//viewMessage->PrintToStream();
	
//	printf("BasicConnectionView::Draw\n");
	/*SetPenSize(0.5);
	SetHighColor(55,55,55,255);
	StrokeRect(Bounds());*/
	
	SetHighColor(255,0,0,255);
	SetPenSize(2.0);
	/*float c;
	BPoint left=Bounds().LeftTop();
	BPoint right=Bounds().RightBottom();
	if (mirrorX)
	{
		c=left.x;
		left.x=right.x;
		right.x=c;
	}
	if (mirrorY)
	{
		c=left.y;
		left.y=right.y;
		right.y=c;
	}*/
//	StrokeLine(fromDrawer->Frame().RightBottom(),toDrawer->Frame().LeftTop());
//	StrokeLine(left,right);
	StrokeLine(	*fromPoint,*toPoint);

}
void BasicConnectionView::MessageReceived(BMessage *message)
{
	switch(message->what) 
	{
		case P_C_VALUE_CHANGED:
				ValueChanged(message);	
			break;
		case B_C_NAME_CHANGED:
		{
			BMessage	*data= new BMessage();
			viewMessage->FindMessage("Data",data);
			data->ReplaceString("Name",connectionName->Text());
			viewMessage->ReplaceMessage("Data",data);
			break;
		}
		default:
			BView::MessageReceived(message);
			break;
	}
}

void BasicConnectionView::ValueChanged(BMessage *changeMessage)
{
	if (LockLooper())
	{
		BRect	*fromRect	= new BRect(0,0,0,0);
		BRect	*toRect		= new BRect(0,0,0,0);
		from->FindRect("Frame",fromRect);
		to->FindRect("Frame",toRect);
		BPoint	fromPt		= BPoint(fromRect->right,fromRect->top+(fromRect->Height()/2));
		BPoint	toPt		= BPoint(toRect->left,toRect->top+(toRect->Height()/2));
		BRect ownRect= BRect(fromPt,toPt);
		if (ownRect.left>ownRect.right)
		{	
			mirrorX = true;
			float c = ownRect.left;
			ownRect.left=ownRect.right;
			ownRect.right=c;
		}
		if (ownRect.top>ownRect.bottom)
		{
			mirrorY = true;
			float c = ownRect.top;
			ownRect.top=ownRect.bottom;
			ownRect.bottom=c;
		}
		if (ownRect.Width()<(connectionName->Bounds().Width()+2))
		{
			float dx = (connectionName->Bounds().Width()-ownRect.Width())/2+2;
			ownRect.left-=dx;
			ownRect.right+=dx;
		}
		if (ownRect.Height()<(connectionName->Bounds().Height()+2))
		{
			float dy = (connectionName->Bounds().Height()-ownRect.Height())/2+2;
			ownRect.top-=dy;
			ownRect.bottom+=dy;
		}
		MoveTo(ownRect.left,ownRect.top);
		ResizeTo(ownRect.Width(),ownRect.Height());	
		*fromPoint	= ConvertFromParent(fromPt);
		*toPoint		= ConvertFromParent(toPt);	
		float dx=ownRect.Width()-connectionName->Bounds().Width();
		float dy=ownRect.Height()-connectionName->Bounds().Height();
		connectionName->MoveTo(dx/2,dy/2);
		Invalidate();
		UnlockLooper();
	}
}
void BasicConnectionView::FrameMoved(BPoint new_position)
{
	status_t err = B_OK;
}

void BasicConnectionView::FrameResized(float new_width, float new_height)
{
	status_t err = B_OK;
	err = viewMessage->ReplaceRect("Frame",Frame());
	if (err != B_OK)
		viewMessage->AddRect("Frame",Frame());
}
