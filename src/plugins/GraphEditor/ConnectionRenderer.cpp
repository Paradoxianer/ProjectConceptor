#include "ConnectionRenderer.h"
#include "ClassRenderer.h"
#include "ProjectConceptorDefs.h"

#include <Catalog.h>
#include <DataIO.h>
#include <interface/Window.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "GraphEditor"


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
	selectedBend	= -1;
	dragBend		= -1;
	dragMoved		= false;
	dragAdded		= false;

//	connectionName	= new BTextControl(BRect(0,0,100,55),"Name",NULL,"Unbenannt",new BMessage(B_C_NAME_CHANGED));
//	AddChild(connectionName);
	BList		*outgoing	= NULL;
	BList		*incoming	= NULL;
	BMessage	*fromNode	= NULL;
	BMessage	*toNode		= NULL;
	BMessage	*data		= new BMessage();

	// A connection whose from/to id never resolved (see Indexer::DeIndexConnection())
	// has neither field set here - degrade to a from/to == NULL renderer
	// (already this class's own documented behavior for that case, see
	// GraphEditor::ValueChanged()'s comment on build order) instead of
	// dereferencing a NULL fromNode/toNode, which used to crash outright.
	if (container->FindPointer(P_C_NODE_CONNECTION_FROM,(void **)&fromNode) != B_OK)
		PRINT(("ERROR:\tConnectionRenderer::Init() - no resolved from-node\n"));
	if (container->FindPointer(P_C_NODE_CONNECTION_TO,(void **)&toNode) != B_OK)
		PRINT(("ERROR:\tConnectionRenderer::Init() - no resolved to-node\n"));
	if (fromNode != NULL) {
		if (fromNode->FindPointer(P_C_NODE_OUTGOING,(void **)&outgoing) != B_OK) {
			outgoing = new BList();
			fromNode->AddPointer(P_C_NODE_OUTGOING,outgoing);
		}
		if (!outgoing->HasItem(container))
			outgoing->AddItem(container);
	}
	if (toNode != NULL) {
		if (toNode->FindPointer(P_C_NODE_INCOMING,(void **)&incoming) != B_OK) {
			incoming = new BList();
			toNode->AddPointer(P_C_NODE_INCOMING,incoming);
		}
		if (!incoming->HasItem(container))
			incoming->AddItem(container);
	}
	if (container->FindMessage(P_C_NODE_DATA,data) != B_OK) {
		data->AddString(P_C_NODE_NAME,B_TRANSLATE("Untitled"));
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
	if (selected && (buttons & B_PRIMARY_MOUSE_BUTTON)) {
		int32	handle	= HandleAt(where);
		int32	plus	= (handle < 0) ? PlusAt(where) : -1;
		if ((handle >= 0) || (plus >= 0)) {
			// dragging any handle fixes the whole route as it is now
			dragPoints	= handles;
			dragAdded	= (plus >= 0);
			dragMoved	= false;
			dragStart	= where;
			if (dragAdded) {
				BPoint	point;
				PlusPoint(plus,&point);
				dragPoints.insert(dragPoints.begin() + plus,point);
				dragBend	= plus;
			}
			else
				dragBend	= handle;
			editor->SetMouseEventMask(B_POINTER_EVENTS, B_NO_POINTER_HISTORY | B_SUSPEND_VIEW_FOCUS | B_LOCK_WINDOW_FOCUS);
			return;
		}
	}
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
	if ((dragBend < 0) || (pt == dragStart))
		return;
	if (editor->GridEnabled()) {
		float	grid	= editor->GridWidth();
		pt.x	= roundf(pt.x / grid) * grid;
		pt.y	= roundf(pt.y / grid) * grid;
	}
	dragPoints[dragBend]	= pt;
	dragMoved				= true;
	editor->Invalidate();
}

void ConnectionRenderer::MouseUp(BPoint where) {
	if (dragBend < 0)
		return;
	// a click on a handle only selects it, one on a "+" adds the bend
	selectedBend	= dragBend;
	dragBend		= -1;
	if (dragMoved || dragAdded) {
		// shown as is until the command's change comes back
		bends	= dragPoints;
		SendBends(dragPoints);
	}
	editor->Invalidate();
}

