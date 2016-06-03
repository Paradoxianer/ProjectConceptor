#include "ClassRenderer.h"
#include "ProjectConceptorDefs.h"

#include <math.h>

#include <interface/Font.h>
#include <interface/View.h>
#include <interface/GraphicsDefs.h>

#include <interface/Window.h>

#include <support/String.h>
#include "AttributRenderer.h"
#include "GroupRenderer.h"


ClassRenderer::ClassRenderer(GraphEditor *parentEditor, BMessage *forContainer):Renderer(parentEditor, forContainer)
{
	TRACE();
	Init();
	ValueChanged();
	name->MouseDown(BPoint(0,0));
}
void ClassRenderer::Init()
{
	TRACE();
	PRINT_OBJECT(*container);
	status_t	err			 	= B_OK;
	resizing					= false;
	showConnecter				= true;
	startMouseDown				= NULL;
	oldPt						= NULL;
	doc							= NULL;
	parentNode					= NULL;

	xRadius						= 10;
	yRadius						= 10;
	attributes					= new vector<Renderer *>();
	frame						= BRect(0,0,0,0);
	selected					= false;
	font						= new AFont();
	penSize						= 1.0;
	connecting					= 0;

	BMessage	*editMessage		= new BMessage(P_C_EXECUTE_COMMAND);
	editMessage->AddPointer("node",container);

	editMessage->AddString("Command::Name","ChangeValue");
	BMessage	*valueContainer	= new BMessage();
	valueContainer->AddString("name","Name");
	valueContainer->AddString("subgroup","Node::Data");
	valueContainer->AddInt32("type",B_STRING_TYPE);
	editMessage->AddMessage("valueContainer",valueContainer);
	name						= new StringRenderer(editor,"",BRect(0,0,100,100), editMessage);
	err 						= container->FindPointer("ProjectConceptor::doc",(void **)&doc);
	sentTo						= new BMessenger(NULL,doc);
	if (container->FindFloat("Node::xRadius",&xRadius) != B_OK)
		container->AddFloat("Node::xRadius",7.0);
	if (container->FindFloat("Node::yRadius",&yRadius) != B_OK)
		container->AddFloat("Node::yRadius",7.0);
	container->FindPointer("Node::parent", (void **)&parentNode);

}


