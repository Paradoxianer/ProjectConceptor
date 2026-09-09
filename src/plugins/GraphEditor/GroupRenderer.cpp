#include "GroupRenderer.h"
#include "GroupBoundary.h"
#include "ProjectConceptorDefs.h"

#include <math.h>
#include <algorithm>
#include <set>

#include <interface/Font.h>
#include <interface/View.h>
#include <interface/GraphicsDefs.h>

#include <interface/Window.h>

#include <support/String.h>
#include "AttributRenderer.h"
#include "PDocument.h"




// Every corner in ComputeGroupBoundary()'s result is a right angle, either
// convex (like a node's own corner) or concave (a notch cut into the shape
// where a shorter child leaves empty space next to a taller one) - the
// nodes themselves use rounded corners, so this shape should too, notches
// included (issue #38). Rather than pull in BShape's SVG-style ArcTo (whose
// sweep-direction flags aren't obvious to get right for a mix of convex and
// concave turns), each vertex is replaced by a handful of points along the
// actual tangent circle: trim `radius` back along both edges meeting at the
// corner to get the arc's endpoints, its center is where those two trimmed
// edges' perpendiculars meet (same construction for either turn direction -
// only the resulting curve's concavity differs), then step along the
// circle between the two endpoints.
static vector<BPoint> RoundCorners(const vector<BPoint> &points, float radius)
{
	vector<BPoint>	result;
	uint32	n	= points.size();
	if ((n < 3) || (radius <= 0))
		return points;

	for (uint32 i=0; i<n; i++) {
		BPoint	prev	= points[(i+n-1)%n];
		BPoint	corner	= points[i];
		BPoint	next	= points[(i+1)%n];
		BPoint	dirIn(corner.x-prev.x,corner.y-prev.y);
		BPoint	dirOut(next.x-corner.x,next.y-corner.y);
		float	lenIn	= sqrt(dirIn.x*dirIn.x+dirIn.y*dirIn.y);
		float	lenOut	= sqrt(dirOut.x*dirOut.x+dirOut.y*dirOut.y);
		float	r		= radius;
		if (lenIn > 0)  r = min(r,lenIn/2);
		if (lenOut > 0) r = min(r,lenOut/2);
		if ((lenIn < 0.01) || (lenOut < 0.01) || (r < 0.5)) {
			result.push_back(corner);
			continue;
		}
		BPoint	unitIn(dirIn.x/lenIn,dirIn.y/lenIn);
		BPoint	unitOut(dirOut.x/lenOut,dirOut.y/lenOut);
		BPoint	entry(corner.x-unitIn.x*r,corner.y-unitIn.y*r);
		BPoint	exit(corner.x+unitOut.x*r,corner.y+unitOut.y*r);
		// dirIn/dirOut are axis-aligned and perpendicular - the arc's
		// center takes its x from whichever of entry/exit sits on the
		// vertical edge, and its y from whichever sits on the horizontal
		// one (the corner of the two edges' own R-offset parallels).
		BPoint	center	= (fabs(unitIn.x) > 0.5)
			? BPoint(entry.x,exit.y) : BPoint(exit.x,entry.y);
		float	startAngle	= atan2(entry.y-center.y,entry.x-center.x);
		float	endAngle	= atan2(exit.y-center.y,exit.x-center.x);
		float	delta		= endAngle-startAngle;
		while (delta > M_PI)  delta -= 2*M_PI;
		while (delta < -M_PI) delta += 2*M_PI;
		const int32	steps	= 6;
		for (int32 s=0; s<=steps; s++) {
			float	a	= startAngle+delta*s/steps;
			result.push_back(BPoint(center.x+r*cos(a),center.y+r*sin(a)));
		}
	}
	return result;
}


