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
	hasPreviewFillColor	= false;
	penSize			= 2.0;
	connectionType	= 2;
	arrows			= 1;
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
	// connections saved before arrow ends were selectable have no such
	// field - give them the target-only default, which is exactly how they
	// used to draw. Added rather than only defaulted locally so the
	// toolbar's ChangeValue has an existing field to replace.
	if (container->FindInt8(P_C_NODE_CONNECTION_ARROWS,&arrows) != B_OK) {
		arrows	= 1;
		container->AddInt8(P_C_NODE_CONNECTION_ARROWS,arrows);
	}
	// same P_C_NODE_PATTERN sub-message a class node has - the Pen size/Fill
	// color toolbar controls (GraphEditor.cpp's G_E_PEN_SIZE_CHANGED/
	// G_E_COLOR_CHANGED) go through ChangeValue targeting exactly these two
	// fields there, regardless of node type
	BMessage	pattern;
	if (container->FindMessage(P_C_NODE_PATTERN,&pattern) == B_OK) {
		pattern.FindFloat("PenSize",&penSize);
		pattern.FindInt32("FillColor",(int32 *)&fillColor);
	}
	// a real committed value just arrived - drop any leftover preview
	// from a picker session, same reasoning as ClassRenderer::ValueChanged()
	hasPreviewFillColor	= false;
}

void ConnectionRenderer::SetPreviewFillColor(rgb_color color) {
	hasPreviewFillColor	= true;
	previewFillColor	= color;
}

void ConnectionRenderer::ClearPreviewFillColor(void) {
	hasPreviewFillColor	= false;
}

void ConnectionRenderer::InvalidateEndpoint(Renderer *removed) {
	if ((Renderer *)from == removed)
		from = NULL;
	if ((Renderer *)to == removed)
		to = NULL;
}