void ClassRenderer::MouseDown(BPoint where, int32 buttons,
	                              int32 clicks,int32 modifiers)
{
	bool		found			= false;
	Renderer*	tmpRenderer		= NULL;
	if (name->Caught(where)) {
		name->MouseDown(where);
		found	= true;
	}
	for (uint32 i = 0; (found == false) && (i < attributes->size());i++) {
		tmpRenderer=(*attributes)[i];
		if (tmpRenderer->Caught(where)) {
			found = true;
			tmpRenderer->MouseDown(where);
		}
	}

	if ((!found) && (startMouseDown == NULL)) {
		uint32 buttons = 0;
		uint32 modifiers = 0;
		BMessage *currentMsg = editor->Window()->CurrentMessage();
		currentMsg->FindInt32("buttons", (int32 *)&buttons);
		currentMsg->FindInt32("modifiers", (int32 *)&modifiers);
		if (buttons & B_PRIMARY_MOUSE_BUTTON) {
			editor->BringToFront(this);
			startMouseDown	= new BPoint(where);
			startFrame		= new BRect(frame);
			editor->SetMouseEventMask(B_POINTER_EVENTS, B_NO_POINTER_HISTORY | B_SUSPEND_VIEW_FOCUS | B_LOCK_WINDOW_FOCUS);
			if  (!selected) {
				BMessage *selectMessage=new BMessage(P_C_EXECUTE_COMMAND);
				if  ((modifiers & B_SHIFT_KEY) != 0)
					selectMessage->AddBool("deselect",false);
				selectMessage->AddPointer("node",container);
				selectMessage->AddString("Command::Name","Select");
				sentTo->SendMessage(selectMessage);
			}
			if (leftConnection.Contains(where))
				connecting	= 1;
			else if (topConnection.Contains(where))
				connecting	= 2;
			else if (rightConnection.Contains(where))
				connecting	= 3;
			else if (bottomConnection.Contains(where))
				connecting	= 4;
			else if ( (where.y >= (frame.bottom-(2*circleSize))) && (where.x >= (frame.right-(2*circleSize))) ) {
				resizing = true;
			}
		}
		else if (buttons & B_SECONDARY_MOUSE_BUTTON )
			editor->SendToBack(this);
	}
}
void ClassRenderer::MouseMoved(BPoint pt, uint32 code, const BMessage *msg) {
	if (startMouseDown) {
		if (connecting == 0) {
			float dx	= 0;
			float dy	= 0;
			//resizedifferenze
			float rdx	= 0;
			float rdy	= 0;

			if (!oldPt)
				oldPt	= startMouseDown;
			if (editor->GridEnabled()) {
				float newPosX = startFrame->left + (pt.x-startMouseDown->x);
				float newPosY = startFrame->top + (pt.y-startMouseDown->y);
				newPosX = newPosX - fmod(newPosX,editor->GridWidth());
				newPosY = newPosY - fmod(newPosY,editor->GridWidth());
				dx	= newPosX - frame.left;
				dy	= newPosY - frame.top;

				newPosX = startFrame->right + (pt.x-startMouseDown->x);
				newPosY = startFrame->bottom + (pt.y-startMouseDown->y);
				newPosX = newPosX - fmod(newPosX,editor->GridWidth());
				newPosY = newPosY - fmod(newPosY,editor->GridWidth());
				rdx	= newPosX - frame.right;
				rdy	= newPosY - frame.bottom;
			}
			else {
				dx = pt.x - oldPt->x;
				dy = pt.y - oldPt->y;	
			}
			oldPt	= new BPoint(pt);
			if (!resizing) { 
				BList *renderer	= editor->RenderList();
				for (int32 i=0;i<renderer->CountItems();i++) {
					MoveAll(renderer->ItemAt(i),dx,dy);
				}
				if (parentNode) {
					GroupRenderer	*parent	= NULL;
					if (parentNode->FindPointer(editor->RenderString(), (void **)&parent) == B_OK)
						parent->RecalcFrame();
				}
				editor->Invalidate();
			}
			else {
				BList *renderer	= editor->RenderList();
				if (editor->GridEnabled())
					for (int32 i=0;i<renderer->CountItems();i++) {
						ResizeAll(renderer->ItemAt(i),rdx,rdy);
					}
				else
					for (int32 i=0;i<renderer->CountItems();i++) {
						ResizeAll(renderer->ItemAt(i),dx,dy);
					}
				if (parentNode) {
					GroupRenderer	*parent	= NULL;
					if (parentNode->FindPointer(editor->RenderString(), (void **)&parent) == B_OK)
						parent->RecalcFrame();
				}
				editor->Invalidate();
			}
		}
		else {
			// make connecting Stuff
			BMessage *connecter=new BMessage(G_E_CONNECTING);
			connecter->AddPoint("Node::from",*startMouseDown);
			connecter->AddPoint("Node::to",pt);
			(new BMessenger((BView *)editor))->SendMessage(connecter);
		}
	}
}


void ClassRenderer::MouseUp(BPoint where) {
	bool		found			= false;
	Renderer*	tmpRenderer		= NULL;
	for (uint32 i = 0; (found == false) && (i < attributes->size());i++) {
		tmpRenderer=(*attributes)[i];
		if (tmpRenderer->Caught(where)) {
			found = true;
			tmpRenderer->MouseUp(where);
		}
	}
	if ( (!found) && (startMouseDown) ) {
		if (connecting == 0) {
			float dx = where.x - startMouseDown->x;
			float dy = where.y - startMouseDown->y;
			if (editor->GridEnabled()) {
				float newPosX = startFrame->left + dx;
				float newPosY = startFrame->top + dy;
				newPosX = newPosX - fmod(newPosX,editor->GridWidth());
				newPosY = newPosY - fmod(newPosY,editor->GridWidth());
				dx = newPosX-startFrame->left;
				dy = newPosY-startFrame->top;
			}
			if (!resizing) {
				BMessage	*mover		= new BMessage(P_C_EXECUTE_COMMAND);
				mover->AddString("Command::Name","Move");
				mover->AddFloat("dx",dx);
				mover->AddFloat("dy",dy);
				AdjustParents(parentNode,mover);
				sentTo->SendMessage(mover);
			}
			else {
				BMessage	*resizer	= new BMessage(P_C_EXECUTE_COMMAND);
				resizer->AddString("Command::Name","Resize");
				resizer->AddFloat("dx",dx);
				resizer->AddFloat("dy",dy);
				AdjustParents(parentNode,resizer);
				sentTo->SendMessage(resizer);
			}
			resizing		= false;
		}
		else {
			BMessage *connecter=new BMessage(G_E_CONNECTED);
			connecter->AddPointer("Node::from",container);
			connecter->AddPoint("Node::to",where);
			(new BMessenger((BView *)editor))->SendMessage(connecter);
		}
		if (startMouseDown) delete startMouseDown;
		if (oldPt) delete oldPt;
		startMouseDown	= NULL;
		oldPt			= NULL;
		connecting		= 0;
	}
}


