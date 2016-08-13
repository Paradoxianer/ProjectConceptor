#include "ProjectConceptorDefs.h"
#include "Insert.h"

#include <support/TypeConstants.h>

Insert::Insert():PCommand() {
}

void Insert::Undo(PDocument *doc,BMessage *undo) {
	BMessage		*parentNode			= NULL;
	BList			*parentAllNodes		= NULL;
	BList			*allConnectinos		= doc->GetAllConnections();
	BList			*allNodes			= doc->GetAllNodes();
	set<BMessage*>		*changed			= doc->GetChangedNodes();
	BMessage		*node				= new BMessage();
	BMessage		*connection			= new BMessage();
	int32			i					= 0;
	status_t		err					= B_OK;
	PCommand::Undo(doc,undo);
	err = undo->FindPointer(P_C_NODE_PARENT, (void **)&parentNode);
	if (parentNode)
		err = parentNode->FindPointer(P_C_NODE_ALLNODES, (void **)&parentAllNodes);
	while (undo->FindPointer("node",i,(void **)&node) == B_OK){
		if (node!=NULL) {
			if (node->what != P_C_CONNECTION_TYPE){
				allNodes->RemoveItem(node);
				if (parentAllNodes)
					parentAllNodes->RemoveItem(node);			
			}
			else
				allConnectinos->RemoveItem(node);
			changed->insert(node);
		}
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
	set<BMessage*>	*changed			= doc->GetChangedNodes();
	BList			*allConnections		= doc->GetAllConnections();
	BList			*allNodes			= doc->GetAllNodes();
	int32			i					= 0;
	status_t		err					= B_OK;
	err = settings->FindPointer(P_C_NODE_PARENT, (void **)&parentNode);
	if ((err==B_OK) && (parentNode != NULL)) {
		if (parentNode->FindPointer(P_C_NODE_ALLNODES, (void **)&parentAllNodes) != B_OK)
			parentNode->AddPointer(P_C_NODE_ALLNODES, new BList());
	}
	while ((err=settings->FindPointer("node",i,(void **)&node)) == B_OK) {
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
		if (node->FindRect(P_C_NODE_FRAME,&insertFrame)==B_OK) {
			BRect	docRect			= doc->Bounds();
			if (insertFrame.bottom >= docRect.Height())
				docRect.bottom= insertFrame.bottom+20;
			if (insertFrame.right >= docRect.Width())
				docRect.right = insertFrame.right+20;
			if (docRect != doc->Bounds())
				doc->Resize(docRect.right,docRect.bottom);
		}
		i++;
		changed->insert(node);
	}
	doc->SetModified();
	settings = PCommand::Do(doc,settings);
	return settings;
}



void Insert::AttachedToManager(void) {
}

void Insert::DetachedFromManager(void) {
}