GroupRenderer::GroupRenderer(GraphEditor *parentEditor, BMessage *forContainer):ClassRenderer(parentEditor, forContainer)
{
	TRACE();
	Init();
	ValueChanged();
}
void GroupRenderer::Init()
{
	TRACE();
	ClassRenderer::Init();
	scale							= 1.0;
	renderer						= new BList();
	father							= NULL;
	usesDefaultFill					= false;
	if (container->FindPointer(P_C_NODE_ALLNODES, (void **)&allNodes) !=B_OK)
		container->AddPointer(P_C_NODE_ALLNODES,allNodes=new BList());
}

void GroupRenderer::BringToFront(Renderer *wichRenderer)
{
	renderer->RemoveItem(wichRenderer);
	renderer->AddItem(wichRenderer);
}

void GroupRenderer::SendToBack(Renderer *wichRenderer)
{
	renderer->RemoveItem(wichRenderer);
	renderer->AddItem(wichRenderer,0);
}

void GroupRenderer::ValueChanged()
{
	TRACE();
	// Used to walk the *entire* session-wide changedNodes set on every call
	// (issue #87) - for a batch covering the whole document (e.g. right
	// after load, or at construction: the constructor calls ValueChanged()
	// immediately) every group re-scanned every other group's children too,
	// O(group count x total changed nodes). allNodes/renderer here are this
	// group's own child list/renderer bookkeeping - both bounded by this
	// group's own size, never by document size - so walk those instead and
	// use changedNodes only for the O(log n) membership check a std::set
	// gives for free.
	set<BMessage*>	*changedNodes	= doc->GetChangedNodes();
	BList			*allDocNodes	= doc->GetAllNodes();
	BMessage		*node			= NULL;
	Renderer		*painter		= NULL;

	ClassRenderer::ValueChanged();

	// A group defaults to just a hint of fill (issue #38) - see the note
	// in Draw(). ClassRenderer just read this group's fill from its
	// pattern; one still carrying the editor's standard fill has never
	// been given a colour of its own (every group ever saved carries that
	// standard pattern, since ClassRenderer adds it on first use), so
	// treat it as unset. A colour actually picked for this group differs
	// and fills solidly.
	rgb_color	standardFill;
	usesDefaultFill	= false;
	if (editor->GetStandartPattern()->FindInt32("FillColor",(int32 *)&standardFill) == B_OK) {
		if ((fillColor.red == standardFill.red)
				&& (fillColor.green == standardFill.green)
				&& (fillColor.blue == standardFill.blue)) {
			fillColor.alpha	= 55;
			usesDefaultFill	= true;
		}
	}

	// ClassRenderer::ValueChanged() just read P_C_NODE_FRAME as-is - if this
	// broadcast came from the generic Resize command (dragging the group's
	// own resize handle), that command has no idea this node is a group and
	// commits whatever the user dragged to, with only a bare minimum-size
	// check, no children-bounds check at all. RecalcFrame() unions that
	// against the children's actual bounds, so a manual shrink below what
	// the children need immediately snaps back to fit instead of leaving
	// them stranded outside a too-small box (matches what already happens
	// live during an in-progress drag via ResizeBy(), just also covering
	// the final committed value).
	RecalcFrame(true);

	// Pass 1: renderers this group already built. One whose underlying node
	// no longer appears in our own allNodes has left the group (ungrouped,
	// moved elsewhere, ...) and its renderer needs to go. Iterated
	// backwards since RemoveRenderer() shrinks this same list.
	for (int32 i = renderer->CountItems()-1; i >= 0; i--) {
		painter	= (Renderer *)renderer->ItemAt(i);
		node	= painter->GetMessage();
		if (!allNodes->HasItem(node))
			RemoveRenderer(painter);
	}

	// Pass 2: our current children. Only ones this broadcast actually
	// touched need anything - refresh an existing renderer, build a
	// missing one, or drop a child that's been deleted from the document
	// entirely (allNodes hasn't caught up to that yet).
	for (int32 i = allNodes->CountItems()-1; i >= 0; i--) {
		node = (BMessage *)allNodes->ItemAt(i);
		if (changedNodes->find(node) == changedNodes->end())
			continue;
		painter = FindRenderer(node);
		if (painter != NULL)
			painter->ValueChanged();
		else if (allDocNodes->HasItem(node))
			InsertRenderObject(node);
		else
			allNodes->RemoveItem(node);
	}
	// after the children are up to date - the notch this sits in is
	// derived from their rects
	PlaceLabel();
}

