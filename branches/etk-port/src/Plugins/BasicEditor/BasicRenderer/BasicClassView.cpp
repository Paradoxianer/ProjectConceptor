/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "BasicClassView.h"
#include "ProjectConceptorDefs.h"
#include "PCommandManager.h"

#include <interface/Window.h>
#include <support/String.h>



BasicClassView::BasicClassView(BMessage *forMessage):BBox(BRect(0,0,100,100),"RenderView")
//:BView(BRect(0,0,100,100),"RenderView",B_FOLLOW_LEFT|B_FOLLOW_TOP,B_WILL_DRAW)
{
	viewMessage	= forMessage;
	Init();
	ValueChanged();
}
void BasicClassView::Init()
{	
	BRect 		*frame			= new BRect(0,0,100,100);
	status_t	err			 	= B_OK;
	resizing					= false;
	startMouseDown				= NULL;
	oldPpt						= NULL;
	doc							= NULL;
	outgoing					= NULL;
	incoming					= NULL;

	connecting					= 0;
	dataMessage					= new BMessage();
	patternMessage				= new BMessage();
	objectName					= new BTextControl(BRect(8,45,95,55),"Ueberschrift",NULL,"Name",new BMessage(B_C_V_NAME_CHANGED),B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP,B_WILL_DRAW | B_PULSE_NEEDED|B_FRAME_EVENTS);	

	SetBorder(B_FANCY_BORDER);
	SetLabel(objectName);	

	err = viewMessage->FindPointer("doc",(void **)&doc);
	sentTo						= new BMessenger(NULL,doc);


	err = viewMessage->FindRect("Frame",frame);
	if (err != B_OK)
		viewMessage->AddRect("Frame",Frame());
	else
	{
		
		MoveTo(frame->LeftTop());
		ResizeTo(frame->Width(),frame->Height());
	}
	if (viewMessage->FindPointer("Outgoing",(void **)&outgoing) != B_OK)
	{
		outgoing = new BList();
		viewMessage->AddPointer("Outgoing",outgoing);
	}
	if (viewMessage->FindPointer("Incoming",(void **)&incoming) != B_OK)
	{
		incoming = new BList();
		viewMessage->AddPointer("Incoming",incoming);
	}
		
	err=B_OK;
	err = viewMessage->FindFloat("xRadius",&xRadius);
	if (err!=B_OK)
	{
		xRadius=10;
		viewMessage->AddFloat("xRadius",xRadius);
	}
	err = B_OK;
	err = viewMessage->FindFloat("yRadius",&yRadius);
	if (err!=B_OK)
	{
		yRadius=10;
		viewMessage->AddFloat("yRadius",yRadius);
	}
	err = B_OK;
	if (viewMessage->FindMessage("Pattern",patternMessage));
	err = patternMessage->FindRGBColor("FillColor",&fillColor);
	if (err!=B_OK)
	{
		rgb_color tmpColor = ViewColor();
		fillColor = tmpColor;
		configMessage->AddRGBColor("FillColor",fillColor);
	}	
	err = B_OK;
	/*err = viewMessage->FindRGBColor("BorderColor",&borderColor);
	if (err!=B_OK)
	{
		rgb_color tmpColor = { 51, 51, 51, 255};
		rgb_color borderColor = tmpColor;
		viewMessage->AddRGBColor("BorderColor",borderColor);
	}*/
	
	err = B_OK;
	err = viewMessage->FindMessage("Data",dataMessage);
	if ((err == B_OK)&&(dataMessage!=NULL))
	{
		BString *name=new BString();
		err = dataMessage->FindString("Name",name);
		if (err != B_OK)
			name->Append("Object");
		objectName->SetText(_T(name->String()));		
	}


}
void BasicClassView::AttachedToWindow()
{
	BRect 		*frame	= new BRect(0,0,100,100);
	status_t	err		= B_OK;
	//SetViewColor(B_TRANSPARENT_COLOR);
//	objectName->SetFontAndColor(be_bold_font);
//	objectName->SetWordWrap(false);
//	objectName->MakeResizable(true,NULL);
	//ResizeTo(100,100);
	//objectName->MakeResizable(true,this);
	//compensate the autoresize from the textView
	objectName->SetTarget(this);
	err = viewMessage->FindRect("Frame",frame);
	if (err == B_OK)
		MoveTo(frame->LeftTop());	
}
	