void ConnectionRenderer::CalcLine() {
	if (from != NULL &&  to != NULL)
	{
		BRect	*fromRect	= new BRect(from->Frame());
		BRect	*toRect		= new BRect(to->Frame());
		BPoint	fromOutward	= BPoint(0,0);
		float	toMiddleX 	=	(toRect->right-toRect->left)/2;
		float	toMiddleY	=	(toRect->bottom-toRect->top)/2;
		alpha		= atan2((toRect->top-fromRect->top),(toRect->left-fromRect->left));
		if ( (alpha < -M_PI_3_4 ) || (alpha > M_PI_3_4) ) {
			first		= BPoint(toRect->right+arrowSize,toRect->top+toMiddleY-arrowSize);
			second		= BPoint(first.x,toRect->top+toMiddleY+arrowSize);
			third		= BPoint(toRect->right,toRect->top+toMiddleY);
			toPoint		= BPoint(first.x,third.y);
			fromPoint	= BPoint(fromRect->left,fromRect->top+(fromRect->bottom-fromRect->top)/2);
			fromOutward	= BPoint(-1,0);
			float	bendLength	= BEND_LENGTH* (fromPoint.x-toPoint.x);
			firstBend.x		= fromPoint.x - bendLength;
			firstBend.y		= fromPoint.y;
			secondBend.x	= toPoint.x + bendLength;
			secondBend.y	= toPoint.y;
		}
		else if (alpha < -M_PI_4) {
			first		= BPoint(toRect->left+toMiddleX-arrowSize,toRect->bottom+arrowSize);
			second		= BPoint(toRect->left+toMiddleX+arrowSize,toRect->bottom+arrowSize);
			third		= BPoint(toRect->left+toMiddleX,toRect->bottom);
			toPoint		= BPoint(third.x,first.y);
			fromPoint	= BPoint(fromRect->left+(fromRect->right-fromRect->left)/2,fromRect->top);
			fromOutward	= BPoint(0,-1);
			float	bendLength	= BEND_LENGTH* (fromPoint.y-toPoint.y);
			firstBend.x		= fromPoint.x;
			firstBend.y		= fromPoint.y - bendLength;
			secondBend.x	= toPoint.x;
			secondBend.y	= toPoint.y + bendLength;
		}
		else if (alpha> M_PI_4) {
			first		= BPoint(toRect->left+toMiddleX-arrowSize,toRect->top-arrowSize);
			second		= BPoint(toRect->left+toMiddleX+arrowSize,toRect->top-arrowSize);
			third		= BPoint(toRect->left+toMiddleX,toRect->top);
			toPoint		= BPoint(third.x,first.y);
			fromPoint	= BPoint(fromRect->left+(fromRect->right-fromRect->left)/2,fromRect->bottom);
			fromOutward	= BPoint(0,1);
			float	bendLength	= BEND_LENGTH* (toPoint.y-fromPoint.y);
			firstBend.x		= fromPoint.x;
			firstBend.y		= fromPoint.y + bendLength;
			secondBend.x	= toPoint.x;
			secondBend.y	= toPoint.y - bendLength;

		}
		else {
			first		= BPoint(toRect->left-arrowSize,toRect->top+toMiddleY-arrowSize);
			second		= BPoint(toRect->left-arrowSize,toRect->top+toMiddleY+arrowSize);
			third		= BPoint(toRect->left,toRect->top+toMiddleY);
			toPoint		= BPoint(first.x,third.y);
			fromPoint	= BPoint(fromRect->right,fromRect->top+(fromRect->bottom-fromRect->top)/2);
			fromOutward	= BPoint(1,0);
			float	bendLength	= BEND_LENGTH* (toPoint.x-fromPoint.x);
			firstBend.x		= fromPoint.x + bendLength;
			firstBend.y		= fromPoint.y;
			secondBend.x	= toPoint.x - bendLength;
			secondBend.y	= toPoint.y;
		}
		// Source arrow head, mirroring the target one each branch above
		// built by hand: tip on the source's own edge, base a arrowSize
		// further out along the direction the line leaves in. The branches
		// only have to say which way that is - the triangle itself is the
		// same construction every time. Note the bend control points above
		// keep using the unmoved fromPoint; the offset is a few pixels and
		// shifting the curve's start by it is not worth recomputing them.
		// BPoint has no scalar multiply, so the offsets are spelled out
		float	outX	= fromOutward.x*arrowSize;
		float	outY	= fromOutward.y*arrowSize;
		float	sideX	= -fromOutward.y*arrowSize;
		float	sideY	= fromOutward.x*arrowSize;
		fromThird	= fromPoint;
		fromFirst	= BPoint(fromPoint.x+outX+sideX,fromPoint.y+outY+sideY);
		fromSecond	= BPoint(fromPoint.x+outX-sideX,fromPoint.y+outY-sideY);
		if (arrows & 2)
			fromPoint	= BPoint(fromPoint.x+outX,fromPoint.y+outY);
		delete fromRect;
		delete toRect;
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
		drawOn->SetPenSize(penSize);
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
		if (arrows & 1)
			drawOn->FillTriangle(shadowfirst,shadowsecond,shadowthird);
		if (arrows & 2)
			drawOn->FillTriangle(BPoint(fromFirst.x,fromFirst.y+3),
				BPoint(fromSecond.x,fromSecond.y+3),BPoint(fromThird.x,fromThird.y+3));
		if (!selected)
			drawOn->SetHighColor(EffectiveFillColor());
		else
			drawOn->SetHighColor(tint_color(EffectiveFillColor(),1.5));
		drawOn->StrokeLine(	fromPoint,toPoint);
		if (arrows & 1)
			drawOn->FillTriangle(first,second,third);
		if (arrows & 2)
			drawOn->FillTriangle(fromFirst,fromSecond,fromThird);
}

void ConnectionRenderer::DrawBended(BView *drawOn, BRect updateRect){
	drawOn->SetPenSize(penSize);
	if (!selected)
		drawOn->SetHighColor(EffectiveFillColor());
	else
		drawOn->SetHighColor(tint_color(EffectiveFillColor(),1.5));
	bezier=BShape();
	bezier.MoveTo(fromPoint);
	BPoint	controlPoints[3];
	controlPoints[0]=firstBend;
	controlPoints[1]=secondBend;
	controlPoints[2]=toPoint;
	bezier.BezierTo(controlPoints);
	// `bezier` above is kept only so CaughtBended()'s Bounds() pre-check has
	// something to work with - it's not used for drawing. StrokeShape()
	// verified live to ignore BView::SetScale() entirely in this Haiku
	// build: at any zoom other than 100% the curve painted at its native,
	// unscaled size/position while every other primitive here (StrokeLine,
	// FillRoundRect, ...) scaled correctly. Approximating the curve as short
	// StrokeLine() segments via PointOnBezier() (same source CaughtBended()
	// already uses for hit-testing) sidesteps StrokeShape() entirely and
	// scales like everything else.
	BPoint	previous = fromPoint;
	for (float t = 0.02; t <= 1.0; t += 0.02) {
		BPoint	current = PointOnBezier(t);
		drawOn->StrokeLine(previous,current);
		previous = current;
	}
	// This style used to draw no arrow head at all, unlike the straight and
	// angled ones - so which ends carry an arrow had no effect on exactly
	// the style new connections default to. Same triangles the others use;
	// CalcLine() computes them regardless of style.
	if (arrows & 1)
		drawOn->FillTriangle(first,second,third);
	if (arrows & 2)
		drawOn->FillTriangle(fromFirst,fromSecond,fromThird);
}

void ConnectionRenderer::DrawAngled(BView *drawOn, BRect updateRect){
	drawOn->SetPenSize(penSize);
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
	if (arrows & 1)
		drawOn->FillTriangle(shadowfirst,shadowsecond,shadowthird);
	if (arrows & 2)
		drawOn->FillTriangle(BPoint(fromFirst.x,fromFirst.y+3),
			BPoint(fromSecond.x,fromSecond.y+3),BPoint(fromThird.x,fromThird.y+3));
	if (!selected)
		drawOn->SetHighColor(EffectiveFillColor());
	else
		drawOn->SetHighColor(tint_color(EffectiveFillColor(),1.5));
	drawOn->StrokeLine(	fromPoint,firstBend);
	drawOn->StrokeLine(	firstBend,secondBend);
	drawOn->StrokeLine(	secondBend,toPoint);
	if (arrows & 1)
		drawOn->FillTriangle(first,second,third);
	if (arrows & 2)
		drawOn->FillTriangle(fromFirst,fromSecond,fromThird);
}

bool ConnectionRenderer::CaughtStraigt(BPoint where){
	return DistanceToSegment(where,fromPoint,toPoint) < max_entfernung;
}

bool ConnectionRenderer::CaughtBended(BPoint where){
	// bezier's points are absolute (document-space), same as
	// PointOnBezier() below, so Bounds() needs no offset here.
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

float ConnectionRenderer::DistanceToSegment(BPoint p, BPoint segStart, BPoint segEnd)
{
	float dx		= segEnd.x-segStart.x;
	float dy		= segEnd.y-segStart.y;
	float lengthSq	= dx*dx+dy*dy;
	if (lengthSq == 0)
		return Distance(p,segStart);
	float t	= ((p.x-segStart.x)*dx+(p.y-segStart.y)*dy)/lengthSq;
	t		= fmax(0.0f,fmin(1.0f,t));
	return Distance(p,BPoint(segStart.x+t*dx,segStart.y+t*dy));
}

BPoint ConnectionRenderer::PointOnBezier(float t)
{
	double x1=fromPoint.x, y1=fromPoint.y;
	double cx1=firstBend.x, cy1=firstBend.y;
	double cx2=secondBend.x, cy2=secondBend.y;
	double x2=toPoint.x, y2=toPoint.y;
	double ax=cx1-x1, ay=cy1-y1;
	double bx=cx2-cx1-ax, by=cy2-cy1-ay;
	double cx=x2-cx2-ax-bx-bx; // instead of ...-ax-2*bx. Does it worth?
	double cy=y2-cy2-ay-by-by;
	
	double x=x1+(t*((3*ax)+(t*((3*bx)+(t*cx)))));
	double y=y1+(t*((3*ay)+(t*((3*by)+(t*cy)))));	
	return BPoint(x,y);
}

bool ConnectionRenderer::CaughtAngled(BPoint where){
	if (DistanceToSegment(where,fromPoint,firstBend) < max_entfernung)
		return true;
	if (DistanceToSegment(where,firstBend,secondBend) < max_entfernung)
		return true;
	return DistanceToSegment(where,secondBend,toPoint) < max_entfernung;
}
