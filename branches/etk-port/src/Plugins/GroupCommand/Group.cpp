#include "Group.h"

Group::Group():PCommand()
{
}

void Group::Undo(PDocument *doc,BMessage *undo)
{
	PCommand::Undo(doc,undo);
	int32 			i					= 0;
	BMessage		*currentContainer	= NULL;
	BMessage		*node				= NULL;
	BMessage		*subNode			= NULL;
	BList			*subList			= NULL;
	BList			*selected			= doc->GetSelected();
	BList			*changed			= doc->GetChangedNodes();
	BList			*allNodes			= doc->GetAllNodes();
	if ( (undo->FindPointer("node",i,(void **)&node) == B_OK) && (node) )
	{
		allNodes->RemoveItem(node);
		if ( (node->FindPointer("Node::allNodes",(void **)&subList) == B_OK) && (subList) )
		{
			while (subList->CountItems()>0)
			{
				subNode	= (BMessage *)subList->RemoveItem((int32)0);
				allNodes->AddItem(subNode);
				changed->AddItem(subNode);
			}
		}

		if ((node->FindPointer("allConnections",(void **)&subList) == B_OK) && (subList))
		{
			while (subList->CountItems()>0)
			{
				subNode	= (BMessage *)subList->RemoveItem((int32)0);
				changed->AddItem(subNode);
			}
		}
	}
}

BMessage* Group::Do(PDocument *doc, BMessage *settings)
{
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
	BList			*gAllConnections	= NULL;
	
	bool			deselect			= true;
	int32			i					= 0;
	BRect			*selectFrame		= new BRect();

	//get all selected Nodes... this Nodes will be Members of the New Group
	BList			*selected			= doc->GetSelected();
	BList			*changed			= doc->GetChangedNodes();
	//detect all Connections wich belongs to the group
	BList			*incoming			= NULL;
	BList			*outgoing			= NULL;
	
	//errorsstuff :)
	status_t		err					= settings->FindBool("deselect",&deselect);
	
	//to calculate the size of the Group
	BRect			groupFrame			= BRect(0,0,-1,-1);
	BRect			groupedNodeFrame	= BRect(0,0,-1,-1);

	if  (settings->FindPointer("node",(void **)&node) == B_OK)
	{
		//paranoid security check :)
		if (node->what == P_C_GROUP_TYPE)
		{
			if (!allNodes->HasItem(node))
				allNodes->AddItem(node);
			//try to find the Lists for the Nodes where the grouped Node should go in
			if (node->FindPointer("Node::allNodes",(void **)&gAllNodes) != B_OK)
				node->AddPointer("Node::allNodes", gAllNodes = new BList());
			//all selectetd 
			for (i=0;i<selected->CountItems();i++)
			{
				groupedNode = (BMessage *)selected->ItemAt(i);
				if ((groupedNode != NULL) && ( (groupedNode->what == P_C_CLASS_TYPE) || (groupedNode->what == P_C_GROUP_TYPE)) )
				{
					//add the parentNode so that we can find out to wich parent the new groupedNode belongs
					if (groupedNode->FindPointer("parentNode",(void **)&oldParentNode) == B_OK)
					{
					/*	BList	*oldAllNodesList	= NULL;
						oldParentNode->FindPointer("Node::allNodes",(void **) &oldAllNodesList);
						if (oldAllNodesList)
							oldAllNodesList->RemoveItem(groupedNode);
						undoMessage->AddPointer("changeNode",groupedNode);
						undoMessage->AddPointer("oldParentNode",oldParentNode);
						groupedNode->ReplacePointer("parentNode",node);*/
					}
					else
						groupedNode->AddPointer("parentNode",node);
					groupedNode->AddPointer("parentNode",node);
					gAllNodes->AddItem(groupedNode);
					
					//calculate the groupFrame
					if (!groupFrame.IsValid())
						groupedNode->FindRect("Node::frame",&groupFrame);
					else
					{
						groupedNode->FindRect("Node::frame",&groupedNodeFrame);
							groupFrame = groupFrame | groupedNodeFrame;
					}
					//add this to the changed List
					if (!changed->HasItem(groupedNode))
						changed->AddItem(groupedNode);	
				}
			}
			if (groupedNodeFrame.IsValid())
			{
				groupFrame.InsetBy(-5,-5);
				groupFrame.top = groupFrame.top-15;
				node->ReplaceRect("Node::frame",groupFrame);
			}
			else
			{
				node->FindRect("Node::frame",&groupFrame);
			}
			if (groupFrame.IsValid())
			{
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



void Group::AttachedToManager(void)
{
}

void Group::DetachedFromManager(void)
{
}

