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
	if (container->FindPointer("Node::allNodes", (void **)&allNodes) !=B_OK)
		container->AddPointer("Node::allNodes",allNodes=new BList());
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
	set<BMessage*>	*changedNodes	= doc->GetChangedNodes();
	BList			*allDocNodes	= doc->GetAllNodes();

	set<BMessage*>::iterator it;
	BMessage	*node			= NULL;
	Renderer	*painter		= NULL;

	ClassRenderer::ValueChanged();
	for ( it=changedNodes->begin();it!=changedNodes->end();it++) {
		node	= *it;
		painter	= FindRenderer(node);
		if (painter != NULL) {
			if (allNodes->HasItem(node))
				painter->ValueChanged();
			else
				RemoveRenderer(painter);
		}
		else {
			if (allNodes->HasItem(node) == true)
				if (allDocNodes->HasItem(node) == true )
					InsertRenderObject(node);
				else
					allNodes->RemoveItem(node);
			else
				RemoveRenderer(painter);
		}
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
}


void GroupRenderer::AddRenderer(Renderer* newRenderer) {
	TRACE();
	renderer->AddItem(newRenderer);
}

void GroupRenderer::RemoveRenderer(Renderer *wichRenderer) {
	TRACE();
	renderer->RemoveItem(wichRenderer);
	delete wichRenderer;
	wichRenderer=NULL;
}


Renderer* GroupRenderer::FindRenderer(BMessage *container) {
	int32		i					= 0;
	Renderer	*currentRenderer	= NULL;
	if ( (container->FindPointer(editor->RenderString(),(void **) &currentRenderer) == B_OK) 
		&& (currentRenderer) && renderer->HasItem(currentRenderer) )
		return currentRenderer;
	else
		return NULL;

/*	bool		found				= false;
	int32		count				= renderer->CountItems();
	while ((i<count) && (!found)) {
		currentRenderer= (Renderer*)renderer->ItemAt(i);
		if (currentRenderer->GetMessage() == container)
			found=true;
		i++;
	}po
	if (found)
		return currentRenderer;
	else
		return NULL;*/
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
	groupFrame.InsetBy(-5,-5);
	groupFrame.top = groupFrame.top-15;
	if (groupFrame != frame) {
		frame =  frame | groupFrame;
		//** need to move the Attribs and the Name...
		if (parentNode) {
			GroupRenderer	*parent	= NULL;
			if (parentNode->FindPointer(editor->RenderString(), (void **)&parent) == B_OK)
				parent->RecalcFrame();
		}
	}
}