void ClassRenderer::Draw(BView *drawOn, BRect updateRect) {
	BRect		shadowFrame = frame;
	bool		fitIn		= true;
	drawOn->SetFont(font);
	rgb_color	drawColor;
	shadowFrame.OffsetBy(3,3);
	drawOn->SetPenSize(penSize);
	drawOn->SetHighColor(0,0,0,77);
	drawOn->FillRoundRect(shadowFrame, xRadius, yRadius);
	drawColor=fillColor;
	if (selected) {
		drawOn->SetPenSize(5.0);
		drawOn->SetHighColor(200,0,0,150);
		BRect selectFrame = frame;
		selectFrame.InsetBy(-2,-2);
		drawOn->StrokeRoundRect(selectFrame, xRadius, yRadius);
		drawOn->SetHighColor(drawColor);
	}
	drawOn->SetHighColor(drawColor);
	drawOn->FillRoundRect(frame, xRadius, yRadius);
	
	

	drawOn->SetHighColor(0,0,0,255);	
	drawOn->FillTriangle(BPoint(frame.right-(3*circleSize),frame.bottom),BPoint(frame.right,frame.bottom-(3*circleSize)),BPoint(frame.right,frame.bottom));
	

	drawOn->SetHighColor(borderColor);
	drawOn->SetPenSize(penSize);
	drawOn->StrokeRoundRect(frame, xRadius, yRadius);
	if (showConnecter) {
		drawOn->SetHighColor(200,0,0,255);

		drawOn->FillEllipse(leftConnection);
		drawOn->FillEllipse(topConnection);
		drawOn->FillEllipse(rightConnection);
		drawOn->FillEllipse(bottomConnection);
		drawOn->SetHighColor(borderColor);

		drawOn->StrokeEllipse(leftConnection);
		drawOn->StrokeEllipse(topConnection);
		drawOn->StrokeEllipse(rightConnection);
		drawOn->StrokeEllipse(bottomConnection);
	}


	name->Draw(drawOn,updateRect);
	vector<Renderer *>::iterator	allAttributes = attributes->begin();
	while( allAttributes != attributes->end() ) {
		if (frame.Contains((*allAttributes)->Frame()))
			(*allAttributes)->Draw(drawOn,updateRect);
		else
			fitIn=false;
		allAttributes++;
	}
	if (!fitIn)
		drawOn->DrawString("...",BPoint(frame.left+circleSize+2,frame.bottom-(yRadius/3)));
}

void ClassRenderer::MessageReceived(BMessage *message) {
	switch(message->what) {
		case P_C_VALUE_CHANGED:
				ValueChanged();
			break;
	}
}


