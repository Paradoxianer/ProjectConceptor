#include "Move.h"


Move::Move():PCommand()
{
}

void Move::Undo(PDocument *doc,BMessage *undo) {
	BMessage		*undoMessage		= new BMessage();
	set<BMessage*>		*changed			= doc->GetChangedNodes();
	BRect			*oldFrame			= new BRect(0,0,100,100);
	BMessage		*node				= NULL;
	BMessage		*settings			= new BMessage();
	int32			i					= 0;
	PCommand::Undo(doc,undoMessage);
	undo->FindMessage("Move::Undo" ,undoMessage);
	//settings is placed by the default implentation of PCommand::Do(...) into the undo MEssage
	while (undoMessage->FindPointer("node",i,(void **)&node) == B_OK) {
		if (undoMessage->FindRect("oldFrame",i,oldFrame) == B_OK) {
			node->ReplaceRect("Node::frame",*oldFrame);
			changed->insert(node);
		}
		i++;
	}
	doc->SetModified();
}

BMessage* Move::Do(PDocument *doc, BMessage *settings) {
	
	BMessage	*undoMessage	= new BMessage();
	BList		*selected		= doc->GetSelected();
	set<BMessage*>	*changed		= doc->GetChangedNodes();
	float		dx,dy;
	BMessage	*node			= new BMessage();
	int32			i					= 0;
	if ( (settings->FindFloat("dx",&dx)==B_OK) && (settings->FindFloat("dy",&dy)==B_OK) )	{
		for (i=0;i<selected->CountItems();i++) {
			node=(BMessage *)selected->ItemAt(i);
			MoveNode(doc, changed,node, dx, dy, undoMessage);
		}
	}
	doc->SetModified();
	settings->RemoveName("Move::Undo");
	settings->AddMessage("Move::Undo", undoMessage);
	settings = PCommand::Do(doc,settings);
	return settings;
}



void Move::AttachedToManager(void) {
}

void Move::DetachedFromManager(void) {
}

void Move::MoveNode(PDocument *doc, set<BMessage*> *changed, BMessage *node, float dx, float dy, BMessage *undoMessage) {
	BList	*allNodes	= NULL;
	int32	i			= 0;
	BRect	*newFrame	= new BRect(0,0,-1,-1);
	BRect	*oldFrame	= new BRect(0,0,-1,-1);
	node->FindRect("Node::frame",oldFrame);
	undoMessage->AddRect("oldFrame",*oldFrame);
	*newFrame=*oldFrame;
	newFrame->OffsetBy(dx,dy);
	node->ReplaceRect("Node::frame",*newFrame);
	undoMessage->AddPointer("node",node);
	changed->insert(node);
	BRect	docRect			= doc->Bounds();
	if (newFrame->bottom >= docRect.Height())
		docRect.bottom= newFrame->bottom+20;
	if (newFrame->right >= docRect.Width())
		docRect.right = newFrame->right+20;
	if (docRect != doc->Bounds())
		doc->Resize(docRect.right,docRect.bottom);
	if ( (node->FindPointer("Node::allNodes",(void **)&allNodes)==B_OK) && (allNodes) ) {
		for (i=0;i<allNodes->CountItems();i++) {
			MoveNode(doc,changed,(BMessage *)allNodes->ItemAt(i), dx, dy,undoMessage);
		}
	}
}

void Move::ResizeParent(PDocument *doc, set<BMessage*> *changed, BMessage *node, float dx, float dy,BMessage *undoMessage) {

}
