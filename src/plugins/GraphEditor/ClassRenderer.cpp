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
#include "SmartGuides.h"

// #127: candidate frames for GuidesEnabled() to align against - every
// unselected node renderer (the dragged node's own selected siblings are
// moving in lockstep this tick, see MoveAll(), and would be a moving
// target; connections have no meaningful Frame() to align to here).
static BList*
CollectGuideTargets(GraphEditor *editor)
{
	BList	*targets	= new BList();
	BList	*renderers	= editor->RenderList();
	for (int32 i=0;i<renderers->CountItems();i++) {
		Renderer	*candidate	= (Renderer *)renderers->ItemAt(i);
		if (candidate->Selected())
			continue;
		if (candidate->GetMessage()->what == P_C_CONNECTION_TYPE)
			continue;
		targets->AddItem(new BRect(candidate->Frame()));
	}
	return targets;
}

static void
DeleteGuideTargets(BList *targets)
{
	for (int32 i=0;i<targets->CountItems();i++)
		delete (BRect *)targets->ItemAt(i);
	delete targets;
}

// Screen-space snap distance, converted to document space the same way
// grid-snap's own dx/dy math already implicitly works in document units -
// GraphEditor::Scale() is applied to mouse coordinates well before they
// reach here (see GraphEditor::MouseMoved()'s scaledWhere), so "pt"/
// "startFrame" here are already document-space; the threshold just needs
// the same conversion so it stays a constant number of *screen* pixels
// regardless of zoom.
static const float	kGuideScreenThreshold	= 8.0f;