void BasicClassView::MouseDown(BPoint where)
{
	BView *renderView = NULL;
	int32 i, count;
	if (startMouseDown == NULL)
	{
		bool selected;
		BView *parent=Parent();
		//Bring this child to front
		parent->RemoveChild(this);
		parent->AddChild(this);
		//** bring all selected to front??
		uint32 buttons = 0;
		BMessage *currentMsg = Window()->CurrentMessage();
		currentMsg->FindInt32("buttons", (int32 *)&buttons);
		uint32 modifiers = 0;
		currentMsg->FindInt32("modifiers", (int32 *)&modifiers);
		if (buttons & B_PRIMARY_MOUSE_BUTTON)
		{
			startMouseDown	= new BPoint(where);
			pStartMouseDown	= new BPoint(ConvertToParent(where));
			//**EventMaske setzen???
			SetMouseEventMask(B_POINTER_EVENTS, B_NO_POINTER_HISTORY | B_SUSPEND_VIEW_FOCUS | B_LOCK_WINDOW_FOCUS);
		}
		viewMessage->FindBool("selected",&selected);
/*		if (((modifiers & B_CONTROL_KEY) != 0) || (selected))
			selectMessage->AddBool("clearSelection",false);*/
		if  (!selected) 
		{
			
			
			BMessage *selectMessage=new BMessage(P_C_EXECUTE_COMMAND);
			if  ((modifiers & B_CONTROL_KEY) != 0) 
				selectMessage->AddBool("deselect",false);
			selectMessage->AddPointer("node",viewMessage);
			selectMessage->AddString("Command::Name","Select");
			sentTo->SendMessage(selectMessage);
		}
//		Invalidate();*/
		float	yOben	= Bounds().Height()/2 - triangleHeight;
		float	yUnten	= yOben + (triangleHeight*2);
		if ((where.y>=yOben)&&(where.y<=yUnten))
		{
			if ((where.x>=0)&&(where.x<=triangleHeight))
			{
				connecting=1;
			}
			else if ((where.x>=Bounds().right-triangleHeight)&&(where.x<=Bounds().right))
			{
				//then the user hit the "output-connecting-triangle "
				connecting=2;
			}
		}
		else if ( (where.y >= (Bounds().Height()-triangleHeight)) && (where.x >= (Bounds().Width()-triangleHeight)) )
		{
			resizing = true;
		}
	}
}
void BasicClassView::MouseMoved(BPoint pt, uint32 code, const BMessage *msg)
{
	if (startMouseDown)
	{
		if (!connecting)
		{
			float dx;
			float dy;
			BPoint	ppt = ConvertToParent(pt);
			if (!oldPpt)
				oldPpt	= pStartMouseDown;
			dx = ppt.x - oldPpt->x;
			dy = ppt.y - oldPpt->y;
			oldPpt	= new BPoint(ppt);
			if (!resizing)
			{
				BMessage	*mover	= new BMessage(P_C_EXECUTE_COMMAND);
				mover->AddString("Command::Name","Move");
				mover->AddFloat("dx",dx);
				mover->AddFloat("dy",dy);
				sentTo->SendMessage(mover);
			}
			else
			{
				BMessage	*resizer	= new BMessage(P_C_EXECUTE_COMMAND);
				resizer->AddString("Command::Name","Resize");
				resizer->AddFloat("dx",dx);
				resizer->AddFloat("dy",dy);
				sentTo->SendMessage(resizer);
			}
		}
		else
		{
			// make connecting Stuff
			BMessage *connecter=new BMessage(B_E_CONNECTING);
//			BMessage *connecter=new BMessage();
/*			connecter->AddInt64("undoID",when);
			connecter->AddString("undoName","Connect");*/
			connecter->AddPoint("From",*pStartMouseDown);
			connecter->AddPoint("To",ConvertToParent(pt));
			(new BMessenger(Parent()))->SendMessage(connecter);
		}
	}
}
void BasicClassView::MouseUp(BPoint where)
{
	//berechnung des Moveparameters ist falsch.. da er sich relativ auf den ausgangspunkt von View bezieht, da sich aber das View bewegt ist das falsch
	if (startMouseDown)
	{
		if (!connecting)
		{
		
/*			if (!resizing)
			{
				BPoint	pWhere	= ConvertToParent(where);
				float dx = pWhere.x - pStartMouseDown->x;
				float dy = pWhere.y - pStartMouseDown->y;
				BMessage	*mover	= new BMessage(P_C_EXECUTE_COMMAND);
				mover->AddPointer("command",moveCommand);
				mover->AddFloat("dx",dx);
				mover->AddFloat("dy",dy);
				sentTo->SendMessage(mover);
			}
			else
			{
				float dx,dy;
				resizing = false;
				BMessage	*resizer		= new BMessage(P_C_EXECUTE_COMMAND);
				resizer->AddPointer("command",resizeCommand);
				resizer->AddFloat("dx",dx);
				resizer->AddFloat("dy",dy);
				sentTo->SendMessage(resizer);	
			}*/
			resizing		= false;
		}
		else
		{
			BMessage *connecter=new BMessage(B_E_CONNECTED);
			connecter->AddPointer("From",viewMessage);
			connecter->AddPoint("To",ConvertToParent(where));
			(new BMessenger(Parent()))->SendMessage(connecter);
		}
		delete startMouseDown;
		delete pStartMouseDown;
		delete oldPpt;
		startMouseDown	= NULL;
		pStartMouseDown	= NULL;
		oldPpt			= NULL;
		connecting=false;
	}
}