BCursorID ConnectionRenderer::CursorAt(BPoint where) {
	if (dragBend >= 0)
		return B_CURSOR_ID_MOVE;
	if (!selected)
		return B_CURSOR_ID_SYSTEM_DEFAULT;
	if (HandleAt(where) >= 0)
		return B_CURSOR_ID_MOVE;
	if (PlusAt(where) >= 0)
		return B_CURSOR_ID_COPY;
	return B_CURSOR_ID_SYSTEM_DEFAULT;
}

BPoint ConnectionRenderer::SelectedBendPoint(void) const {
	if ((selectedBend < 0) || (selectedBend >= (int32)handles.size()))
		return BPoint(0,0);
	return handles[selectedBend];
}

void ConnectionRenderer::DeleteSelectedBend(void) {
	if ((selectedBend < 0) || (selectedBend >= (int32)handles.size()))
		return;
	std::vector<BPoint>	points	= handles;
	points.erase(points.begin() + selectedBend);
	selectedBend	= -1;
	SendBends(points);
}

void ConnectionRenderer::AutoRoute(void) {
	selectedBend	= -1;
	SendBends(std::vector<BPoint>());
}

void ConnectionRenderer::SendBends(const std::vector<BPoint> &points) {
	BMessage	bendsMessage;
	for (size_t i = 0; i < points.size(); i++)
		bendsMessage.AddPoint(P_C_BEND_POINT,points[i]);
	BMallocIO	flat;
	bendsMessage.Flatten(&flat);
	BMessage	valueContainer;
	valueContainer.AddString("name",P_C_NODE_CONNECTION_BENDS);
	valueContainer.AddInt32("type",B_MESSAGE_TYPE);
	valueContainer.AddData("newValue",B_MESSAGE_TYPE,flat.Buffer(),flat.BufferLength());
	BMessage	change(P_C_EXECUTE_COMMAND);
	change.AddString("Command::Name","ChangeValue");
	change.AddPointer("node",container);
	change.AddMessage("valueContainer",&valueContainer);
	sentTo->SendMessage(&change);
}

void ConnectionRenderer::LanguageChanged() {
}

void ConnectionRenderer::Draw(BView *drawOn, BRect updateRect) {
	if (editor->ForPaper() && (selected)) {
		// paper shows the graph, not the selection or the pointer
		bool	wasSelected	= selected;
		selected	= false;
		Draw(drawOn,updateRect);
		selected	= wasSelected;
		return;
	}

	CalcLine();
	DrawRoute(drawOn);
	if (selected)
		DrawHandles(drawOn);
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
	// tmpNode can legitimately be NULL - an endpoint whose id never
	// resolved (see Indexer::DeIndexConnection()) has no
	// P_C_NODE_CONNECTION_FROM/TO field at all here. Leave from/to as
	// they are (NULL from Init(), or whatever they last resolved to) -
	// CalcLine() already only draws once both are non-NULL.
	BMessage	*tmpNode	= NULL;
	if (container->FindPointer(P_C_NODE_CONNECTION_FROM,(void **)&tmpNode) == B_OK)
		tmpNode->FindPointer(editor->RenderString(),(void **)&from);
	if (container->FindPointer(P_C_NODE_CONNECTION_TO,(void **)&tmpNode) == B_OK)
		tmpNode->FindPointer(editor->RenderString(),(void **)&to);
	selected		= container->GetBool(P_C_NODE_SELECTED,false);
	if (!selected)
		selectedBend	= -1;
	connectionType	= container->GetInt8(P_C_NODE_CONNECTION_TYPE,connectionType);
	// added empty for the same reason as the arrows below
	BMessage	bendsMessage;
	if (container->FindMessage(P_C_NODE_CONNECTION_BENDS,&bendsMessage) != B_OK)
		container->AddMessage(P_C_NODE_CONNECTION_BENDS,&bendsMessage);
	bends.clear();
	BPoint		bend;
	for (int32 i = 0; bendsMessage.FindPoint(P_C_BEND_POINT,i,&bend) == B_OK; i++)
		bends.push_back(bend);
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
		penSize	= pattern.GetFloat("PenSize",penSize);
		rgb_color	color;
		if (pattern.FindInt32("FillColor",(int32 *)&color) == B_OK)
			fillColor	= color;
	}
	// a real committed value just arrived - drop any leftover preview
	// from a picker session, same reasoning as ClassRenderer::ValueChanged()
	hasPreviewFillColor	= false;
}