void GroupRenderer::MoveBy(float dx,float dy) {
	ClassRenderer::MoveBy(dx,dy);
	for (int32 i=0;i<renderer->CountItems();i++)
		((Renderer *)renderer->ItemAt(i))->MoveBy(dx,dy);
}

void GroupRenderer::ResizeBy(float dx,float dy) {
	ClassRenderer::ResizeBy(dx,dy);
	RecalcFrame(true);
}


void GroupRenderer::InsertRenderObject(BMessage *node) {
	TRACE();
	Renderer	*newRenderer = NULL;
	void		*parentPointer = NULL;	
	void		*tmpDoc	= NULL;
	if (node->FindPointer("ProjectConceptor::doc",&tmpDoc)==B_OK)
		node->ReplacePointer("ProjectConceptor::doc",doc);
	else
		node->AddPointer("ProjectConceptor::doc",doc);
	//find the pointer to the renderobject because the node was somehow added to the Grapheditor and has therefore already a renderobject
	if (node->FindPointer(editor->RenderString(),(void **)&newRenderer)== B_OK)
		AddRenderer(newRenderer);
	else
		AddRenderer(editor->CreateRendererFor(node));
	// a child just joined this group for the first time (new insert, or an
	// existing node grouped in) - grow the box to include it now instead of
	// leaving it to whatever happens to touch this group's frame next
	RecalcFrame(true);
}


void GroupRenderer::AddRenderer(Renderer* newRenderer) {
	TRACE();
	// see the same guard in GraphEditor::AddRenderer() - RemoveRenderer()
	// only drops the first matching entry, so a duplicate here would leave
	// a second, stale reference in this group's own bookkeeping list
	if (!renderer->HasItem(newRenderer))
		renderer->AddItem(newRenderer);
}

void GroupRenderer::RemoveRenderer(Renderer *wichRenderer) {
	TRACE();
	// bookkeeping only - GraphEditor::RemoveRenderer() is the single place
	// that ever deletes a renderer (see the comment there). This just drops
	// it from this group's own child list, e.g. because it fell out of the
	// group's P_C_NODE_ALLNODES (GroupRenderer::ValueChanged()) or is being
	// removed via GraphEditor::RemoveRenderer()'s delegation - either way
	// the object may still be alive and owned elsewhere.
	renderer->RemoveItem(wichRenderer);
}


Renderer* GroupRenderer::FindRenderer(BMessage *container) {
	Renderer	*currentRenderer	= NULL;
	if ( (container->FindPointer(editor->RenderString(),(void **) &currentRenderer) == B_OK) 
		&& (currentRenderer) && renderer->HasItem(currentRenderer) )
		return currentRenderer;
	else
		return NULL;
}