ClassRenderer::ClassRenderer(GraphEditor *parentEditor, BMessage *forContainer):Renderer(parentEditor, forContainer)
{
	TRACE();
	Init();
	ValueChanged();
	initialized	= true;
}
void ClassRenderer::Init()
{
	TRACE();
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
	hasPreviewFillColor			= false;
	animating					= false;
	animPosX = animPosY		= 0;
	animVelX = animVelY		= 0;
	initialized					= false;

	BMessage	*editMessage		= new BMessage(P_C_EXECUTE_COMMAND);
	editMessage->AddPointer("node",container);

	editMessage->AddString("Command::Name","ChangeValue");
	BMessage	*valueContainer	= new BMessage();
	valueContainer->AddString("name",P_C_NODE_NAME);
	valueContainer->AddString("subgroup",P_C_NODE_DATA);
	valueContainer->AddInt32("type",B_STRING_TYPE);
	editMessage->AddMessage("valueContainer",valueContainer);
	name						= new StringRenderer(editor,"",BRect(0,0,100,100), editMessage);
	err 						= container->FindPointer("ProjectConceptor::doc",(void **)&doc);
	sentTo						= new BMessenger(NULL,doc);
	if (container->FindFloat(P_C_NODE_X_RADIUS,&xRadius) != B_OK)
		container->AddFloat(P_C_NODE_X_RADIUS,7.0);
	if (container->FindFloat(P_C_NODE_Y_RADIUS,&yRadius) != B_OK)
		container->AddFloat(P_C_NODE_Y_RADIUS,7.0);
	container->FindPointer(P_C_NODE_PARENT, (void **)&parentNode);

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
			else if (SupportsResize()
					&& BRect(ResizeCorner()-BPoint(3*circleSize,3*circleSize),ResizeCorner()).Contains(where)) {
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
			else if (!resizing && editor->GuidesEnabled()) {
				// Same absolute-from-drag-start shape as the grid branch
				// above (not incremental from oldPt) - guide matches have
				// to be recomputed against the true current candidate
				// position every tick, not accumulated.
				float	rawDx			= pt.x - startMouseDown->x;
				float	rawDy			= pt.y - startMouseDown->y;
				BList	*targets		= CollectGuideTargets(editor);
				float	thresholdDoc	= kGuideScreenThreshold / editor->Scale();
				GuideSnapResult	snap	= ComputeGuideSnap(*startFrame,rawDx,rawDy,targets,thresholdDoc);
				DeleteGuideTargets(targets);
				dx	= (startFrame->left + snap.dx) - frame.left;
				dy	= (startFrame->top + snap.dy) - frame.top;
				editor->SetActiveGuides(snap);
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
			connecter->AddPoint(P_C_NODE_CONNECTION_FROM,*startMouseDown);
			connecter->AddPoint(P_C_NODE_CONNECTION_TO,pt);
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
			else if (!resizing && editor->GuidesEnabled()) {
				// dx/dy above are already absolute-from-startFrame (same
				// shape ComputeGuideSnap() expects/returns), unlike
				// MouseMoved()'s incremental case - matches the last
				// on-screen preview exactly.
				BList	*targets		= CollectGuideTargets(editor);
				float	thresholdDoc	= kGuideScreenThreshold / editor->Scale();
				GuideSnapResult	snap	= ComputeGuideSnap(*startFrame,dx,dy,targets,thresholdDoc);
				DeleteGuideTargets(targets);
				dx	= snap.dx;
				dy	= snap.dy;
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
			connecter->AddPointer(P_C_NODE_CONNECTION_FROM,container);
			connecter->AddPoint(P_C_NODE_CONNECTION_TO,where);
			(new BMessenger((BView *)editor))->SendMessage(connecter);
		}
		if (startMouseDown) delete startMouseDown;
		if (oldPt) delete oldPt;
		startMouseDown	= NULL;
		oldPt			= NULL;
		connecting		= 0;
		editor->ClearActiveGuides();
		editor->Invalidate();
	}
}


// The node's color as a band across the top of a rounded card: the card's
// own outline down to height, sampled along the corner arcs.
static void FillStripe(BView *view, BRect frame, float radius, float height)
{
	if (height > radius)
		height	= radius;
	const int32		steps	= 6;
	vector<BPoint>	points;
	for (int32 i = 0; i <= steps; i++) {
		float	y	= frame.top + height - height * i / steps;
		float	dy	= radius - (y - frame.top);
		float	dx	= sqrtf(radius * radius - dy * dy);
		points.push_back(BPoint(frame.left + radius - dx, y));
	}
	for (int32 i = steps; i >= 0; i--) {
		float	y	= frame.top + height - height * i / steps;
		float	dy	= radius - (y - frame.top);
		float	dx	= sqrtf(radius * radius - dy * dy);
		points.push_back(BPoint(frame.right - radius + dx, y));
	}
	view->FillPolygon(&points[0], points.size());
}

rgb_color ClassRenderer::CardBorderColor(const GraphStyle &style) {
	// the editor's standard border means "not set": then the style's,
	// which also works in a dark color scheme
	rgb_color	standard;
	if ((editor->GetStandartPattern()->FindInt32("BorderColor",(int32 *)&standard) == B_OK)
		&& (standard.red == borderColor.red) && (standard.green == borderColor.green)
		&& (standard.blue == borderColor.blue))
		return style.cardBorder;
	return borderColor;
}

void ClassRenderer::Draw(BView *drawOn, BRect updateRect) {
	// mid-slide: shift this node's whole draw (shape+name+attributes+
	// connectors, all already positioned at the real/final frame) so it
	// paints at the current animated position instead - frame itself and
	// every child stay at their real, final values throughout (see
	// AnimationStep()/ValueChanged()).
	bool	offsetForAnim	= animating;
	BPoint	priorOrigin		= drawOn->Origin();
	if (offsetForAnim) {
		BPoint	delta(animPosX-frame.left,animPosY-frame.top);
		drawOn->PushState();
		drawOn->SetOrigin(priorOrigin+delta);
	}

	const GraphStyle	&style		= editor->Style();
	const float			radius		= style.cornerRadius;
	rgb_color			nodeColor	= hasPreviewFillColor ? previewFillColor : fillColor;
	bool				fitIn		= true;
	drawOn->SetFont(font);

	editor->Shadows().Draw(drawOn,shape,shape.Name(),frame,radius,
		style.shadow,style.shadowBlur,style.shadowOffsetY);

	if (selected) {
		BRect	outline	= frame;
		outline.InsetBy(-style.selectionGap,-style.selectionGap);
		// the gap between ring and card shows the canvas, not the shadow
		NodeShape	grown(shape);
		grown.Layout(outline);
		drawOn->SetHighColor(style.canvas);
		if (shape.HasPath())
			grown.Fill(drawOn);
		else
			drawOn->FillRoundRect(outline,radius+style.selectionGap,radius+style.selectionGap);
		drawOn->SetPenSize(style.selectionWidth);
		drawOn->SetHighColor(style.accent);
		if (shape.HasPath())
			grown.Stroke(drawOn);
		else
			drawOn->StrokeRoundRect(outline,radius+style.selectionGap,radius+style.selectionGap);
	}

	drawOn->SetHighColor(style.cardFill);
	if (shape.HasPath())
		shape.Fill(drawOn);
	else
		drawOn->FillRoundRect(frame,radius,radius);

	if (shape.HasPath()) {
		// no room for a stripe: the outline carries the node's color
		drawOn->SetPenSize(1.5);
		drawOn->SetHighColor(nodeColor);
		shape.Stroke(drawOn);
	} else {
		drawOn->SetHighColor(nodeColor);
		FillStripe(drawOn,frame,radius,style.stripeHeight);
		drawOn->SetPenSize(1.0);
		drawOn->SetHighColor(CardBorderColor(style));
		drawOn->StrokeRoundRect(frame,radius,radius);
	}

	if (SupportsResize()) {
		BPoint	corner	= ResizeCorner();
		drawOn->SetHighColor(style.mutedText);
		drawOn->FillTriangle(BPoint(corner.x-(3*circleSize),corner.y),BPoint(corner.x,corner.y-(3*circleSize)),corner);
	}

	if (showConnecter) {
		drawOn->SetPenSize(1.0);
		drawOn->SetHighColor(style.cardFill);
		drawOn->FillEllipse(leftConnection);
		drawOn->FillEllipse(topConnection);
		drawOn->FillEllipse(rightConnection);
		drawOn->FillEllipse(bottomConnection);
		drawOn->SetHighColor(style.accent);
		drawOn->StrokeEllipse(leftConnection);
		drawOn->StrokeEllipse(topConnection);
		drawOn->StrokeEllipse(rightConnection);
		drawOn->StrokeEllipse(bottomConnection);
	}

	name->Draw(drawOn,updateRect);
	drawOn->SetHighColor(style.text);
	BRect	content	= ContentFrame();
	vector<Renderer *>::iterator	allAttributes = attributes->begin();
	while( allAttributes != attributes->end() ) {
		if (content.Contains((*allAttributes)->Frame()))
			(*allAttributes)->Draw(drawOn,updateRect);
		else
			fitIn=false;
		allAttributes++;
	}
	if (!fitIn) {
		drawOn->SetHighColor(style.mutedText);
		drawOn->DrawString("…",BPoint(content.left+style.paddingX,content.bottom-(style.paddingY/2)));
	}

	if (offsetForAnim)
		drawOn->PopState();
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

	BRect	oldFrame	= frame;
	container->FindRect(P_C_NODE_FRAME,&frame);
	if ((initialized) && (oldFrame.LeftTop() != frame.LeftTop())) {
		if (!animating) {
			animPosX	= oldFrame.left;
			animPosY	= oldFrame.top;
			animVelX	= 0;
			animVelY	= 0;
		}
		// else: already mid-animation (e.g. Auto-Layout run again before the
		// last one settled) - keep the current animated position/velocity as
		// the start of the new leg instead of snapping back to oldFrame.
		animating	= true;
		editor->StartAnimating(this);
	}
	container->FindBool(P_C_NODE_SELECTED,&selected);
	container->FindFloat(P_C_NODE_X_RADIUS,&xRadius);
	container->FindFloat(P_C_NODE_Y_RADIUS,&yRadius);
	if (container->FindMessage(P_C_NODE_PATTERN,pattern) != B_OK) {
		container->AddMessage(P_C_NODE_PATTERN,editor->GetStandartPattern());
		pattern = editor->GetStandartPattern();
	}
	if (container->FindMessage(P_C_NODE_FONT,messageFont) == B_OK) {
		delete font;
		font	= new AFont(messageFont);
	}
	container->FindMessage(P_C_NODE_DATA,data);
	pattern->FindInt32("FillColor",(int32 *)&fillColor);
	pattern->FindInt32("BorderColor",(int32 *)&borderColor);
	pattern->FindFloat("PenSize",&penSize);
	// a real committed value just arrived (this is only ever called from
	// a genuine P_C_VALUE_CHANGED, never from the preview path below) -
	// any leftover preview from a picker session is now stale, drop it
	// so Draw() goes back to the real fillColor just read above
	hasPreviewFillColor			= false;
	// older documents have no shape yet; like the pattern above it is
	// added so ChangeValue has a field to replace
	BMessage	shapeMessage;
	if (container->FindMessage(P_C_NODE_SHAPE,&shapeMessage) != B_OK) {
		NodeShape::BuildBuiltIn("rounded",&shapeMessage);
		container->AddMessage(P_C_NODE_SHAPE,&shapeMessage);
	}
	shape.SetTo(&shapeMessage);
	shape.Layout(frame);
	data->FindString(P_C_NODE_NAME,(const char **)&newName);
	name->SetString(newName);
	const GraphStyle	&style	= editor->Style();
	BFont	nameFont(be_bold_font);
	nameFont.SetSize(style.nameFontSize);
	name->SetFont(nameFont);
	name->SetColor(style.text);
	BRect	content		= ContentFrame();
	// a shape's text area already keeps clear of its outline
	float	padding		= shape.HasPath() ? style.paddingX/3 : style.paddingX;
	float	nameTop		= content.top+(shape.HasPath() ? 0 : style.stripeHeight)+(style.paddingY/2);
	name->SetFrame(BRect(content.left+padding-2,nameTop,content.right-padding,nameTop+12));
	
	
	//delete all "old" Attribs
	attributes->erase(attributes->begin(),attributes->end());
	//and add all attribs we found
	// every entry of a name gets its own row - AddAttribute appends, so
	// a name may hold several values (#138)
	for (int32 i = 0; data->GetInfo(B_MESSAGE_TYPE, i,(char **) &attribName, &type, &count) == B_OK; i++) {
		for (int32 entry = 0; entry < count; entry++) {
			if (data->FindMessage(attribName,entry,attribMessage) == B_OK)
				InsertAttribute(attribName,attribMessage, entry);
		}
	}
	container->FindPointer(P_C_NODE_PARENT, (void **)&parentNode);
	shape.Layout(frame);
	UpdateConnectors();

	// this node's own frame just changed via a committed command (Move,
	// ChangeValue/Auto-Layout, either one's Undo, ...) rather than an
	// interactive drag - MouseMoved()'s own live-drag path already
	// cascades to a parent group directly (same pattern as here), but
	// none of those commands mark the *parent* as changed (only this
	// node), so without this the group's box never learns a lone child
	// moved/resized outside of a drag (issue #38).
	if ((oldFrame != frame) && (parentNode != NULL)) {
		GroupRenderer	*parent	= NULL;
		if (parentNode->FindPointer(editor->RenderString(),(void **)&parent) == B_OK)
			parent->RecalcFrame();
	}
}

BRect ClassRenderer::Frame( void ) {
	return frame;
}

void ClassRenderer::UpdateConnectors(void) {
	float	yMiddle	= frame.top+(frame.Height()/2);
	float	xMiddle	= frame.left+(frame.Width()/2);
	BPoint	left	= shape.Anchor(BPoint(frame.left,yMiddle));
	BPoint	top		= shape.Anchor(BPoint(xMiddle,frame.top));
	BPoint	right	= shape.Anchor(BPoint(frame.right,yMiddle));
	BPoint	bottom	= shape.Anchor(BPoint(xMiddle,frame.bottom));
	leftConnection.Set(left.x-circleSize,left.y-circleSize,left.x+circleSize,left.y+circleSize);
	topConnection.Set(top.x-circleSize,top.y-circleSize,top.x+circleSize,top.y+circleSize);
	rightConnection.Set(right.x-circleSize,right.y-circleSize,right.x+circleSize,right.y+circleSize);
	bottomConnection.Set(bottom.x-circleSize,bottom.y-circleSize,bottom.x+circleSize,bottom.y+circleSize);
}

bool  ClassRenderer::Caught(BPoint where) {
	 bool contains	= shape.HasPath() ? shape.Contains(where) : frame.Contains(where);
	 if (!contains && SupportsResize())
		contains = BRect(ResizeCorner()-BPoint(3*circleSize,3*circleSize),ResizeCorner()).Contains(where);
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

// see the identical guard in the Move command (Move.cpp): a group's own
// MoveBy() cascades into its children, so a child that is selected in its
// own right must not be moved again from here
static bool HasSelectedAncestor(BMessage *node)
{
	BMessage	*parent		= NULL;
	bool		isSelected	= false;
	while ((node != NULL)
			&& (node->FindPointer(P_C_NODE_PARENT,(void **)&parent) == B_OK)
			&& (parent != NULL)) {
		isSelected	= false;
		if ((parent->FindBool(P_C_NODE_SELECTED,&isSelected) == B_OK) && isSelected)
			return true;
		node	= parent;
		parent	= NULL;
	}
	return false;
}

bool  ClassRenderer::MoveAll(void *arg,float dx, float dy) {
	Renderer	*renderer	= (Renderer*)arg;
	if (renderer->Selected() && !HasSelectedAncestor(renderer->GetMessage()))
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
	shape.Layout(frame);
	UpdateConnectors();
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
	shape.Layout(frame);
	UpdateConnectors();
}

void ClassRenderer::SetPreviewFillColor(rgb_color color) {
	hasPreviewFillColor	= true;
	previewFillColor	= color;
}

void ClassRenderer::ClearPreviewFillColor(void) {
	hasPreviewFillColor	= false;
}

bool ClassRenderer::AnimationStep(float dt) {
	if (!animating)
		return false;
	// critically-damped-ish spring toward frame.LeftTop() (the already-
	// committed target) - k=stiffness, c=damping, mass=1.
	const float	k		= 180.0f;
	const float	c		= 24.0f;
	float		targetX	= frame.left;
	float		targetY	= frame.top;
	float		accelX	= k*(targetX-animPosX) - c*animVelX;
	float		accelY	= k*(targetY-animPosY) - c*animVelY;
	animVelX	+= accelX*dt;
	animVelY	+= accelY*dt;
	animPosX	+= animVelX*dt;
	animPosY	+= animVelY*dt;

	const float	epsilonPos	= 0.5f;
	const float	epsilonVel	= 2.0f;
	if ((fabs(targetX-animPosX) < epsilonPos) && (fabs(targetY-animPosY) < epsilonPos)
			&& (fabs(animVelX) < epsilonVel) && (fabs(animVelY) < epsilonVel)) {
		animating	= false;
		return false;
	}
	return true;
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
			editMessage->AddString("subgroup",P_C_NODE_DATA);
			editMessage->AddString("name",attribName);
			attribute->FindString(P_C_NODE_NAME,(const char **)&realName);
			BString		*testString 	= new BString(attribName);
			Renderer	*testRenderer	= new StringRenderer(editor,"",BRect(frame.left+2,frame.top+10,frame.right-2,frame.bottom-2), editMessage);
			attributes->insert(pair<BString *,Renderer*>(testString,testRenderer));
			break;
		}
	}*/
	BRect	attributeRect;
	if (attributes->empty())
	{
		BRect	content	= ContentFrame();
		attributeRect = BRect(content.left+circleSize+2,name->Frame().bottom+6,content.right-circleSize-2,content.bottom);
	}
	else
	{
		Renderer* lastRenderer = (*attributes)[attributes->size()-1];
		BRect	content	= ContentFrame();
		attributeRect = BRect(content.left+circleSize+2,lastRenderer->Frame().bottom,content.right-circleSize-2,content.bottom);
	}
	BMessage*	editMessage		= new BMessage(P_C_EXECUTE_COMMAND);
	editMessage->AddPointer("node",container);
	editMessage->AddString("Command::Name","ChangeValue");
	BMessage*	valueContainer	= new BMessage();
	valueContainer->AddString("subgroup",P_C_NODE_DATA);
	valueContainer->AddString("subgroup",attribName);
	valueContainer->AddInt32("subgroupindex",0);
	valueContainer->AddInt32("subgroupindex",count);
	editMessage->AddMessage("valueContainer",valueContainer);
	delete valueContainer;
	BMessage*	removeAttribMessage		= new BMessage(P_C_EXECUTE_COMMAND);
	removeAttribMessage->AddPointer("node",container);
	removeAttribMessage->AddString("Command::Name","RemoveAttribute");
	valueContainer	= new BMessage();
	valueContainer->AddString("subgroup",P_C_NODE_DATA);
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
			tmpParent->FindRect(P_C_NODE_FRAME,&parentRect);
			GroupRenderer	*parent	= NULL;
			if (tmpParent->FindPointer(editor->RenderString(), (void **)&parent) == B_OK) {
				if (parent->Frame() != parentRect) {
					BMessage	*changeValue		= new BMessage(P_C_EXECUTE_COMMAND);
					BMessage	*valueContainer		= new BMessage();
					changeValue->AddString("Command::Name","ChangeValue");
					changeValue->AddPointer("node",tmpParent);
					valueContainer->AddInt32("type",B_RECT_TYPE);
					valueContainer->AddString("name", P_C_NODE_FRAME );
					valueContainer->AddRect("newValue", parent->Frame());
					changeValue->AddMessage("valueContainer",valueContainer);
					command->AddMessage("PCommand::subPCommand",changeValue);
				}
			}
			tmpParent->FindPointer(P_C_NODE_PARENT, (void **)&tmpParent);
		}
	}
}
