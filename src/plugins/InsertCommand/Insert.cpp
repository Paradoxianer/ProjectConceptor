#include "ProjectConceptorDefs.h"
#include "Insert.h"

#include <support/TypeConstants.h>

Insert::Insert():PCommand() {
}

void Insert::Undo(PDocument *doc,BMessage *undo) {
	BList			*allConnectinos		= doc->GetAllConnections();
	BList			*allNodes			= doc->GetAllNodes();
	set<BMessage*>		*changed			= doc->GetChangedNodes();
	BMessage		*node				= new BMessage();
	int32			i					= 0;
	PCommand::Undo(doc,undo);
	// mirrors Do(): a node's parent lives on the node itself
	// (P_C_NODE_PARENT), not on the command wrapper - see there.
	while (undo->FindPointer("node",i,(void **)&node) == B_OK){
		if (node!=NULL) {
			if (node->what != P_C_CONNECTION_TYPE){
				allNodes->RemoveItem(node);
				BMessage	*parentNode			= NULL;
				BList		*parentAllNodes		= NULL;
				if ((node->FindPointer(P_C_NODE_PARENT, (void **)&parentNode) == B_OK) && (parentNode != NULL)) {
					if ((parentNode->FindPointer(P_C_NODE_ALLNODES, (void **)&parentAllNodes) == B_OK) && (parentAllNodes))
						parentAllNodes->RemoveItem(node);
				}
			}
			else
				allConnectinos->RemoveItem(node);
			changed->insert(node);
		}
		i++;
	}
	doc->SetModified();
}

BMessage* Insert::Do(PDocument *doc, BMessage *settings) {
	TRACE();
	BMessage		*node				= NULL;
	set<BMessage*>	*changed			= doc->GetChangedNodes();
	BList			*allConnections		= doc->GetAllConnections();
	BList			*allNodes			= doc->GetAllNodes();
	int32			i					= 0;
	status_t		err					= B_OK;
	while ((err=settings->FindPointer("node",i,(void **)&node)) == B_OK) {
		if (node->what != P_C_CONNECTION_TYPE) {
			allNodes->AddItem(node);
			// a node's intended parent (e.g. set by GroupRenderer when
			// double-clicking a group to insert a child) lives on the node
			// itself, not on this command's wrapper message - each inserted
			// node can have its own parent, so this has to be looked up per
			// node, not once for the whole batch (was issue #36: the old
			// lookup on `settings` never matched anything a caller actually
			// set, so new children silently ended up as top-level siblings
			// instead of being registered in their parent's node list)
			BMessage	*parentNode			= NULL;
			BList		*parentAllNodes		= NULL;
			if ((node->FindPointer(P_C_NODE_PARENT, (void **)&parentNode) == B_OK) && (parentNode != NULL)) {
				if (parentNode->FindPointer(P_C_NODE_ALLNODES, (void **)&parentAllNodes) != B_OK) {
					parentAllNodes = new BList();
					parentNode->AddPointer(P_C_NODE_ALLNODES, parentAllNodes);
				}
				parentAllNodes->AddItem(node);
			}
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
