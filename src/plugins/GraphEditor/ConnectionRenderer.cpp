#include "ConnectionRenderer.h"
#include "ClassRenderer.h"
#include "ProjectConceptorDefs.h"

#include <interface/Window.h>
#include <stdio.h>
#include <math.h>


ConnectionRenderer::ConnectionRenderer(GraphEditor *parentEditor, BMessage *forContainer):Renderer(parentEditor, forContainer) {
	TRACE();
	Init();
}

void ConnectionRenderer::Init() {
	TRACE();
	from			= NULL;
	to				= NULL;
	fromPoint		= BPoint(0,0);
	toPoint			= BPoint(0,0);
	selected		= false;
	fillColor		= make_color(187,67,47,255);
	connectionType	= 2;
	bezier			= BShape();

//	connectionName	= new BTextControl(BRect(0,0,100,55),"Name",NULL,"Unbenannt",new BMessage(B_C_NAME_CHANGED));
//	AddChild(connectionName);
	BList		*outgoing	= NULL;
	BList		*incoming	= NULL;
	BMessage	*fromNode	= NULL;
	BMessage	*toNode		= NULL;
	BMessage	*data		= new BMessage();

	container->FindPointer(P_C_NODE_CONNECTION_FROM,(void **)&fromNode);
	container->FindPointer(P_C_NODE_CONNECTION_TO,(void **)&toNode);
	if (fromNode->FindPointer(P_C_NODE_OUTGOING,(void **)&outgoing) != B_OK) {
		outgoing = new BList();
		fromNode->AddPointer(P_C_NODE_OUTGOING,outgoing);
	}
	if (!outgoing->HasItem(container))
		outgoing->AddItem(container);
	if (toNode->FindPointer(P_C_NODE_INCOMING,(void **)&incoming) != B_OK) {
		incoming = new BList();
		toNode->AddPointer(P_C_NODE_INCOMING,incoming);
	}
	if (!incoming->HasItem(container))
		incoming->AddItem(container);
	if (container->FindMessage(P_C_NODE_DATA,data) != B_OK) {
		data->AddString(P_C_NODE_NAME,"Unbenannt");
		container->AddMessage(P_C_NODE_DATA,data);
	}
	container->FindPointer("ProjectConceptor::doc",(void **)&doc);
	sentTo						= new BMessenger(NULL,doc);
//	PCommandManager	*commandManager	= doc->GetCommandManager();
//	selectCommand	= commandManager->GetPCommand("Select");
	ValueChanged();
}

void ConnectionRenderer::MouseDown(BPoint where, int32 buttons,
	                              int32 clicks,int32 modifiers) {
	if (Caught(where)==true) {
		if (buttons & B_PRIMARY_MOUSE_BUTTON)
			editor->BringToFront(this);
		else if (buttons & B_SECONDARY_MOUSE_BUTTON )
			editor->SendToBack(this);
		if  (!selected)  {
			BMessage *selectMessage=new BMessage(P_C_EXECUTE_COMMAND);
			if  ((modifiers & B_SHIFT_KEY) != 0)
				selectMessage->AddBool("deselect",false);
			selectMessage->AddPointer("node",container);
			selectMessage->AddString("Command::Name","Select");
			sentTo->SendMessage(selectMessage);
		}
	}
}

void ConnectionRenderer::MouseMoved(BPoint pt, uint32 code, const BMessage *msg) {
}

void ConnectionRenderer::MouseUp(BPoint where) {
}

void ConnectionRenderer::LanguageChanged() {
}

void ConnectionRenderer::Draw(BView *drawOn, BRect updateRect) {

	//ValueChanged();
	CalcLine();
	if (connectionType == 0)
		DrawStraight(drawOn,updateRect);
	else if (connectionType == 1)
		DrawBended(drawOn,updateRect);
	else if (connectionType == 2)
		DrawAngled(drawOn,updateRect);
	else
		DrawStraight(drawOn,updateRect);

}

void ConnectionRenderer::MessageReceived(BMessage *message) {
	switch(message->what) {
		case P_C_VALUE_CHANGED:
				ValueChanged();
			break;
/*		case B_C_NAME_CHANGED:
		{
			BMessage	*data= new BMessage();
			container->FindMessage(P_C_NODE_DATA,data);
//			data->ReplaceString("Name",connectionName->Text());
			container->ReplaceMessage(P_C_NODE_DATA,data);
			break;
		}*/
	}
}

void ConnectionRenderer::ValueChanged() {
	BMessage	*tmpNode	= NULL;
	container->FindPointer(P_C_NODE_CONNECTION_FROM,(void **)&tmpNode);
	tmpNode->FindPointer(editor->RenderString(),(void **)&from);
	container->FindPointer(P_C_NODE_CONNECTION_TO,(void **)&tmpNode);
	tmpNode->FindPointer(editor->RenderString(),(void **)&to);
	container->FindBool(P_C_NODE_SELECTED,&selected);
	container->FindInt8(P_C_NODE_CONNECTION_TYPE, (int8 *)&connectionType);
}

