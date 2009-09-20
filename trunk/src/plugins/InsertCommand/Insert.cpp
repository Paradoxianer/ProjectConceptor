#include "ProjectConceptorDefs.h"
#include "Insert.h"


Insert::Insert():PCommand() {
}

void Insert::Undo(PDocument *doc,BMessage *undo) {
	BMessage		*parentNode			= NULL;
	BList			*parentAllNodes		= NULL;
	BList			*allConnectinos		= doc->GetAllConnections();
	BList			*allNodes			= doc->GetAllNodes();
	BList			*changed			= doc->GetChangedNodes();
	BMessage		*node				= new BMessage();
	BMessage		*connection			= new BMessage();
	int32			i					= 0;
	PCommand::Undo(doc,undo);
	undo->FindPointer("Node::parent", (void **)&parentNode);
	if (parentNode)
		parentNode->FindPointer("Node::allNodes", (void **)&parentAllNodes);
	while (undo->FindPointer("node",i,(void **)&node) == B_OK) {
		if (node->what != P_C_CONNECTION_TYPE){
			allNodes->RemoveItem(node);
			if (parentAllNodes)
				parentAllNodes->RemoveItem(node);			
		}
		else
			allConnectinos->RemoveItem(node);
		changed->AddItem(node);
		i++;
	}
	i=0;

	doc->SetModified();
}

BMessage* Insert::Do(PDocument *doc, BMessage *settings) {
	TRACE();
	BMessage		*node				= NULL;
	BMessage		*parentNode			= NULL;
	BList			*parentAllNodes		= NULL;
	BList			*changed			= doc->GetChangedNodes();
	BList			*allConnections		= doc->GetAllConnections();
	BList			*allNodes			= doc->GetAllNodes();
	int32			i					= 0;
	status_t		err					= B_OK;
	err = settings->FindPointer("Node::parent", (void **)&parentNode);
	if ((err==B_OK) && (parentNode != NULL)) {
		if (parentNode->FindPointer("Node::allNodes", (void **)&parentAllNodes) != B_OK)
			parentNode->AddPointer("Node::allNodes", new BList());
	}
	while (settings->FindPointer("node",i,(void **)&node) == B_OK) {
		if (node->what != P_C_CONNECTION_TYPE) {
			allNodes->AddItem(node);
			if (parentAllNodes)
				parentAllNodes->AddItem(node);
		}
		else
			allConnections->AddItem(node);
		//recalc size
		//**check if there is a passed "docRect"
		BRect	insertFrame		= BRect(0,0,0,0);
		if (node->FindRect("Node::frame",&insertFrame)==B_OK) {
			BRect	docRect			= doc->Bounds();
			if (insertFrame.bottom >= docRect.Height())
				docRect.bottom= insertFrame.bottom+20;
			if (insertFrame.right >= docRect.Width())
				docRect.right = insertFrame.right+20;
			if (docRect != doc->Bounds())
				doc->Resize(docRect.right,docRect.bottom);
		}
		i++;
		if (!changed->HasItem(node))
			changed->AddItem(node);
	}
	doc->SetModified();
	settings = PCommand::Do(doc,settings);
	return settings;
}



void Insert::AttachedToManager(void) {
}

void Insert::DetachedFromManager(void) {
}
