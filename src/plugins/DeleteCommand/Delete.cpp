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
	// i is deliberately never incremented - RemoveItem(0) repeatedly is
	// the correct way to drain a shrinking BList, since every removal
	// shifts the next item into index 0. The inner loops below used to
	// reuse this same `i` as their own counter, left it at whatever value
	// their own list's size happened to produce, and this loop's next
	// RemoveItem(i) call then used that leftover value instead of 0 -
	// skipping or repeating items unpredictably, and in a document with
	// grouped nodes and connections together (any real graph, basically),
	// producing a live hang: confirmed via ./dev.sh smoke-style manual
	// testing, "Select all" then "Clear" on such a document never returns.
	int32			i					= 0;
	while (	(node = (BMessage *)selected->RemoveItem(i)) != NULL) {
		allNodes->RemoveItem(node);
		connections->RemoveItem(node);
		changed->insert(node);
		undoMessage->AddPointer("node",node);
		if (node->FindPointer(P_C_NODE_OUTGOING,(void **)&outgoing) == B_OK) {
			for (int32 j=0;j<outgoing->CountItems();j++) {
				connection= (BMessage *)outgoing->ItemAt(j);
				connections->RemoveItem(connection);
				changed->insert(connection);
				undoMessage->AddPointer("node",connection);
			}
		}
		if (node->FindPointer(P_C_NODE_INCOMING,(void **)&incoming) == B_OK) {
			for (int32 j=0;j<incoming->CountItems();j++) {
				connection= (BMessage *)incoming->ItemAt(j);
				connections->RemoveItem(connection);
				changed->insert(connection);
				undoMessage->AddPointer("node",connection);
			}
		}
		//** find all NOdes wich belong to a group and delete them - korrekt the undopart??
		if  (node->what == P_C_GROUP_TYPE){
			if (node->FindPointer(P_C_NODE_ALLNODES, (void **)&gallNodes) == B_OK)
				for (int32 j=0; j< gallNodes->CountItems(); j++){
					allNodes->RemoveItem(gallNodes->ItemAt(j));
					connections->RemoveItem(gallNodes->ItemAt(j));
					changed->insert((BMessage*)gallNodes->ItemAt(j));
					undoMessage->AddPointer("node",gallNodes->ItemAt(j));
					//***somehow store the grouping...
				}
		}
		// was "!= B_OK && parent != NULL" - FindPointer failing leaves
		// `parent` untouched (not NULL'd), so this always evaluated
		// against whatever `parent` happened to hold from a previous
		// iteration instead of this node's actual parent.
		if (node->FindPointer(P_C_NODE_PARENT,(void **)&parent) == B_OK && parent != NULL) {
			if (parent->FindPointer(P_C_NODE_ALLNODES, (void **)&gallNodes) == B_OK && gallNodes != NULL){
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
