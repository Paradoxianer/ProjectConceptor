#include "GroupRenderer.h"
#include "ProjectConceptorDefs.h"

#include <math.h>

#include <interface/Font.h>
#include <interface/View.h>
#include <interface/GraphicsDefs.h>

#include <interface/Window.h>

#include <support/String.h>
#include "AttributRenderer.h"
#include "PDocument.h"




GroupRenderer::GroupRenderer(GraphEditor *parentEditor, Renderer *parentRenderer, BMessage *forContainer):Renderer(parentEditor, parentRenderer, forContainer)
{
	TRACE();
	Init();
	ValueChanged();
}
void GroupRenderer::Init()
{
	TRACE();
	status_t	err			 		= B_OK;
	int32		i					= 0;
	resizing						= false;
	startMouseDown					= NULL;
	scale							= 1.0;
	oldPt							= NULL;
	doc								= NULL;
	allNodes						= NULL;
	allConnections					= NULL;
	mouseReciver					= NULL;
	xRadius							= 10;
	yRadius							= 10;
	frame							= BRect(0,0,0,0);
	selected						= false;
	font							= new BFont();
	penSize							= 1.0;
	connecting						= 0;
	renderer						= new BList();
	father							= NULL;

	BMessage*		node			= NULL;

	//Prepare Message to be sent when Name was changed
	BMessage*		editMessage		= new BMessage(P_C_EXECUTE_COMMAND);
	editMessage->AddPointer("node",container);
	editMessage->AddString("Command::Name","ChangeValue");
	BMessage	*valueContainer	= new BMessage();
	valueContainer->AddString("name","Name");
	valueContainer->AddString("subgroup","Data");
	valueContainer->AddInt32("type",B_STRING_TYPE);
	editMessage->AddMessage("valueContainer",valueContainer);
	//add this Message to the StringRenderer
	name						= new StringRenderer(editor,"",BRect(0,0,100,100), editMessage);
	
	//find the document the node belongs to
	err 						= container->FindPointer("doc",(void **)&doc);
	//generate a Sender
	sentTo						= new BMessenger(NULL,doc);
	if (container->FindFloat("xRadius",&xRadius) != B_OK)
		container->AddFloat("xRadius",7.0);
	if (container->FindFloat("yRadius",&yRadius) != B_OK)
		container->AddFloat("yRadius",7.0);
	if (container->FindPointer("allNodes", (void **)&allNodes) !=B_OK)
		container->AddPointer("allNodes",allNodes=new BList());
	if (container->FindPointer("allConnections", (void **)&allConnections) !=B_OK)
		container->AddPointer("allConnections",allConnections=new BList());
	for (i=0;i<allNodes->CountItems();i++)
	{
		node = (BMessage *)allNodes->ItemAt(i);
		if (FindRenderer(node) == NULL)
			InsertRenderObject(node);
	}
	for (i=0;i<allConnections->CountItems();i++)
	{
		node = (BMessage *)allConnections->ItemAt(i);
		if (FindRenderer(node) == NULL)
			InsertRenderObject(node);
	}
	PRINT_OBJECT(*container);
}


void GroupRenderer::MouseDown(BPoint where)
{
	bool		found			= false;
	Renderer*	tmpRenderer		= NULL;
	BPoint		scaledWhere;
	scaledWhere.x	= where.x / scale;
	scaledWhere.y	= where.y / scale;
	if (name->Caught(where))
	{
		name->MouseDown(where);
		found	= true;
	}
	for (int32 i=(renderer->CountItems()-1);((!found) && (i>=0) );i--)
	{
		if (((Renderer*)renderer->ItemAt(i))->Caught(scaledWhere))
		{
			mouseReciver = (Renderer*)renderer->ItemAt(i);
			mouseReciver->MouseDown(scaledWhere);
			found			= true;
		}
	}
	if ((!found) && (startMouseDown == NULL))
	{
		uint32 buttons = 0;
		uint32 modifiers = 0;
		BMessage *currentMsg = editor->Window()->CurrentMessage();
		currentMsg->FindInt32("buttons", (int32 *)&buttons);
		currentMsg->FindInt32("modifiers", (int32 *)&modifiers);
		if (buttons & B_PRIMARY_MOUSE_BUTTON)
		{
			if (parent)
				((GroupRenderer *)parent)->BringToFront(this);
			else 
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
			if (parent)
				((GroupRenderer *)parent)->SendToBack(this);
			else 
				editor->SendToBack(this);
		}
	}
}
void GroupRenderer::MouseMoved(BPoint pt, uint32 code, const BMessage *msg)
{
	BPoint		scaledWhere;
	scaledWhere.x	= pt.x / scale;
	scaledWhere.y	= pt.y / scale;
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
				BPoint	deltaPoint(dx,dy);
				BList	*renderer	= editor->RenderList();
				renderer->DoForEach(MoveAll, &deltaPoint);
				editor->Invalidate();
			}
			else
			{
				BPoint	deltaPoint(dx,dy);
				BList	*renderer	= editor->RenderList();
				renderer->DoForEach(ResizeAll, &deltaPoint);
				editor->Invalidate();
			}
		}
		else
		{
			// make connecting Stuff
			BMessage *connecter=new BMessage(G_E_CONNECTING);
			connecter->AddPoint("From",*startMouseDown);
			connecter->AddPoint("To",pt);
			(new BMessenger((BView *)editor))->SendMessage(connecter);
		}
	}
	else if (mouseReciver != NULL)
	{
		mouseReciver->MouseMoved(scaledWhere,code,msg);
	}
}

