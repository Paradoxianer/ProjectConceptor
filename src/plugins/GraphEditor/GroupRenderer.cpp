#include "GroupRenderer.h"
#include "ProjectConceptorDefs.h"

#include <math.h>
#include <algorithm>

#include <interface/Font.h>
#include <interface/View.h>
#include <interface/GraphicsDefs.h>

#include <interface/Window.h>

#include <support/String.h>
#include "AttributRenderer.h"
#include "PDocument.h"


static bool PointLess(const BPoint &a, const BPoint &b)
{
	return (a.x < b.x) || ((a.x == b.x) && (a.y < b.y));
}

static float Cross(const BPoint &o, const BPoint &a, const BPoint &b)
{
	return (a.x-o.x)*(b.y-o.y) - (a.y-o.y)*(b.x-o.x);
}

// Andrew's monotone chain - the group is meant to look like a string pulled
// taut around all its children (issue #38), not a union of separate boxes
// with gaps where children happen to be far apart. Standard O(n log n)
// convex hull: sort by (x,y), then build the lower and upper chains,
// popping the last point whenever the last three make a non-left turn.
static vector<BPoint> ConvexHull(vector<BPoint> points)
{
	int32	n	= points.size();
	if (n < 3)
		return points;
	sort(points.begin(),points.end(),PointLess);

	vector<BPoint>	hull(2*n);
	int32	k	= 0;
	for (int32 i=0; i<n; i++) {
		while ((k >= 2) && (Cross(hull[k-2],hull[k-1],points[i]) <= 0))
			k--;
		hull[k++]	= points[i];
	}
	for (int32 i=n-2, lower=k+1; i>=0; i--) {
		while ((k >= lower) && (Cross(hull[k-2],hull[k-1],points[i]) <= 0))
			k--;
		hull[k++]	= points[i];
	}
	hull.resize(k-1);
	return hull;
}


// A convex hull's own edges are generally diagonal wherever the point set's
// boundary isn't already axis-aligned - "a string pulled taut" but with only
// right-angle turns (issue #38) means each diagonal edge (a,b) needs an
// axis-aligned dogleg corner inserted. Both candidate corners - (a.x,b.y)
// and (b.x,a.y) - complete the same right triangle with the diagonal; the
// one farther from the hull's own centroid is the one that stays outside
// the hull (bulges further out) rather than cutting into it.
static vector<BPoint> OrthogonalizeHull(const vector<BPoint> &hull)
{
	if (hull.size() < 3)
		return hull;

	float	cx	= 0, cy	= 0;
	for (uint32 i=0; i<hull.size(); i++) {
		cx	+= hull[i].x;
		cy	+= hull[i].y;
	}
	cx	/= hull.size();
	cy	/= hull.size();

	vector<BPoint>	result;
	uint32	n	= hull.size();
	for (uint32 i=0; i<n; i++) {
		BPoint	a	= hull[i];
		BPoint	b	= hull[(i+1)%n];
		result.push_back(a);
		if ((a.x != b.x) && (a.y != b.y)) {
			BPoint	corner1(a.x,b.y);
			BPoint	corner2(b.x,a.y);
			float	d1	= (corner1.x-cx)*(corner1.x-cx)+(corner1.y-cy)*(corner1.y-cy);
			float	d2	= (corner2.x-cx)*(corner2.x-cx)+(corner2.y-cy)*(corner2.y-cy);
			result.push_back((d1 > d2) ? corner1 : corner2);
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
	groupFrame.InsetBy(-5,-5);
	groupFrame.top = groupFrame.top-15;
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
		//** need to move the Attribs and the Name...
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

	// one connected shape wrapped tightly around every child (like a
	// string pulled taut around them - issue #38), not separate boxes
	// with gaps where children happen to be spread far apart - so this
	// collects every child's own 4 corners (+5px margin, same as
	// RecalcFrame()'s inset) as hull candidate points rather than
	// unioning rects.
	vector<BPoint>	points;
	for (int32 i=0; i<renderer->CountItems(); i++) {
		Renderer	*child	= (Renderer *)renderer->ItemAt(i);
		if ((child->GetMessage()->what == P_C_CLASS_TYPE)
				|| (child->GetMessage()->what == P_C_GROUP_TYPE)) {
			BRect	r	= child->Frame();
			r.InsetBy(-5,-5);
			points.push_back(r.LeftTop());
			points.push_back(r.RightTop());
			points.push_back(r.LeftBottom());
			points.push_back(r.RightBottom());
		}
	}
	if (points.empty()) {
		if (offsetForAnim)
			drawOn->PopState();
		return;
	}
	// label space along the top - same purpose as RecalcFrame()'s "-15 on
	// top", added as two extra hull candidates rather than a separate
	// strip so the whole shape stays one convex polygon.
	float	minX	= points[0].x, maxX = points[0].x, minY = points[0].y;
	for (uint32 i=1; i<points.size(); i++) {
		if (points[i].x < minX) minX = points[i].x;
		if (points[i].x > maxX) maxX = points[i].x;
		if (points[i].y < minY) minY = points[i].y;
	}
	points.push_back(BPoint(minX,minY-15));
	points.push_back(BPoint(maxX,minY-15));

	vector<BPoint>	hull	= OrthogonalizeHull(ConvexHull(points));
	if (hull.size() < 3) {
		if (offsetForAnim)
			drawOn->PopState();
		return;
	}

	rgb_color	drawColor	= hasPreviewFillColor ? previewFillColor : fillColor;

	vector<BPoint>	shadowHull(hull);
	for (uint32 i=0; i<shadowHull.size(); i++)
		shadowHull[i]	+= BPoint(3,3);
	drawOn->SetHighColor(0,0,0,77);
	drawOn->FillPolygon(&shadowHull[0],shadowHull.size());

	if (selected) {
		drawOn->SetPenSize(5.0);
		drawOn->SetHighColor(200,0,0,150);
		drawOn->StrokePolygon(&hull[0],hull.size());
		drawOn->SetPenSize(penSize);
	}

	drawOn->SetHighColor(drawColor);
	drawOn->FillPolygon(&hull[0],hull.size());

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
