#include "ClassRenderer.h"
#include "ProjectConceptorDefs.h"
//#include "PCommandManager.h"

#include <math.h>


#include <interface/Font.h>
#include <interface/View.h>
#include <interface/GraphicsDefs.h>
//#include <interface/InterfaceDefs.h>*/
#include <interface/Window.h>

#include <support/String.h>




ClassRenderer::ClassRenderer(GraphEditor *parentEditor,BMessage *forContainer):Renderer(parentEditor,forContainer)
{
	TRACE();
	Init();
	ValueChanged();
}
void ClassRenderer::Init()
{	
	TRACE();
	status_t	err			 	= B_OK;
	resizing					= false;
	startMouseDown				= NULL;
	oldPt						= NULL;
	doc							= NULL;
	outgoing					= NULL;
	incoming					= NULL;
	
	xRadius						= 10;
	yRadius						= 10;
	frame						= BRect(0,0,0,0);
	selected					= false;
	font						= new BFont();
	penSize						= 1.0;
	connecting					= 0;

	BMessage*	editMessage		= new BMessage(P_C_EXECUTE_COMMAND);
	editMessage->AddPointer("node",container);
	editMessage->AddString("name","Name");
	editMessage->AddString("subgroup","Data");
	editMessage->AddString("Command::Name","ChangeValue");
	editMessage->AddInt32("type",B_STRING_TYPE);
	name						= new StringRenderer(editor,"",BRect(0,0,100,100), editMessage);
	err 						= container->FindPointer("doc",(void **)&doc);
	sentTo						= new BMessenger(NULL,doc);
	if (container->FindFloat("xRadius",&xRadius) != B_OK)
		container->AddFloat("xRadius",7.0);
	if (container->FindFloat("yRadius",&yRadius) != B_OK)
		container->AddFloat("yRadius",7.0);
	PRINT_OBJECT(*container);
}

	
void ClassRenderer::MouseDown(BPoint where)
{
	if (name->Frame().Contains(where))
	{
		name->MouseDown(where);
	}
	else if (startMouseDown == NULL)
	{
		uint32 buttons = 0;
		uint32 modifiers = 0;
/*		BPoint	tmpPoint;
		editor->GetMouse(&tmpPoint,&buttons,true);*/
		BMessage *currentMsg = editor->Window()->CurrentMessage();
		currentMsg->FindInt32("buttons", (int32 *)&buttons);
		currentMsg->FindInt32("modifiers", (int32 *)&modifiers);
		if (buttons & B_PRIMARY_MOUSE_BUTTON)
		{
			editor->BringToFront(this);
			startMouseDown	= new BPoint(where);
			startLeftTop	= new BPoint(frame.LeftTop());
			editor->SetMouseEventMask(B_POINTER_EVENTS, B_NO_POINTER_HISTORY | B_SUSPEND_VIEW_FOCUS | B_LOCK_WINDOW_FOCUS);
			if  (!selected) 
			{
				BMessage *selectMessage=new BMessage(P_C_EXECUTE_COMMAND);
				if  ((modifiers & B_SHIFT_KEY) != 0) 
					selectMessage->AddBool("deselect",false);
				selectMessage->AddPointer("node",container);
				selectMessage->AddString("Command::Name","Select");
				sentTo->SendMessage(selectMessage);
			}
			float	yOben	= frame.top+frame.Height()/2 - triangleHeight;
			float	yUnten	= yOben + (triangleHeight*2);
			if ((where.y>=yOben)&&(where.y<=yUnten))
			{
				if ((where.x >= frame.left)&&(where.x <= (frame.left+(triangleHeight*2))))
				{
					connecting=1;
				}
				else if ((where.x >= frame.right-(triangleHeight*2))&&(where.x <= frame.right))
				{
					//then the user hit the "output-connecting-triangle "
					connecting=2;
				}
			}
			else if ( (where.y >= (frame.bottom-triangleHeight)) && (where.x >= (frame.right-triangleHeight)) )
			{
				resizing = true;
			}
		}
		else if (buttons & B_SECONDARY_MOUSE_BUTTON )
		{
			editor->SendToBack(this);
		}
	}
}
void ClassRenderer::MouseMoved(BPoint pt, uint32 code, const BMessage *msg)
{
	if (startMouseDown)
	{
		if (!connecting)
		{
			float dx	= 0;
			float dy	= 0;
			if (!oldPt)
				oldPt	= startMouseDown;
			if (editor->GridEnabled())
			{
			
				float newPosX = startLeftTop->x + (pt.x-startMouseDown->x);
				float newPosY = startLeftTop->y + (pt.y-startMouseDown->y);
				newPosX = newPosX - fmod(newPosX,editor->GridWidth());
				newPosY = newPosY - fmod(newPosY,editor->GridWidth());
				dx = newPosX - frame.left;
				dy = newPosY -frame.top ;
			}
			else
			{
				dx = pt.x - oldPt->x;
				dy = pt.y - oldPt->y;				
			}
			oldPt	= new BPoint(pt);
			if (!resizing)
			{
				BList	*selected	= doc->GetSelected();
				BPoint	deltaPoint(dx,dy);
				selected->DoForEach(MoveAll,&deltaPoint);
				editor->ValueChanged();
			}
			else
			{
				BList	*selected	= doc->GetSelected();
				BPoint	deltaPoint(dx,dy);
				selected->DoForEach(ResizeAll,&deltaPoint);
				editor->ValueChanged();
			}
		}
		else
		{
			// make connecting Stuff
			BMessage *connecter=new BMessage(G_E_CONNECTING);
//			BMessage *connecter=new BMessage();
/*			connecter->AddInt64("undoID",when);
			connecter->AddString("undoName","Connect");*/
			connecter->AddPoint("From",*startMouseDown);
			connecter->AddPoint("To",pt);
			(new BMessenger((BView *)editor))->SendMessage(connecter);
		}
	}
}
void ClassRenderer::MouseUp(BPoint where)
{
	//berechnung des Moveparameters ist falsch.. da er sich relativ auf den ausgangspunkt von View bezieht, da sich aber das View bewegt ist das falsch
	if (startMouseDown)
	{
		if (!connecting)
		{
			float dx = where.x - startMouseDown->x;
			float dy = where.y - startMouseDown->y;
			if (!resizing)
			{
				BList		*selected	= doc->GetSelected();
				BPoint deltaPoint(-dx,-dy);
				selected->DoForEach(MoveAll,&deltaPoint);
				BMessage	*mover	= new BMessage(P_C_EXECUTE_COMMAND);
				mover->AddString("Command::Name","Move");
				mover->AddFloat("dx",dx);
				mover->AddFloat("dy",dy);
				sentTo->SendMessage(mover);
			}
			else
			{
				BList	*selected	= doc->GetSelected();
				BPoint	deltaPoint(-dx,-dy);
				selected->DoForEach(ResizeAll,&deltaPoint);
				BMessage	*resizer	= new BMessage(P_C_EXECUTE_COMMAND);
				resizer->AddString("Command::Name","Resize");
				resizer->AddFloat("dx",dx);
				resizer->AddFloat("dy",dy);
				sentTo->SendMessage(resizer);	
			}
			resizing		= false;
		}
		else
		{
			BMessage *connecter=new BMessage(G_E_CONNECTED);
			if (connecting == 2)
			{
				connecter->AddPointer("From",container);
				connecter->AddPoint("To",where);
			}
			else
			{
				connecter->AddPoint("From",where);
				connecter->AddPointer("To",container);
			}
			(new BMessenger((BView *)editor))->SendMessage(connecter);
		}
		delete startMouseDown;
		delete oldPt;
		startMouseDown	= NULL;
		oldPt			= NULL;
		connecting=false;
	}
}