void ConnectionRenderer::SetPreviewColor(const char *field, rgb_color color) {
	// a connection's line is its fill color
	if (strcmp(field,"FillColor") != 0)
		return;
	hasPreviewFillColor	= true;
	previewFillColor	= color;
}

void ConnectionRenderer::ClearPreviewColor(void) {
	hasPreviewFillColor	= false;
}

void ConnectionRenderer::InvalidateEndpoint(Renderer *removed) {
	if ((Renderer *)from == removed)
		from = NULL;
	if ((Renderer *)to == removed)
		to = NULL;
}

void ConnectionRenderer::CalcLine() {
	// the head grows with the line, or a thick line outgrows it
	const float	lineWidth	= (penSize <= 1.0f) ? editor->Style().connectionWidth : penSize;
	const float	arrowSize	= fmax(::arrowSize,lineWidth*2.5f+2.0f);
	legs.clear();
	handles.clear();
	if (from != NULL &&  to != NULL)
	{
		const std::vector<BPoint>	&route	= (dragMoved || dragAdded) && (dragBend >= 0)
			? dragPoints : bends;
		BRect	*fromRect	= new BRect(from->Frame());
		BRect	*toRect		= new BRect(to->Frame());
		BPoint	fromOutward	= BPoint(0,0);
		BPoint	toOutward	= BPoint(0,0);
		float	toMiddleX 	=	(toRect->right-toRect->left)/2;
		float	toMiddleY	=	(toRect->bottom-toRect->top)/2;
		alpha		= atan2((toRect->top-fromRect->top),(toRect->left-fromRect->left));
		if (!route.empty()) {
			CalcFixedEnds(route,&fromOutward,&toOutward);
			// the same head the branches below build, toward the last bend
			BPoint	base(third.x+toOutward.x*arrowSize,third.y+toOutward.y*arrowSize);
			first		= BPoint(base.x-toOutward.y*arrowSize,base.y+toOutward.x*arrowSize);
			second		= BPoint(base.x+toOutward.y*arrowSize,base.y-toOutward.x*arrowSize);
			toPoint		= base;
		}
		else if ( (alpha < -M_PI_3_4 ) || (alpha > M_PI_3_4) ) {
			third		= to->Anchor(BPoint(toRect->right,toRect->top+toMiddleY));
			first		= BPoint(third.x+arrowSize,third.y-arrowSize);
			second		= BPoint(first.x,third.y+arrowSize);
			toPoint		= BPoint(first.x,third.y);
			fromPoint	= from->Anchor(BPoint(fromRect->left,fromRect->top+(fromRect->bottom-fromRect->top)/2));
			fromOutward	= BPoint(-1,0);
			float	bendLength	= BEND_LENGTH* (fromPoint.x-toPoint.x);
			firstBend.x		= fromPoint.x - bendLength;
			firstBend.y		= fromPoint.y;
			secondBend.x	= toPoint.x + bendLength;
			secondBend.y	= toPoint.y;
		}
		else if (alpha < -M_PI_4) {
			third		= to->Anchor(BPoint(toRect->left+toMiddleX,toRect->bottom));
			first		= BPoint(third.x-arrowSize,third.y+arrowSize);
			second		= BPoint(third.x+arrowSize,third.y+arrowSize);
			toPoint		= BPoint(third.x,first.y);
			fromPoint	= from->Anchor(BPoint(fromRect->left+(fromRect->right-fromRect->left)/2,fromRect->top));
			fromOutward	= BPoint(0,-1);
			float	bendLength	= BEND_LENGTH* (fromPoint.y-toPoint.y);
			firstBend.x		= fromPoint.x;
			firstBend.y		= fromPoint.y - bendLength;
			secondBend.x	= toPoint.x;
			secondBend.y	= toPoint.y + bendLength;
		}
		else if (alpha> M_PI_4) {
			third		= to->Anchor(BPoint(toRect->left+toMiddleX,toRect->top));
			first		= BPoint(third.x-arrowSize,third.y-arrowSize);
			second		= BPoint(third.x+arrowSize,third.y-arrowSize);
			toPoint		= BPoint(third.x,first.y);
			fromPoint	= from->Anchor(BPoint(fromRect->left+(fromRect->right-fromRect->left)/2,fromRect->bottom));
			fromOutward	= BPoint(0,1);
			float	bendLength	= BEND_LENGTH* (toPoint.y-fromPoint.y);
			firstBend.x		= fromPoint.x;
			firstBend.y		= fromPoint.y + bendLength;
			secondBend.x	= toPoint.x;
			secondBend.y	= toPoint.y - bendLength;

		}
		else {
			third		= to->Anchor(BPoint(toRect->left,toRect->top+toMiddleY));
			first		= BPoint(third.x-arrowSize,third.y-arrowSize);
			second		= BPoint(third.x-arrowSize,third.y+arrowSize);
			toPoint		= BPoint(first.x,third.y);
			fromPoint	= from->Anchor(BPoint(fromRect->right,fromRect->top+(fromRect->bottom-fromRect->top)/2));
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
		if (route.empty())
			CalcAutoLegs();
		else
			CalcFixedLegs(route,fromOutward,toOutward);
	}
	if (selectedBend >= (int32)handles.size())
		selectedBend	= -1;
}


// the middle of the frame's side facing toward - measured against the
// frame's diagonal, so a wide node docks on top for a point above it
static BPoint FacingSide(BRect frame, BPoint toward, BPoint *outward)
{
	float	midX	= (frame.left + frame.right) / 2;
	float	midY	= (frame.top + frame.bottom) / 2;
	float	dx		= toward.x - midX;
	float	dy		= toward.y - midY;
	if (fabs(dx) * frame.Height() > fabs(dy) * frame.Width()) {
		*outward	= BPoint((dx > 0) ? 1 : -1,0);
		return BPoint((dx > 0) ? frame.right : frame.left,midY);
	}
	*outward	= BPoint(0,(dy > 0) ? 1 : -1);
	return BPoint(midX,(dy > 0) ? frame.bottom : frame.top);
}


static BPoint Cubic(BPoint start, BPoint control1, BPoint control2, BPoint end, float t)
{
	float	u	= 1 - t;
	float	a	= u*u*u;
	float	b	= 3*u*u*t;
	float	c	= 3*u*t*t;
	float	d	= t*t*t;
	return BPoint(a*start.x + b*control1.x + c*control2.x + d*end.x,
		a*start.y + b*control1.y + c*control2.y + d*end.y);
}


void ConnectionRenderer::CalcFixedEnds(const std::vector<BPoint> &route,
	BPoint *fromOutward, BPoint *toOutward)
{
	fromPoint	= from->Anchor(FacingSide(from->Frame(),route.front(),fromOutward));
	third		= to->Anchor(FacingSide(to->Frame(),route.back(),toOutward));
}


void ConnectionRenderer::CalcAutoLegs(void)
{
	std::vector<BPoint>	leg;
	leg.push_back(fromPoint);
	if (connectionType == 1) {
		// split at a third and two thirds: the handles sit on the curve,
		// not on its control points
		const int32	steps	= 48;
		for (int32 k = 1; k <= steps; k++) {
			leg.push_back(PointOnBezier((float)k / steps));
			if ((k % (steps / 3) == 0) && (k < steps)) {
				handles.push_back(leg.back());
				legs.push_back(leg);
				leg.clear();
				leg.push_back(handles.back());
			}
		}
	}
	else if (connectionType == 2) {
		handles.push_back(firstBend);
		handles.push_back(secondBend);
		leg.push_back(firstBend);
		legs.push_back(leg);
		leg.clear();
		leg.push_back(firstBend);
		leg.push_back(secondBend);
		legs.push_back(leg);
		leg.clear();
		leg.push_back(secondBend);
		leg.push_back(toPoint);
	}
	else
		leg.push_back(toPoint);
	legs.push_back(leg);
}


void ConnectionRenderer::CalcFixedLegs(const std::vector<BPoint> &route,
	BPoint fromOutward, BPoint toOutward)
{
	handles	= route;
	std::vector<BPoint>	points;
	points.push_back(fromPoint);
	points.insert(points.end(),route.begin(),route.end());
	points.push_back(toPoint);
	int32	last	= points.size() - 1;
	if (connectionType == 1) {
		// a smooth curve through the points: leaving and entering the
		// nodes straight out of their sides, in between along the
		// neighbours (Catmull-Rom)
		std::vector<BPoint>	tangent(points.size());
		float	firstLength	= Distance(points[0],points[1]);
		float	lastLength	= Distance(points[last-1],points[last]);
		tangent[0]		= BPoint(fromOutward.x*firstLength,fromOutward.y*firstLength);
		tangent[last]	= BPoint(-toOutward.x*lastLength,-toOutward.y*lastLength);
		for (int32 i = 1; i < last; i++)
			tangent[i]	= BPoint((points[i+1].x-points[i-1].x)/2,(points[i+1].y-points[i-1].y)/2);
		for (int32 i = 0; i < last; i++) {
			BPoint	control1(points[i].x+tangent[i].x/3,points[i].y+tangent[i].y/3);
			BPoint	control2(points[i+1].x-tangent[i+1].x/3,points[i+1].y-tangent[i+1].y/3);
			std::vector<BPoint>	leg;
			for (int32 k = 0; k <= 16; k++)
				leg.push_back(Cubic(points[i],control1,control2,points[i+1],k / 16.0f));
			legs.push_back(leg);
		}
	}
	else if (connectionType == 2) {
		// every bend is a corner: each leg starts along the source side's
		// axis and turns once; the last one has to arrive along the
		// target side's axis, which takes a second turn if they agree
		bool	horizontal		= (fromOutward.x != 0);
		bool	endHorizontal	= (toOutward.x != 0);
		for (int32 i = 0; i < last; i++) {
			BPoint	start	= points[i];
			BPoint	end		= points[i+1];
			std::vector<BPoint>	leg;
			leg.push_back(start);
			if ((i == last-1) && (horizontal == endHorizontal)) {
				if (horizontal) {
					float	middle	= (start.x+end.x)/2;
					leg.push_back(BPoint(middle,start.y));
					leg.push_back(BPoint(middle,end.y));
				}
				else {
					float	middle	= (start.y+end.y)/2;
					leg.push_back(BPoint(start.x,middle));
					leg.push_back(BPoint(end.x,middle));
				}
			}
			else if (horizontal)
				leg.push_back(BPoint(end.x,start.y));
			else
				leg.push_back(BPoint(start.x,end.y));
			leg.push_back(end);
			legs.push_back(leg);
		}
	}
	else {
		for (int32 i = 0; i < last; i++) {
			std::vector<BPoint>	leg;
			leg.push_back(points[i]);
			leg.push_back(points[i+1]);
			legs.push_back(leg);
		}
	}
}


BRect ConnectionRenderer::Frame()
{
	CalcLine();
	BRect	frame(fromPoint,fromPoint);
	frame	= frame | BRect(toPoint,toPoint);
	for (size_t i = 0; i < legs.size(); i++) {
		for (size_t j = 0; j < legs[i].size(); j++)
			frame	= frame | BRect(legs[i][j],legs[i][j]);
	}
	return frame;
}

bool ConnectionRenderer::Caught(BPoint where){
	if (selected && ((HandleAt(where) >= 0) || (PlusAt(where) >= 0)))
		return true;
	for (size_t i = 0; i < legs.size(); i++) {
		for (size_t j = 1; j < legs[i].size(); j++) {
			if (DistanceToSegment(where,legs[i][j-1],legs[i][j]) < max_entfernung)
				return true;
		}
	}
	return false;
}

// in view pixels, whatever the zoom
static const float	kHandleRadius	= 4.5;
static const float	kPlusHalf		= 5;
static const float	kMinPlusLeg		= 36;

void ConnectionRenderer::DrawRoute(BView *drawOn){
	const GraphStyle	&style	= editor->Style();
	drawOn->SetPenSize(LineWidth(style));
	drawOn->SetLineMode(B_ROUND_CAP,B_ROUND_JOIN);
	drawOn->SetHighColor(LineColor(style));
	// short StrokeLine()s rather than StrokeShape() for the curves:
	// StrokeShape() was verified live to ignore BView::SetScale() in this
	// Haiku build, painting the curve unscaled at any zoom but 100%
	for (size_t i = 0; i < legs.size(); i++) {
		for (size_t j = 1; j < legs[i].size(); j++)
			drawOn->StrokeLine(legs[i][j-1],legs[i][j]);
	}
	DrawArrows(drawOn);
}

void ConnectionRenderer::DrawHandles(BView *drawOn){
	const GraphStyle	&style	= editor->Style();
	float	scale	= editor->Scale();
	float	radius	= kHandleRadius / scale;
	float	half	= kPlusHalf / scale;
	float	arm		= half * 0.55f;
	drawOn->SetPenSize(1.5f / scale);
	for (int32 i = 0; i < (int32)legs.size(); i++) {
		BPoint	plus;
		if (!PlusPoint(i,&plus))
			continue;
		BRect	box(plus.x-half,plus.y-half,plus.x+half,plus.y+half);
		drawOn->SetHighColor(style.cardFill);
		drawOn->FillRect(box);
		drawOn->SetHighColor(style.accent);
		drawOn->StrokeRect(box);
		drawOn->StrokeLine(BPoint(plus.x-arm,plus.y),BPoint(plus.x+arm,plus.y));
		drawOn->StrokeLine(BPoint(plus.x,plus.y-arm),BPoint(plus.x,plus.y+arm));
	}
	for (int32 i = 0; i < (int32)handles.size(); i++) {
		drawOn->SetHighColor((i == selectedBend) ? style.accent : style.cardFill);
		drawOn->FillEllipse(handles[i],radius,radius);
		drawOn->SetHighColor(style.accent);
		drawOn->StrokeEllipse(handles[i],radius,radius);
	}
}

int32 ConnectionRenderer::HandleAt(BPoint where){
	float	reach	= (kHandleRadius + 3) / editor->Scale();
	for (int32 i = handles.size() - 1; i >= 0; i--) {
		if (Distance(where,handles[i]) <= reach)
			return i;
	}
	return -1;
}

int32 ConnectionRenderer::PlusAt(BPoint where){
	float	reach	= (kPlusHalf + 2) / editor->Scale();
	for (int32 i = 0; i < (int32)legs.size(); i++) {
		BPoint	plus;
		if (PlusPoint(i,&plus) && (fabs(where.x-plus.x) <= reach)
				&& (fabs(where.y-plus.y) <= reach))
			return i;
	}
	return -1;
}

bool ConnectionRenderer::PlusPoint(int32 i, BPoint *point){
	if ((i < 0) || (i >= (int32)legs.size()))
		return false;
	const std::vector<BPoint>	&leg	= legs[i];
	float	length	= 0;
	for (size_t j = 1; j < leg.size(); j++)
		length	+= Distance(leg[j-1],leg[j]);
	if (length * editor->Scale() < kMinPlusLeg)
		return false;
	float	left	= length / 2;
	for (size_t j = 1; j < leg.size(); j++) {
		float	step	= Distance(leg[j-1],leg[j]);
		if ((step > 0) && (left <= step)) {
			float	t	= left / step;
			*point	= BPoint(leg[j-1].x + t*(leg[j].x-leg[j-1].x),
				leg[j-1].y + t*(leg[j].y-leg[j-1].y));
			return true;
		}
		left	-= step;
	}
	*point	= leg.back();
	return true;
}

rgb_color ConnectionRenderer::LineColor(const GraphStyle &style) {
	if (selected)
		return style.accent;
	rgb_color	color	= EffectiveFillColor();
	if (!hasPreviewFillColor && (color.red == kDefaultConnectionColor.red)
		&& (color.green == kDefaultConnectionColor.green)
		&& (color.blue == kDefaultConnectionColor.blue))
		return style.connection;
	return color;
}

float ConnectionRenderer::LineWidth(const GraphStyle &style) {
	// the standard pen size of 1 means "not set"
	float	width	= (penSize <= 1.0f) ? style.connectionWidth : penSize;
	return selected ? width + 0.75f : width;
}

void ConnectionRenderer::DrawArrows(BView *drawOn) {
	if (arrows & 1)
		drawOn->FillTriangle(first,second,third);
	if (arrows & 2)
		drawOn->FillTriangle(fromFirst,fromSecond,fromThird);
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

