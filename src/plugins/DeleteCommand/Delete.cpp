#include "Delete.h"
#include "ProjectConceptorDefs.h"

Delete::Delete():PCommand() {
}

void Delete::Undo(PDocument *doc,BMessage *undo) {
	BMessage		*undoMessage		= new BMessage();
	BMessage		*settings			= new BMessage();
	BMessage		*node				= NULL;
	BMessage		*connection			= NULL;
	BList			*parentGroupList	= NULL;
	set<BMessage*>		*changed			= doc->GetChangedNodes();
	BList			*allNodes			= doc->GetAllNodes();
	BList			*allConnections		= doc->GetAllConnections();
	BMessage		*commandMessage		= new BMessage();
	int32			i					= 0;
	PCommand::Undo(doc,undo);
	undo->FindMessage("Delete::Undo" ,undoMessage);
	while (undoMessage->FindPointer("node",i,(void **)&node) == B_OK) {
		if (undoMessage->FindPointer("parentGroupList",i,(void **)&parentGroupList) == B_OK)
			parentGroupList->AddItem(node);
		else {
			if (node->what != P_C_CONNECTION_TYPE)
				allNodes->AddItem(node);
			else
				allConnections->AddItem(node);
		}
		//(doc->GetTrash())->RemoveItem(node);
		(doc->GetSelected())->AddItem(node);
		i++;
		changed->insert(node);
	}
	doc->SetModified();
}

BMessage* Delete::Do(PDocument *doc, BMessage *settings) {
	BMessage		*undoMessage		= new BMessage();
	BList			*selected			= doc->GetSelected();
	BList			*connections		= doc->GetAllConnections();
	BList			*allNodes			= doc->GetAllNodes();
	BList			*gallNodes			= NULL;
	set<BMessage*>	*changed			= doc->GetChangedNodes();
	BMessage		*node				= NULL;
	BMessage		*connection			= NULL;
	BMessage		*parent				= NULL;
	BList			*outgoing			= NULL;
	BList			*incoming			= NULL;
	int32			i					= 0;
	while (	(node = (BMessage *)selected->RemoveItem(i)) != NULL) {
		allNodes->RemoveItem(node);
		connections->RemoveItem(node);
		changed->insert(node);
		undoMessage->AddPointer("node",node);
		if (node->FindPointer("Node::outgoing",(void **)&outgoing) == B_OK) {
			for (i=0;i<outgoing->CountItems();i++) {
				connection= (BMessage *)outgoing->ItemAt(i);
				connections->RemoveItem(connection);
				changed->insert(connection);
				undoMessage->AddPointer("node",connection);
			}
		}
		if (node->FindPointer("Node::incoming",(void **)&incoming) == B_OK) {
			for (i=0;i<incoming->CountItems();i++) {
				connection= (BMessage *)incoming->ItemAt(i);
				connections->RemoveItem(connection);
				changed->insert(connection);
				undoMessage->AddPointer("node",connection);
			}
		}
		//** find all NOdes wich belong to a group and delete them - korrekt the undopart??
		if  (node->what == P_C_GROUP_TYPE){
			if (node->FindPointer("Node::allNodes", (void **)&gallNodes) == B_OK)
				for (i=0; i< gallNodes->CountItems(); i++){
					allNodes->RemoveItem(gallNodes->ItemAt(i));
					connections->RemoveItem(gallNodes->ItemAt(i));
					changed->insert((BMessage*)gallNodes->ItemAt(i));
					undoMessage->AddPointer("node",gallNodes->ItemAt(i));
					//***somehow store the grouping...
				}
		}
		if (node->FindPointer("Parent",(void **)&parent) != B_OK && parent != NULL) {
			if (parent->FindPointer("Node::allNodes", (void **)&gallNodes) == B_OK && gallNodes != NULL){
				gallNodes->RemoveItem(node);
				changed->insert(parent);
				//** do the undopart
			}
		}
	}
	doc->SetModified();
	settings->RemoveName("Delete::Undo");
	settings->AddMessage("Delete::Undo",undoMessage);
	settings = PCommand::Do(doc,settings);
	return settings;
}



void Delete::AttachedToManager(void) {
}

void Delete::DetachedFromManager(void) {
}