void GroupRenderer::RecalcFrame(bool toFit) {
	
	Renderer*	tmpRenderer		= NULL;
	BRect			groupFrame			= BRect(0,0,-1,-1);
	for (int32 i=0;(i<renderer->CountItems());i++) {
		tmpRenderer = (Renderer *) renderer->ItemAt(i);
		if ( (tmpRenderer->GetMessage()->what == P_C_CLASS_TYPE) || (tmpRenderer->GetMessage()->what == P_C_GROUP_TYPE) ) {
			if (!groupFrame.IsValid())
				groupFrame = tmpRenderer->Frame();
			else
				groupFrame = groupFrame | tmpRenderer->Frame();
		}
	}
	// no children registered in this group's own bookkeeping list yet (e.g.
	// called from ValueChanged() before any child has been processed, or
	// right at construction) - groupFrame is still the invalid (0,0)-(-1,-1)
	// default here, and unioning that into `frame` below would corrupt it
	// into a huge, (0,0)-anchored rect that paints over the rest of the
	// canvas. Nothing to fit yet, so leave the existing frame alone.
	if (!groupFrame.IsValid())
		return;
	// same margins Draw()/CollectChildRects() use, so `frame` actually
	// contains the drawn outline instead of clipping it - the top used to
	// reserve a flat 15px for the label while Draw() reserves however much
	// the name and attribute rows really need.
	groupFrame.top		-= 5;
	groupFrame.left		-= 5;
	groupFrame.bottom	+= 8;
	groupFrame.right	+= 8;
	groupFrame.top		-= LabelSpace();
	if (groupFrame != frame) {
		// exact assignment, not a union with the old frame (issue #38) -
		// a group is a strict auto-fit rectangle around its children, so
		// it has to shrink back down just as readily as it grows. No
		// manual resize handle exists anymore (SupportsResize() is false
		// here) to fight this; a stray committed resize from some other
		// path (an old macro replay, say) gets corrected back to fit the
		// next time this runs, same as an oversized one would.
		frame = groupFrame;
		// without this, the next ValueChanged() on this renderer (any later
		// change anywhere - changedNodes never clears - will trigger one)
		// re-reads P_C_NODE_FRAME from container via ClassRenderer's own
		// ValueChanged() and overwrites this recalculation right back to
		// its old, too-small value
		container->ReplaceRect(P_C_NODE_FRAME,frame);
		// the notch the label sits in moved with the children
		PlaceLabel();
		if (parentNode) {
			GroupRenderer	*parent	= NULL;
			if (parentNode->FindPointer(editor->RenderString(), (void **)&parent) == B_OK)
				parent->RecalcFrame();
		}
	}
}


void GroupRenderer::MouseDown(BPoint where, int32 buttons,
	                              int32 clicks,int32 modifiers){
	if (clicks == 2)
	{	
		//insert a new Subnode	
		TRACE();
		BMessage *newNodeCommand=editor->GenerateInsertCommand(P_C_CLASS_TYPE);
		BMessage *node;
		newNodeCommand->FindPointer("node",(void **)&node);
		node->AddPointer(P_C_NODE_PARENT,container);
		editor->SendMessageToDoc(newNodeCommand);
	}
	ClassRenderer::MouseDown(where,buttons,clicks,modifiers);
}


// Each child's own rect plus margin - the shape the boundary is built
// around. More margin at the bottom/right than top/left: that is where a
// child's own drop shadow lands, so it needs the extra room.
void GroupRenderer::CollectChildRects(vector<BRect> &rects)
{
	for (int32 i=0; i<renderer->CountItems(); i++) {
		Renderer	*child	= (Renderer *)renderer->ItemAt(i);
		if ((child->GetMessage()->what == P_C_CLASS_TYPE)
				|| (child->GetMessage()->what == P_C_GROUP_TYPE)) {
			BRect	r	= child->Frame();
			r.top		-= 5;
			r.left		-= 5;
			r.bottom	+= 8;
			r.right		+= 8;
			rects.push_back(r);
		}
	}
}


// Height the outline has to keep clear above the leftmost child for this
// group's own name and attribute rows - measured off what they actually
// occupy, not a guessed constant.
float GroupRenderer::LabelSpace(void)
{
	float	space	= name->Frame().Height()+4;
	vector<Renderer *>::iterator	attribute	= attributes->begin();
	while (attribute != attributes->end()) {
		space	+= (*attribute)->Frame().Height();
		attribute++;
	}
	return space;
}