void GroupRenderer::MouseUp(BPoint where)
{
	bool		found			= false;
	Renderer*	tmpRenderer		= NULL;
	BPoint		scaledWhere;
	scaledWhere.x	= where.x / scale;
	scaledWhere.y	= where.y / scale;
	if ( (!found) && (startMouseDown) )
	{
		if (!connecting)
		{
			float dx = where.x - startMouseDown->x;
			float dy = where.y - startMouseDown->y;
			if (editor->GridEnabled())
			{
				float newPosX = startLeftTop->x + dx;
				float newPosY = startLeftTop->y + dy;
				newPosX = newPosX - fmod(newPosX,editor->GridWidth());
				newPosY = newPosY - fmod(newPosY,editor->GridWidth());
				dx = newPosX-startLeftTop->x;
				dy = newPosY-startLeftTop->y;
			}
			if (!resizing)
			{
				//BList		*selected	= doc->GetSelected();
				BPoint deltaPoint(-dx,-dy);
//				selected->DoForEach(MoveAll,&deltaPoint);
				BMessage	*mover	= new BMessage(P_C_EXECUTE_COMMAND);
				mover->AddString("Command::Name","Move");
				mover->AddFloat("dx",dx);
				mover->AddFloat("dy",dy);
				sentTo->SendMessage(mover);
			}
			else
			{
			//	BList	*selected	= doc->GetSelected();
				BPoint	deltaPoint(-dx,-dy);
			//	selected->DoForEach(ResizeAll,&deltaPoint);
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
	else if (mouseReciver != NULL)
	{
		mouseReciver->MouseUp(scaledWhere);
		mouseReciver = NULL;
	}
}


void GroupRenderer::Draw(BView *drawOn, BRect updateRect)
{
	BRect		shadowFrame = frame;
	bool		fitIn		= true;
	Renderer*	tmpRenderer	= NULL;

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
/*	for (int32 i=0;(fitIn)&& (i<attributes->size());i++)
	{
		tmpRenderer=(*attributes)[i];
		if (frame.Contains(tmpRenderer->Frame()))
			tmpRenderer->Draw(drawOn,updateRect);
		else
			fitIn=false;
	}
	if (!fitIn)
		drawOn->DrawString("...",BPoint(frame.left+triangleHeight+2,frame.bottom-(yRadius/3)));*/
	for (int32 i=0;(i<renderer->CountItems());i++)
	{
		tmpRenderer = (Renderer *) renderer->ItemAt(i);
		tmpRenderer->Draw(drawOn, tmpRenderer->Frame());
	}
	//renderer->DoForEach(DrawRenderer,editor);
}

void GroupRenderer::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
		case P_C_VALUE_CHANGED:
				ValueChanged();
			break;
	}
}

void GroupRenderer::ValueChanged()
{
	TRACE();
	BList		*changedNodes	= doc->GetChangedNodes();
	Renderer	*painter		= NULL;
	BMessage	*pattern		= new BMessage();
	BMessage	*messageFont	= new BMessage();
	BMessage	*node			= NULL;

	BMessage	*data			= new BMessage();
	char		*newName		= NULL;

	char		*attribName		= NULL;
	BMessage	*attribMessage	= new BMessage();
	uint32		type			= B_ANY_TYPE;
	int32		count			= 0;
	bool		found			= false;
	void		*parent			= NULL;
	void		*pointer		= NULL;
	
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
	name->SetFrame(BRect(frame.left+(xRadius/3),frame.top+(yRadius/3),frame.right-(xRadius/3),frame.top+12));
	for (int32 i=0;i<changedNodes->CountItems();i++)
	{
		node = (BMessage *)changedNodes->ItemAt(i);
		if ( (node->FindPointer("Parent",(void **)&parent) == B_OK) && (parent == this) )
		{
			if ((allConnections->HasItem(node))||(allNodes->HasItem(node)))
			{
				//**maby we could speed this up
				painter = FindRenderer(node);
				painter->ValueChanged();
			}
			else
				RemoveRenderer(FindRenderer(node));
			//we have to find the father ... wich should be invalidatet :( :) maby there is a nother faster way to do this ..
			while ((((Renderer *)parent)->GetMessage())->FindPointer("Parent",(void **)&parent)==B_OK)
				father = (Renderer *)parent;
		}
		else
		{
			//check if this node is in the node or connection list because it it is not it´s a nodd frome a subgroup or it was deleted
			if ((allConnections->HasItem(node))||(allNodes->HasItem(node)))
				InsertRenderObject(node);
			else
				RemoveRenderer(FindRenderer(node));
		}
	}
}

BRect GroupRenderer::Frame( void )
{
	return frame;
}

bool  GroupRenderer::Caught(BPoint where)
{
	return frame.Contains(where);
}
//**implement this
void  GroupRenderer::SetFrame(BRect newFrame)
{
}

bool  GroupRenderer::MoveAll(void *arg,void *deltaPoint)
{
	BPoint		*delta	= (BPoint *)deltaPoint;
	Renderer	*renderer	= (Renderer*)arg;
	if (renderer->Selected())
		renderer->MoveBy(delta->x,delta->y);
	return false;
}

bool  GroupRenderer::ResizeAll(void *arg,void *deltaPoint)
{
	BPoint		*delta	= (BPoint *)deltaPoint;
	Renderer	*renderer	= (Renderer*)arg;
	if (renderer->Selected())
		renderer->ResizeBy(delta->x,delta->y);
	return false;
}

void GroupRenderer::MoveBy(float dx,float dy)
{
	frame.OffsetBy(dx,dy);
	name->MoveBy(dx,dy);
	for (int32 i=0;i<renderer->CountItems();i++)
	{
		((Renderer *)renderer->ItemAt(i))->MoveBy(dx,dy);
	}


}

void GroupRenderer::ResizeBy(float dx,float dy)
{
	if ((frame.right+dx-frame.left) > 70)
		frame.right		+= dx;
	if  ((frame.bottom+dy-frame.top) > 30)
		frame.bottom	+= dy;
	name->ResizeBy(dy,dy);
}

void GroupRenderer::InsertAttribute(char *attribName,BMessage *attribute,int32 count)
{
	char	*realName	= NULL;
	BRect	attributeRect;
/*	if (attributes->empty())
	{
		attributeRect = BRect(frame.left+triangleHeight+2,name->Frame().bottom+6,frame.right-triangleHeight-2,frame.bottom-2);
	}
	else
	{
		Renderer* lastRenderer = (*attributes)[attributes->size()-1];
		attributeRect = BRect(frame.left+triangleHeight+2,lastRenderer->Frame().bottom+1,frame.right-triangleHeight-2,frame.bottom-2);
	}*/
	BMessage*	editMessage		= new BMessage(P_C_EXECUTE_COMMAND);
	editMessage->AddPointer("node",container);
	editMessage->AddString("Command::Name","ChangeValue");
	BMessage*	valueContainer	= new BMessage();
	valueContainer->AddString("subgroup","Data");
	valueContainer->AddString("subgroup",attribName);
	editMessage->AddMessage("valueContainer",valueContainer);
	delete valueContainer;
	BMessage*	removeAttribMessage		= new BMessage(P_C_EXECUTE_COMMAND);
	removeAttribMessage->AddPointer("node",container);
	removeAttribMessage->AddString("Command::Name","RemoveAttribute");
	valueContainer	= new BMessage();
	valueContainer->AddString("subgroup","Data");
	valueContainer->AddString("name",attribName);
	valueContainer->AddInt32("index",count);
	removeAttribMessage->AddMessage("valueContainer",valueContainer);

//	attribute->FindString("Name",(const char **)&realName);
//	Renderer	*testRenderer	= new StringRenderer(editor,realName,attributeRect, editMessage);
	Renderer	*testRenderer	= new AttributRenderer(editor,attribute,attributeRect, editMessage,removeAttribMessage);
//	attributes->push_back(testRenderer);

}

void GroupRenderer::InsertRenderObject(BMessage *node)
{
	TRACE();
	Renderer	*newRenderer = NULL;
	void		*parentPointer = NULL;	
	void		*tmpDoc	= NULL;
	if (node->FindPointer("doc",&tmpDoc)==B_OK)
		node->ReplacePointer("doc",doc);
	else
		node->AddPointer("doc",doc);
	switch(node->what)
	{
		case P_C_CLASS_TYPE:
			newRenderer	= new  ClassRenderer(editor,this,node);
		break;
		case P_C_GROUP_TYPE:
			newRenderer = new GroupRenderer(editor,this,node);
		break;
		case P_C_CONNECTION_TYPE:
			newRenderer	= new ConnectionRenderer(editor,this,node);
		break;
	}
	if (node->FindPointer("Parent",&parentPointer) != B_OK)
		node->AddPointer("Parent",this);
	AddRenderer(newRenderer);

}


void GroupRenderer::AddRenderer(Renderer* newRenderer)
{
	TRACE();
	renderer->AddItem(newRenderer);
	activRenderer = newRenderer;
}

void GroupRenderer::RemoveRenderer(Renderer *wichRenderer)
{
	TRACE();
	if (activRenderer == wichRenderer)
		activRenderer = NULL;
	if (mouseReciver == wichRenderer)
		mouseReciver = NULL;
	renderer->RemoveItem(wichRenderer);
	delete wichRenderer;
}

Renderer* GroupRenderer::FindRenderer(BPoint where)
{
	TRACE();
	Renderer *currentRenderer = NULL;
	bool found	=	false;
	for (int32 i=(renderer->CountItems()-1);((!found) && (i>=0));i--)
	{
		if (((Renderer*)renderer->ItemAt(i))->Caught(where))
		{
			found			= true;
			currentRenderer	= (Renderer*)renderer->ItemAt(i);
		}
	}
	return currentRenderer;
}

Renderer* GroupRenderer::FindNodeRenderer(BPoint where)
{
	TRACE();
	Renderer *currentRenderer = NULL;
	bool found	=	false;
	for (int32 i=(renderer->CountItems()-1);((!found) && (i>=0));i--)
	{
		if (((Renderer*)renderer->ItemAt(i))->Caught(where))
		{
			currentRenderer	= (Renderer*)renderer->ItemAt(i);
			if (currentRenderer->GetMessage()->what == P_C_CLASS_TYPE)
				found			= true;
		}
	}
	return currentRenderer;
}

Renderer* GroupRenderer::FindConnectionRenderer(BPoint where)
{
	TRACE();
	Renderer *currentRenderer = NULL;
	bool found	=	false;
	for (int32 i=(renderer->CountItems()-1);((!found) && (i>=0));i--)
	{
		if (((Renderer*)renderer->ItemAt(i))->Caught(where))
		{
			currentRenderer	= (Renderer*)renderer->ItemAt(i);
			if (currentRenderer->GetMessage()->what == P_C_CONNECTION_TYPE)
				found			= true;
		}
	}
	return currentRenderer;
}

Renderer* GroupRenderer::FindRenderer(BMessage *container)
{
	int32		i					= 0;
	Renderer	*currentRenderer	= NULL;
	bool		found				= false;
	while ((i<renderer->CountItems()) && (!found))
	{
		currentRenderer= (Renderer*)renderer->ItemAt(i);
		if (currentRenderer->GetMessage() == container)
			found=true;
		i++;
	}
	if (found)
		return currentRenderer;
	else
		return NULL;
}

void GroupRenderer::BringToFront(Renderer *wichRenderer)
{
	TRACE();
	renderer->RemoveItem(wichRenderer);
	renderer->AddItem(wichRenderer);
	//**draw the new "onTop" renderer

}

void GroupRenderer::SendToBack(Renderer *wichRenderer)
{
	TRACE();
	renderer->RemoveItem(wichRenderer);
	renderer->AddItem(wichRenderer,0);
	//**draw all wich are under the Thing redraw
}

bool GroupRenderer::DrawRenderer(void *arg,void *editor)
{
	Renderer *painter=(Renderer *)arg;
	painter->Draw((GraphEditor*)editor,BRect(0,0,0,0));
	return false;
}

void GroupRenderer::RecalcFrame(void)
{
	Renderer*	tmpRenderer		= NULL;
	BRect			groupFrame			= BRect(0,0,-1,-1);
	for (int32 i=0;(i<renderer->CountItems());i++)
	{
		tmpRenderer = (Renderer *) renderer->ItemAt(i);
		if ( (tmpRenderer->GetMessage()->what == P_C_CLASS_TYPE) || (tmpRenderer->GetMessage()->what == P_C_GROUP_TYPE) )
		{
			if (!groupFrame.IsValid())
				groupFrame = tmpRenderer->Frame();
			else
				groupFrame = groupFrame | tmpRenderer->Frame();
		}
	}
	groupFrame.InsetBy(-5,-5);
	groupFrame.top = groupFrame.top-15;
	if (groupFrame != frame)
	{
		frame = groupFrame;
		//** need to move the Attribs and the Name...
		if (parent)
			((GroupRenderer *)parent)->RecalcFrame();
	}
}