void ClassRenderer::LanguageChanged()
{
}

void ClassRenderer::Draw(BView *drawOn, BRect updateRect)
{	
	BRect shadowFrame = frame;
	rgb_color	drawColor;
	shadowFrame.OffsetBy(3,3);
	drawOn->SetPenSize(penSize);
	drawOn->SetHighColor(0,0,0,77);
	drawOn->FillRoundRect(shadowFrame, xRadius, yRadius);
	if (!selected)
		drawColor = fillColor;

	else
		drawColor = tint_color(fillColor,1.35);
	drawOn->SetHighColor(drawColor);
	drawOn->FillRoundRect(frame, xRadius, yRadius);
	drawOn->SetPenSize(1.0);
	drawOn->BeginLineArray(20);
	drawOn->AddLine(BPoint(frame.left+xRadius,frame.top+1),BPoint(frame.right-xRadius,frame.top+1),tint_color(drawColor,0));
	drawOn->AddLine(BPoint(frame.left,frame.top+2),BPoint(frame.right,frame.top+2),tint_color(drawColor,0.05));
	drawOn->AddLine(BPoint(frame.left,frame.top+3),BPoint(frame.right,frame.top+3),tint_color(drawColor,0.1));
	drawOn->AddLine(BPoint(frame.left,frame.top+4),BPoint(frame.right,frame.top+4),tint_color(drawColor,0.15));	
	drawOn->AddLine(BPoint(frame.left,frame.top+5),BPoint(frame.right,frame.top+5),tint_color(drawColor,0.2));
	drawOn->AddLine(BPoint(frame.left,frame.top+6),BPoint(frame.right,frame.top+6),tint_color(drawColor,0.25));	
	drawOn->AddLine(BPoint(frame.left,frame.top+7),BPoint(frame.right,frame.top+7),tint_color(drawColor,0.3));
	drawOn->AddLine(BPoint(frame.left,frame.top+8),BPoint(frame.right,frame.top+8),tint_color(drawColor,0.35));
	drawOn->AddLine(BPoint(frame.left,frame.top+9),BPoint(frame.right,frame.top+9),tint_color(drawColor,0.4));
	drawOn->AddLine(BPoint(frame.left,frame.top+10),BPoint(frame.right,frame.top+10),tint_color(drawColor,0.45));
	drawOn->AddLine(BPoint(frame.left,frame.top+11),BPoint(frame.right,frame.top+11),tint_color(drawColor,0.5));
	drawOn->AddLine(BPoint(frame.left,frame.top+12),BPoint(frame.right,frame.top+12),tint_color(drawColor,0.55));
	drawOn->AddLine(BPoint(frame.left,frame.top+13),BPoint(frame.right,frame.top+13),tint_color(drawColor,0.6));
	drawOn->AddLine(BPoint(frame.left,frame.top+14),BPoint(frame.right,frame.top+14),tint_color(drawColor,0.65));	
	drawOn->AddLine(BPoint(frame.left,frame.top+15),BPoint(frame.right,frame.top+15),tint_color(drawColor,0.7));
	drawOn->AddLine(BPoint(frame.left,frame.top+16),BPoint(frame.right,frame.top+16),tint_color(drawColor,0.75));	
	drawOn->AddLine(BPoint(frame.left,frame.top+17),BPoint(frame.right,frame.top+17),tint_color(drawColor,0.8));
	drawOn->AddLine(BPoint(frame.left,frame.top+18),BPoint(frame.right,frame.top+18),tint_color(drawColor,0.85));
	drawOn->AddLine(BPoint(frame.left,frame.top+19),BPoint(frame.right,frame.top+19),tint_color(drawColor,0.9));
	drawOn->AddLine(BPoint(frame.left,frame.top+20),BPoint(frame.right,frame.top+20),tint_color(drawColor,0.95));
	drawOn->EndLineArray();
	#ifdef B_ZETA_VERSION_1_0_0
		drawOn->SetHighColor(ui_color(B_UI_DOCUMENT_LINK_COLOR));	
	#else
		drawOn->SetHighColor(0,0,255,255);	
	#endif 
	float	yOben	= frame.top+frame.Height()/2 - triangleHeight;
	float	yMitte	= yOben + triangleHeight;
	float	yUnten	= yMitte + triangleHeight;

	drawOn->FillTriangle(BPoint(frame.left,yOben),BPoint(frame.left+triangleHeight,yMitte),BPoint(frame.left,yUnten));
	drawOn->FillTriangle(BPoint(frame.right-triangleHeight,yOben),BPoint(frame.right,yMitte),BPoint(frame.right-triangleHeight,yUnten));
	//pattern resizePattern = { 0x55, 0x55, 0x55, 0x55, 0x55,0x55, 0x55, 0x55 };
	drawOn->FillTriangle(BPoint(frame.right-triangleHeight,frame.bottom),BPoint(frame.right,frame.bottom-triangleHeight),BPoint(frame.right,frame.bottom),B_MIXED_COLORS);

	drawOn->SetHighColor(borderColor);
	drawOn->SetPenSize(penSize);
	drawOn->StrokeRoundRect(frame, xRadius, yRadius);
	drawOn->SetPenSize(1.0);
	drawOn->StrokeTriangle(BPoint(frame.left,yOben),BPoint(frame.left+triangleHeight,yMitte),BPoint(frame.left,yUnten));
	drawOn->StrokeTriangle(BPoint(frame.right-triangleHeight,yOben),BPoint(frame.right,yMitte),BPoint(frame.right-triangleHeight,yUnten));
	name->Draw(drawOn,updateRect);
}