void ConnectionRenderer::CalcLine() {
	if (from != NULL &&  to != NULL)
	{
		BRect	*fromRect	= new BRect(from->Frame());
		BRect	*toRect		= new BRect(to->Frame());
		float	toMiddleX 	=	(toRect->right-toRect->left)/2;
		float	toMiddleY	=	(toRect->bottom-toRect->top)/2;
		alpha		= atan2((toRect->top-fromRect->top),(toRect->left-fromRect->left));
		if ( (alpha < -M_PI_3_4 ) || (alpha > M_PI_3_4) ) {
			first		= BPoint(toRect->right+circleSize,toRect->top+toMiddleY-circleSize);
			second		= BPoint(first.x,toRect->top+toMiddleY+circleSize);
			third		= BPoint(toRect->right,toRect->top+toMiddleY);
			toPoint		= BPoint(first.x,third.y);
			fromPoint	= BPoint(fromRect->left,fromRect->top+(fromRect->bottom-fromRect->top)/2);
			float	bendLength	= BEND_LENGTH* (fromPoint.x-toPoint.x);
			firstBend.x		= fromPoint.x - bendLength;
			firstBend.y		= fromPoint.y;
			secondBend.x	= toPoint.x + bendLength;
			secondBend.y	= toPoint.y;
		}
		else if (alpha < -M_PI_4) {
			first		= BPoint(toRect->left+toMiddleX-circleSize,toRect->bottom+circleSize);
			second		= BPoint(toRect->left+toMiddleX+circleSize,toRect->bottom+circleSize);
			third		= BPoint(toRect->left+toMiddleX,toRect->bottom);
			toPoint		= BPoint(third.x,first.y);
			fromPoint	= BPoint(fromRect->left+(fromRect->right-fromRect->left)/2,fromRect->top);
			float	bendLength	= BEND_LENGTH* (fromPoint.y-toPoint.y);
			firstBend.x		= fromPoint.x;
			firstBend.y		= fromPoint.y - bendLength;
			secondBend.x	= toPoint.x;
			secondBend.y	= toPoint.y + bendLength;
		}
		else if (alpha> M_PI_4) {
			first		= BPoint(toRect->left+toMiddleX-circleSize,toRect->top-circleSize);
			second		= BPoint(toRect->left+toMiddleX+circleSize,toRect->top-circleSize);
			third		= BPoint(toRect->left+toMiddleX,toRect->top);
			toPoint		= BPoint(third.x,first.y);
			fromPoint	= BPoint(fromRect->left+(fromRect->right-fromRect->left)/2,fromRect->bottom);
			float	bendLength	= BEND_LENGTH* (toPoint.y-fromPoint.y);
			firstBend.x		= fromPoint.x;
			firstBend.y		= fromPoint.y + bendLength;
			secondBend.x	= toPoint.x;
			secondBend.y	= toPoint.y - bendLength;

		}
		else {
			first		= BPoint(toRect->left-circleSize,toRect->top+toMiddleY-circleSize);
			second		= BPoint(toRect->left-circleSize,toRect->top+toMiddleY+circleSize);
			third		= BPoint(toRect->left,toRect->top+toMiddleY);
			toPoint		= BPoint(first.x,third.y);
			fromPoint	= BPoint(fromRect->right,fromRect->top+(fromRect->bottom-fromRect->top)/2);
			float	bendLength	= BEND_LENGTH* (toPoint.x-fromPoint.x);
			firstBend.x		= fromPoint.x + bendLength;
			firstBend.y		= fromPoint.y;
			secondBend.x	= toPoint.x - bendLength;
			secondBend.y	= toPoint.y;
		}
	}
}


BRect ConnectionRenderer::Frame()
{
	float	left	= fromPoint.x;
	float	top		= fromPoint.y;
	float	right	= toPoint.x;
	float	bottom	= toPoint.y;
	float	c;
	if (left>right) 	{
		c		= right;
		right	= left;
		left	= c;
	}
	if (top>bottom) {
		c		= top;
		top		= bottom;
		bottom	= c;
	}
	return BRect(left,top,right,bottom);
}

bool ConnectionRenderer::Caught(BPoint where){
	if (connectionType == 0)
		return CaughtStraigt(where);
	else if (connectionType == 1)
		return CaughtBended(where);
	else if (connectionType == 2)
		return CaughtAngled(where);
}