void ClassRenderer::ValueChanged() {
	TRACE();
	BMessage	*pattern		= new BMessage();
	BMessage	*messageFont	= new BMessage();
	BMessage	*data			= new BMessage();
	char		*newName		= NULL;

	char		*attribName		= NULL;
	BMessage	*attribMessage	= new BMessage();
	uint32		type			= B_ANY_TYPE;
	int32		count			= 0;

	container->FindRect("Node::frame",&frame);
	container->FindBool("Node::selected",&selected);
	container->FindFloat("Node::xRadius",&xRadius);
	container->FindFloat("Node::yRadius",&yRadius);
	if (container->FindMessage("Node::Pattern",pattern) != B_OK) {
		container->AddMessage("Node::Pattern",editor->GetStandartPattern());
		pattern = editor->GetStandartPattern();
	}
	if (container->FindMessage("Node::Font",messageFont) == B_OK) {
		delete font;
		font	= new AFont(messageFont);
	}
	container->FindMessage("Node::Data",data);
	pattern->FindInt32("FillColor",(int32 *)&fillColor);
	pattern->FindInt32("BorderColor",(int32 *)&borderColor);
	pattern->FindFloat("PenSize",&penSize);
	data->FindString("Name",(const char **)&newName);
	name->SetString(newName);
	name->SetFrame(BRect(frame.left+(xRadius/3),frame.top+(yRadius/3),frame.right-(xRadius/3),frame.top+12));
	
	
	//delete all "old" Attribs
	attributes->erase(attributes->begin(),attributes->end());
	//and add all attribs we found
	for (int32 i = 0; data->GetInfo(B_MESSAGE_TYPE, i,(char **) &attribName, &type, &count) == B_OK; i++) {
		if (data->FindMessage(attribName,count-1,attribMessage) == B_OK)
			InsertAttribute(attribName,attribMessage, count-1);
	}
	container->FindPointer("Node::parent", (void **)&parentNode);
	float yMiddle = frame.top+(frame.Height()/2);
	float xMiddle = frame.left+(frame.Width()/2);
	leftConnection.Set(frame.left-circleSize,yMiddle-circleSize,frame.left+circleSize,yMiddle+circleSize);
	topConnection.Set(xMiddle-circleSize,frame.top-circleSize,xMiddle+circleSize,frame.top+circleSize);
	rightConnection.Set(frame.right-circleSize,yMiddle-circleSize,frame.right+circleSize,yMiddle+circleSize);
	bottomConnection.Set(xMiddle-circleSize,frame.bottom-circleSize,xMiddle+circleSize,frame.bottom+circleSize);
}

BRect ClassRenderer::Frame( void ) {
	return frame;
}

bool  ClassRenderer::Caught(BPoint where) {
	 bool contains	= frame.Contains(where);
	 if (!contains) {
	 	contains = leftConnection.Contains(where);
	 	if (!contains) {
		 	contains = topConnection.Contains(where);
		 	if (!contains) {
			 	contains = rightConnection.Contains(where);
				if (!contains)
	 				contains = bottomConnection.Contains(where);
		 	}
	 	}
	 }
	 return contains;
}
//**implement this
void  ClassRenderer::SetFrame(BRect newFrame) {
}

bool  ClassRenderer::MoveAll(void *arg,float dx, float dy) {
	Renderer	*renderer	= (Renderer*)arg;
	if (renderer->Selected())
		renderer->MoveBy(dx,dy);
	return false;
}

bool  ClassRenderer::ResizeAll(void *arg,float dx, float dy) {
	Renderer	*renderer	= (Renderer*)arg;
	if (renderer->Selected())
		renderer->ResizeBy(dx,dy);
	return false;
}

void ClassRenderer::MoveBy(float dx,float dy) {
	frame.OffsetBy(dx,dy);
	leftConnection.OffsetBy(dx,dy);
	topConnection.OffsetBy(dx,dy);
	rightConnection.OffsetBy(dx,dy);
	bottomConnection.OffsetBy(dx,dy);
	name->MoveBy(dx,dy);
	vector<Renderer *>::iterator	allAttributes = attributes->begin();
	while( allAttributes != attributes->end() ) {
		(*allAttributes)->MoveBy(dx,dy);
		allAttributes++;
	}
}

void ClassRenderer::ResizeBy(float dx,float dy) {
	if ((frame.right+dx-frame.left) > 70)
		frame.right		+= dx;
	if  ((frame.bottom+dy-frame.top) > 30)
		frame.bottom	+= dy;
	name->ResizeBy(dy,dy);
	vector<Renderer *>::iterator	allAttributes = attributes->begin();
	while( allAttributes != attributes->end() ) {
		(*allAttributes)->ResizeBy(dx,dy);
		allAttributes++;
	}
	float yMiddle = frame.top+(frame.Height()/2);
	float xMiddle = frame.left+(frame.Width()/2);
	leftConnection.Set(frame.left-circleSize,yMiddle-circleSize,frame.left+circleSize,yMiddle+circleSize);
	topConnection.Set(xMiddle-circleSize,frame.top-circleSize,xMiddle+circleSize,frame.top+circleSize);
	rightConnection.Set(frame.right-circleSize,yMiddle-circleSize,frame.right+circleSize,yMiddle+circleSize);
	bottomConnection.Set(xMiddle-circleSize,frame.bottom-circleSize,xMiddle+circleSize,frame.bottom+circleSize);
}

