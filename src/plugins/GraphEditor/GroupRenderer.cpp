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