void ConnectionRenderer::DrawStraight(BView *drawOn, BRect updateRect){
		drawOn->SetPenSize(2.0);
		BPoint	shadowFrom		= fromPoint;
		BPoint	shadowTo		= toPoint;
		BPoint	shadowfirst		= first;
		BPoint	shadowsecond	= second;
		BPoint	shadowthird		= third;
		shadowFrom.y			+=3;
		shadowTo.y				+=3;
		shadowfirst.y			+=3;
		shadowsecond.y			+=3;
		shadowthird.y			+=3;

		drawOn->SetHighColor(0,0,0,77);
		drawOn->StrokeLine(	shadowFrom,shadowTo);
		drawOn->FillTriangle(shadowfirst,shadowsecond,shadowthird);
		if (!selected)
			drawOn->SetHighColor(fillColor);
		else
			drawOn->SetHighColor(tint_color(fillColor,1.5));
		drawOn->StrokeLine(	fromPoint,toPoint);
		drawOn->FillTriangle(first,second,third);
}

void ConnectionRenderer::DrawBended(BView *drawOn, BRect updateRect){
	drawOn->SetPenSize(2.0);
	if (!selected)
		drawOn->SetHighColor(fillColor);
	else
		drawOn->SetHighColor(tint_color(fillColor,1.5));
	bezier=BShape();
	bezier.MoveTo(BPoint(0,0));
	BPoint	controlPoints[3];
	controlPoints[0]=firstBend-fromPoint;
	controlPoints[1]=secondBend-fromPoint;
	controlPoints[2]=toPoint-fromPoint;
	bezier.BezierTo(controlPoints);
	drawOn->MovePenTo(fromPoint);
	drawOn->StrokeShape(&bezier);
}

void ConnectionRenderer::DrawAngled(BView *drawOn, BRect updateRect){
	drawOn->SetPenSize(2.0);
	BPoint	shadowFrom		= fromPoint;
	BPoint	shadowTo		= toPoint;
	BPoint	shadowfirst		= first;
	BPoint	shadowsecond	= second;
	BPoint	shadowthird		= third;
	BPoint	sFirstBend		= firstBend;
	BPoint	sSecondBend		= secondBend;
	shadowFrom.y			+=3;
	shadowTo.y				+=3;
	shadowfirst.y			+=3;
	shadowsecond.y			+=3;
	shadowthird.y			+=3;
	sFirstBend.x			+=3;
	sFirstBend.y			+=3;
	sSecondBend.x			+=3;
	sSecondBend.y			+=3;
	drawOn->SetHighColor(0,0,0,77);
	drawOn->StrokeLine(	shadowFrom,sFirstBend);
	drawOn->StrokeLine(	sFirstBend,sSecondBend);
	drawOn->StrokeLine(	sSecondBend,shadowTo);
	drawOn->FillTriangle(shadowfirst,shadowsecond,shadowthird);
	if (!selected)
		drawOn->SetHighColor(fillColor);
	else
		drawOn->SetHighColor(tint_color(fillColor,1.5));
	drawOn->StrokeLine(	fromPoint,firstBend);
	drawOn->StrokeLine(	firstBend,secondBend);
	drawOn->StrokeLine(	secondBend,toPoint);
	drawOn->FillTriangle(first,second,third);
}

bool ConnectionRenderer::CaughtStraigt(BPoint where){
	/*float newy	= (double)where.x*ax+mx;
	float newx	= (double)where.y*ay+my;
	float dx	= (newx-where.x);
	float dy	= (newy-where.y);
	float dist	= sqrt(dx*dx+dy*dy);
	if (dist<max_entfernung)
		return true;
	else*/
		return false;
}

bool ConnectionRenderer::CaughtBended(BPoint where){
	if (bezier.Bounds().Contains(where) == true )
	{
		float t = 0;
		float minDist=999999;
		for ( t=0; t<1.0; t+=0.01)
		{
			minDist =fmin(minDist, Distance(where,PointOnBezier(t)));
			if (minDist < max_entfernung)
				return true;
		}
		return false;
	}
	else
		return false;
}

float ConnectionRenderer::Distance(BPoint one, BPoint two)
{
	float dx = one.x - two.x;
	float dy = one.y - two.y;
	return sqrt(dx*dx+dy*dy);
}

BPoint ConnectionRenderer::PointOnBezier(float t)
{
	double x1=fromPoint.x, y1=fromPoint.y;
	double cx1=firstBend.x, cy1=firstBend.y;
	double cx2=secondBend.x, cy2=secondBend.y;
	double x2=toPoint.x, y2=toPoint.x;
	double ax=cx1-x1, ay=cy1-y1;
	double bx=cx2-cx1-ax, by=cy2-cy1-ay;
	double cx=x2-cx2-ax-bx-bx; // instead of ...-ax-2*bx. Does it worth?
	double cy=y2-cy2-ay-by-by;
	
	double x=x1+(t*((3*ax)+(t*((3*bx)+(t*cx)))));
	double y=y1+(t*((3*ay)+(t*((3*by)+(t*cy)))));	
	return BPoint(x,y);
}

bool ConnectionRenderer::CaughtAngled(BPoint where){
	return false;
}