void ClassRenderer::InsertAttribute(char *attribName,BMessage *attribute,int32 count)
{
	/*switch(attribute->what)
	{
		case B_STRING_TYPE:
		{
			BMessage*	editMessage		= new BMessage(P_C_EXECUTE_COMMAND);
			editMessage->AddPointer("node",container);
			editMessage->AddString("Command::Name","ChangeValue");
			editMessage->AddString("subgroup","Node::Data");
			editMessage->AddString("name",attribName);
			attribute->FindString("Name",(const char **)&realName);
			BString		*testString 	= new BString(attribName);
			Renderer	*testRenderer	= new StringRenderer(editor,"",BRect(frame.left+2,frame.top+10,frame.right-2,frame.bottom-2), editMessage);
			attributes->insert(pair<BString *,Renderer*>(testString,testRenderer));
			break;
		}
	}*/
	BRect	attributeRect;
	if (attributes->empty())
	{
		attributeRect = BRect(frame.left+circleSize+2,name->Frame().bottom+6,frame.right-circleSize-2,frame.bottom);
	}
	else
	{
		Renderer* lastRenderer = (*attributes)[attributes->size()-1];
		attributeRect = BRect(frame.left+circleSize+2,lastRenderer->Frame().bottom,frame.right-circleSize-2,frame.bottom);
	}
	BMessage*	editMessage		= new BMessage(P_C_EXECUTE_COMMAND);
	editMessage->AddPointer("node",container);
	editMessage->AddString("Command::Name","ChangeValue");
	BMessage*	valueContainer	= new BMessage();
	valueContainer->AddString("subgroup","Node::Data");
	valueContainer->AddString("subgroup",attribName);
	editMessage->AddMessage("valueContainer",valueContainer);
	delete valueContainer;
	BMessage*	removeAttribMessage		= new BMessage(P_C_EXECUTE_COMMAND);
	removeAttribMessage->AddPointer("node",container);
	removeAttribMessage->AddString("Command::Name","RemoveAttribute");
	valueContainer	= new BMessage();
	valueContainer->AddString("subgroup","Node::Data");
	valueContainer->AddString("name",attribName);
	valueContainer->AddInt32("index",count);
	removeAttribMessage->AddMessage("valueContainer",valueContainer);
	Renderer	*testRenderer	= new AttributRenderer(editor,attribute,attributeRect, editMessage,removeAttribMessage);
	attributes->push_back(testRenderer);
}

void ClassRenderer::AdjustParents(BMessage* theParent, BMessage *command) {
	if (theParent) {
		BMessage *tmpParent = theParent;
		BRect		parentRect	= BRect(0,0,0,0);

		//run through all Parents until we find the "Masterparent"
		while (tmpParent) {
			tmpParent->FindRect("Node::frame",&parentRect);
			GroupRenderer	*parent	= NULL;
			if (tmpParent->FindPointer(editor->RenderString(), (void **)&parent) == B_OK) {
				if (parent->Frame() != parentRect) {
					BMessage	*changeValue		= new BMessage(P_C_EXECUTE_COMMAND);
					BMessage	*valueContainer		= new BMessage();
					changeValue->AddString("Command::Name","ChangeValue");
					changeValue->AddPointer("node",tmpParent);
					valueContainer->AddInt32("type",B_RECT_TYPE);
					valueContainer->AddString("name", "Node::frame" );
					valueContainer->AddRect("newValue", parent->Frame());
					changeValue->AddMessage("valueContainer",valueContainer);
					command->AddMessage("PCommand::subPCommand",changeValue);
				}
			}
			tmpParent->FindPointer("Node::parent", (void **)&tmpParent);
		}
	}
}