// ClassRenderer places the name (and, since ClassRenderer::InsertAttribute()
// is inherited unchanged, every attribute row) relative to `frame`, which
// for a group is the bounding box of every child - so both the vertical
// position and the *width* came out wrong: the label ended up at the top
// of the *topmost* child instead of the notch above the *leftmost* one,
// and every row stretched across the whole group instead of just that
// notch. Move each row into the notch and resize it to the notch's own
// width, keeping its height (a row's own text metrics, untouched here).
void GroupRenderer::PlaceLabel(void)
{
	vector<BRect>	rects;
	CollectChildRects(rects);
	if (rects.empty())
		return;

	BRect	leftmost	= rects[0];
	for (uint32 i=1; i<rects.size(); i++) {
		if (rects[i].left < leftmost.left)
			leftmost	= rects[i];
	}

	float	targetLeft	= leftmost.left+(xRadius/3);
	float	targetRight	= leftmost.right-(xRadius/3);
	BRect	current		= name->Frame();
	float	dy			= (leftmost.top-LabelSpace()+(yRadius/3)) - current.top;
	if ((current.left == targetLeft) && (current.right == targetRight) && (dy == 0))
		return;
	name->SetFrame(BRect(targetLeft,current.top+dy,targetRight,current.bottom+dy));
	vector<Renderer *>::iterator	attribute	= attributes->begin();
	while (attribute != attributes->end()) {
		BRect	row	= (*attribute)->Frame();
		(*attribute)->SetFrame(BRect(targetLeft,row.top+dy,targetRight,row.bottom+dy));
		attribute++;
	}
}


void GroupRenderer::Draw(BView *drawOn, BRect updateRect)
{
	bool	offsetForAnim	= animating;
	BPoint	priorOrigin		= drawOn->Origin();
	if (offsetForAnim) {
		BPoint	delta(animPosX-frame.left,animPosY-frame.top);
		drawOn->PushState();
		drawOn->SetOrigin(priorOrigin+delta);
	}

	drawOn->SetFont(font);
	drawOn->SetPenSize(penSize);

	vector<BRect>	rects;
	CollectChildRects(rects);
	if (rects.empty()) {
		if (offsetForAnim)
			drawOn->PopState();
		return;
	}

	vector<BPoint>	hull	= ComputeGroupBoundary(rects,LabelSpace());
	if (hull.size() < 3) {
		if (offsetForAnim)
			drawOn->PopState();
		return;
	}
	hull	= RoundCorners(hull,xRadius);

	rgb_color	drawColor	= hasPreviewFillColor ? previewFillColor : fillColor;
	// A group's fill lies behind every child across the whole enclosed
	// area, so a solid one tints all of it and reads far heavier than the
	// outline needs (issue #38) - by default it stays a faint tint. The
	// drop shadow only makes sense under a fill solid enough to cast one;
	// under the default tint it would be darker than the shape itself.
	// Both go solid as soon as the group is given a real fill colour.
	bool	filled	= (drawColor.alpha != 0);
	bool	shadowed	= filled && (hasPreviewFillColor || !usesDefaultFill);

	if (shadowed) {
		vector<BPoint>	shadowHull(hull);
		for (uint32 i=0; i<shadowHull.size(); i++)
			shadowHull[i]	+= BPoint(3,3);
		drawOn->SetHighColor(0,0,0,77);
		drawOn->FillPolygon(&shadowHull[0],shadowHull.size());
	}

	if (selected) {
		drawOn->SetPenSize(5.0);
		drawOn->SetHighColor(200,0,0,150);
		drawOn->StrokePolygon(&hull[0],hull.size());
		drawOn->SetPenSize(penSize);
	}

	if (filled) {
		drawOn->SetHighColor(drawColor);
		drawOn->FillPolygon(&hull[0],hull.size());
	}

	drawOn->SetHighColor(borderColor);
	drawOn->StrokePolygon(&hull[0],hull.size());

	name->Draw(drawOn,updateRect);
	vector<Renderer *>::iterator	allAttributes	= attributes->begin();
	while (allAttributes != attributes->end()) {
		(*allAttributes)->Draw(drawOn,updateRect);
		allAttributes++;
	}

	if (offsetForAnim)
		drawOn->PopState();
}