void ClassRenderer::MessageReceived(BMessage *message)
{
	switch(message->what) 
	{
		case P_C_VALUE_CHANGED:
				ValueChanged();	
			break;
	}
}

void ClassRenderer::ValueChanged()
{
	TRACE();
	BMessage	*pattern		= new BMessage();
	BMessage	*messageFont	= new BMessage();
	BMessage	*data			= new BMessage();
	char		*newName;

	container->FindRect("Frame",&frame);
	container->FindBool("selected",&selected);
	container->FindFloat("xRadius",&xRadius);
	container->FindFloat("yRadius",&yRadius);
	container->FindMessage("Pattern",pattern);
	container->FindMessage("Font",messageFont);
	container->FindMessage("Data",data);
	
	pattern->FindRGBColor("FillColor",&fillColor);
	pattern->FindRGBColor("BorderColor",&borderColor);
	pattern->FindFloat("PenSize",&penSize);
	data->FindString("Name",(const char **)&newName);
	name->SetString(newName);
	name->SetFrame(BRect(frame.left+2,frame.top+2,frame.right,frame.top+12));
}

BRect ClassRenderer::Frame( void )
{
	return frame;
}

bool  ClassRenderer::Caught(BPoint where)
{
	return frame.Contains(where);
}
bool  ClassRenderer::MoveAll(void *arg,void *deltaPoint)
{
	BMessage	*node	=(BMessage *)arg;
	BRect		rect;
	BPoint		*delta	= (BPoint *)deltaPoint;
	node->FindRect("Frame",&rect);
	rect.OffsetBy(delta->x,delta->y);
	node->ReplaceRect("Frame",rect);
	return false;
}
bool  ClassRenderer::ResizeAll(void *arg,void *deltaPoint)
{
	BMessage	*node	=(BMessage *)arg;
	BRect		rect;
	BPoint		*delta	= (BPoint *)deltaPoint;
	node->FindRect("Frame",&rect);
	rect.right		+=	delta->x;
	rect.bottom		+=	delta->y;
	if ( (rect.IsValid()) && (rect.Width()>20) && ((rect.Height()>20)) )
		node->ReplaceRect("Frame",rect);
	return false;
}