void BasicClassView::LanguageChanged()
{
}

void BasicClassView::Draw(BRect updateRect)
{	
	BBox::Draw(updateRect);
	SetHighColor(tint_color(fillColor,1.35));
	float	yOben	= Bounds().Height()/2 - triangleHeight;
	float	yMitte	= yOben + triangleHeight;
	float	yUnten	= yMitte + triangleHeight;

	FillTriangle(BPoint(0,yOben),BPoint(triangleHeight,yMitte),BPoint(0,yUnten));
	FillTriangle(BPoint(Bounds().right-triangleHeight,yOben),BPoint(Bounds().right,yMitte),BPoint(Bounds().right-triangleHeight,yUnten));
	
	pattern resizePattern = { 0x55, 0x55, 0x55, 0x55, 0x55,0x55, 0x55, 0x55 };
	FillTriangle(BPoint(Bounds().right-triangleHeight,Bounds().bottom),BPoint(Bounds().right,Bounds().bottom-triangleHeight),BPoint(Bounds().right,Bounds().bottom),resizePattern);
	
	SetHighColor(tint_color(borderColor,1.35));
	StrokeTriangle(BPoint(0,yOben),BPoint(triangleHeight,yMitte),BPoint(0,yUnten));
	StrokeTriangle(BPoint(Bounds().right-triangleHeight,yOben),BPoint(Bounds().right,yMitte),BPoint(Bounds().right-triangleHeight,yUnten));

	//SetHighColor(old);
}
void BasicClassView::MessageReceived(BMessage *message)
{
	switch(message->what) 
	{
		case P_C_VALUE_CHANGED:
				ValueChanged();	
			break;
		case B_C_V_NAME_CHANGED:
		{
			BMessage	*data= new BMessage();
			viewMessage->FindMessage("Data",data);
			data->ReplaceString("Name",objectName->Text());
			viewMessage->ReplaceMessage("Data",data);
			break;
		}
		default:
			BView::MessageReceived(message);
			break;
	}
}

void BasicClassView::ValueChanged()
{
	//status_t err = B_OK;
	//** to get fast access to the value wich was changed
//	changeMessage->FindBool("which",&selected);
	bool	selected	= false;
	BRect	frame		= BRect(0,0,0,0);
	if (LockLooper())
	{
		viewMessage->FindBool("selected",&selected);
		if (selected)
		{
			SetViewColor(tint_color(fillColor,1.35));
			objectName->SetViewColor(tint_color(fillColor,1.35));
			objectName->Invalidate();
		}
		else
		{
			SetViewColor(fillColor);
			objectName->SetViewColor(fillColor);
			objectName->Invalidate();
		}
		viewMessage->FindRect("Frame",&frame);
		if (frame != Frame())
		{
			MoveTo(frame.left,frame.top);
			ResizeTo(frame.Width(),frame.Height());
			BMessage	*connection	= NULL;
			for (int32 i=0;i<outgoing->CountItems();i++)
			{
				connection = (BMessage *) outgoing->ItemAt(i);
				char	*name; 
				uint32	type; 
				int32	count; 
				int32	q=0;
				while (connection->GetInfo(B_POINTER_TYPE,q ,(const char **)&name, &type, &count) == B_OK)
				{
					if (strstr(name,"::Renderer")!=NULL)
					{
						BView	*updateView	=NULL;
						connection->FindPointer(name,(void **)&updateView);
						(new BMessenger(updateView))->SendMessage(P_C_VALUE_CHANGED);
					}
					q++;
				}
				//finde alle Views und update sie :-)
			}
			for (int32 i=0;i<incoming->CountItems();i++)
			{
				connection = (BMessage *) incoming->ItemAt(i);
				char	*name; 
				uint32	type; 
				int32	count; 
				int32	q=0;
				while (connection->GetInfo(B_POINTER_TYPE,q ,(const char **)&name, &type, &count) == B_OK)
				{
					if (strstr(name,"::Renderer")!=NULL)
					{
						BView	*updateView	=NULL;
						connection->FindPointer(name,(void **)&updateView);
						(new BMessenger(updateView))->SendMessage(P_C_VALUE_CHANGED);
					}
					q++;
				}
			}
			//Parent()->Invalidate();
		}
		Invalidate();
		UnlockLooper();
	}
}
void BasicClassView::FrameMoved(BPoint new_position)
{
	status_t err=B_OK;
/*	err = viewMessage->ReplaceRect("frame",Frame());
	if (err != B_OK)
		viewMessage->AddRect("frame",Frame());*/
}

void BasicClassView::FrameResized(float new_width, float new_height)
{
}
