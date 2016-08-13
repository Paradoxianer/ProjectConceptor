#include "Group.h"
#include "ProjectConceptorDefs.h"


Group::Group():PCommand() {
}

void Group::Undo(PDocument *doc,BMessage *undo) {
	PCommand::Undo(doc,undo);
	int32 			i					= 0;
	BMessage		*currentContainer	= NULL;
	BMessage		*node				= NULL;
	BMessage		*subNode			= NULL;
	BList			*subList			= NULL;
	BList			*selected			= doc->GetSelected();
	set<BMessage*>		*changed			= doc->GetChangedNodes();
	BList			*allNodes			= doc->GetAllNodes();
	if ( (undo->FindPointer("node",i,(void **)&node) == B_OK) && (node) ) {
		allNodes->RemoveItem(node);
		if ( (node->FindPointer(P_C_NODE_ALLNODES,(void **)&subList) == B_OK) && (subList) ) {
			while (subList->CountItems()>0) {
				subNode	= (BMessage *)subList->RemoveItem((int32)0);
				allNodes->AddItem(subNode);
				changed->insert(subNode);
			}
		}

		if ((node->FindPointer(P_C_NODE_ALLCONNECTIONS,(void **)&subList) == B_OK) && (subList))	{
			while (subList->CountItems()>0) {
				subNode	= (BMessage *)subList->RemoveItem((int32)0);
				changed->insert(subNode);
			}
		}
	}
}

BMessage* Group::Do(PDocument *doc, BMessage *settings) {
	//stores all undoinformations
	BMessage		*undoMessage		= new BMessage();
	BMessage		*node				= NULL;
	BMessage		*groupedNode		= NULL;
	BMessage		*paretnNode			= NULL;
	BMessage		*oldParentNode		= NULL;

	//the allNodes List and the allConnections List where the nodes come from
	BList			*allNodes			= allNodes=doc->GetAllNodes();
	
	//the allNodes List and the allConnections List where the nodes should go to
	BList			*gAllNodes			= NULL;
	BList			*gAllOldNodes		= NULL;
	BList			*gAllConnections	= NULL;
	
	bool			deselect			= true;
	int32			i					= 0;
	BRect			*selectFrame		= new BRect();

	//get all selected Nodes... this Nodes will be Members of the New Group
	BList			*selected			= doc->GetSelected();
	set<BMessage*>	*changed			= doc->GetChangedNodes();
	//detect all Connections wich belongs to the group
	BList			*incoming			= NULL;
	BList			*outgoing			= NULL;
	
	//errorsstuff :)
	status_t		err					= settings->FindBool("deselect",&deselect);
	
	//to calculate the size of the Group
	BRect			groupFrame			= BRect(0,0,-1,-1);
	BRect			groupedNodeFrame	= BRect(0,0,-1,-1);

	if  (settings->FindPointer("node",(void **)&node) == B_OK) {
		//paranoid security check :)
		if (node->what == P_C_GROUP_TYPE) {
			if (!allNodes->HasItem(node))
				allNodes->AddItem(node);
			//try to find the Lists for the Nodes where the grouped Node should go in
			if (node->FindPointer(P_C_NODE_ALLNODES,(void **)&gAllNodes) != B_OK){
				gAllNodes = new BList();
				node->AddPointer(P_C_NODE_ALLNODES, gAllNodes);
			}
			//all selectetd 
			for (i=0;i<selected->CountItems();i++) {
				groupedNode = (BMessage *)selected->ItemAt(i);
				if ((groupedNode != NULL) && ( (groupedNode->what == P_C_CLASS_TYPE) || (groupedNode->what == P_C_GROUP_TYPE)) ) {
					if (groupedNode->FindPointer(P_C_NODE_PARENT,(void **)&oldParentNode) != B_OK) {
						groupedNode->AddPointer(P_C_NODE_PARENT,node);
						gAllNodes->AddItem(groupedNode);					
						//calculate the groupFrame
						if (!groupFrame.IsValid())
							groupedNode->FindRect(P_C_NODE_FRAME,&groupFrame);
						else {
							groupedNode->FindRect(P_C_NODE_FRAME,&groupedNodeFrame);
							groupFrame = groupFrame | groupedNodeFrame;
						}
						//add this to the changed List
						changed->insert(groupedNode);
					}
				}
			}
			if (groupedNodeFrame.IsValid()) {
				groupFrame.InsetBy(-5,-5);
				groupFrame.top = groupFrame.top-15;
				node->ReplaceRect(P_C_NODE_FRAME,groupFrame);
			}
			else
			    node->FindRect(P_C_NODE_FRAME,&groupFrame);
			if (groupFrame.IsValid()) {
				BRect	docRect			= doc->Bounds();
				if (groupFrame.bottom >= docRect.Height())
					docRect.bottom= groupFrame.bottom+20;
				if (groupFrame.right >= docRect.Width())
					docRect.right = groupFrame.right+20;
				if (docRect != doc->Bounds())
					doc->Resize(docRect.right,docRect.bottom);
			}
			//**check if there is a passed "docRect"
		}
	}
	settings->RemoveName("Group::Undo");
	settings->AddMessage("Group::Undo",undoMessage);
	settings	= PCommand::Do(doc,settings);
	doc->SetModified();
	return settings;
}



void Group::AttachedToManager(void) {
}

void Group::DetachedFromManager(void) {
}

