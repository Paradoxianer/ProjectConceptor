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
	BList			*allNodes			= NULL;
	BList			*allConnections		= NULL;
	undo->FindPointer("allNodes",i,(void **)&allNodes);
	if (allNodes==NULL)
		allNodes=doc->GetAllNodes();
	undo->FindPointer("allConnections",i,(void **)&allConnections);
	if (allConnections==NULL)
		allConnections=doc->GetAllConnections();
	if ( (undo->FindPointer("node",i,(void **)&node) == B_OK) && (node) )
	{
		allNodes->RemoveItem(node);
		if ( (node->FindPointer("allNodes",(void **)&subList) == B_OK) && (subList) )
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
				allConnections->AddItem(subNode);
				changed->AddItem(subNode);
			}
		}
	}
}

BMessage* Group::Do(PDocument *doc, BMessage *settings)
{
	BRect			*selectFrame		= new BRect();
	BMessage		*node				= NULL;
	BMessage		*groupedNode		= NULL;
	//the allNodes List and the allConnectionsList from the Group
	BList			*allNodes			= NULL;
	BList			*allConnections		= NULL;
	BList			*gAllNodes			= NULL;
	BList			*gAllConnections	= NULL;
	BMessage		*commandMessage		= new BMessage();
	bool			selectAll			= false;
	bool			deselect			= true;
	int32			i					= 0;
	//get all selected Nodes... this Nodes will be Members of the New Group
	BList			*selected			= doc->GetSelected();
	BList			*changed			= doc->GetChangedNodes();
	status_t		err					= settings->FindBool("deselect",&deselect);
	BRect			groupFrame			= BRect(0,0,-1,-1);
	BRect			groupedNodeFrame	= BRect(0,0,-1,-1);

	if  (settings->FindPointer("node",(void **)&node) == B_OK)
	{
		//if a allNodes List hase passe we use this List because this List coud be a List of a group (not the document->allNodes List)
		settings->FindPointer("allNodes",i,(void **)&allNodes);
		if (allNodes==NULL)
			allNodes=doc->GetAllNodes();
		settings->FindPointer("allConnections",i,(void **)&allConnections);
		if (allConnections==NULL)
			allConnections=doc->GetAllConnections();
		if (node->what != P_C_CONNECTION_TYPE)
			allNodes->AddItem(node);
		else
			allConnections->AddItem(node);
		//recalc size
		//**check if there is a passed "docRect"
		if (node->FindPointer("allNodes",(void **)&gAllNodes) != B_OK)
			node->AddPointer("allNodes", gAllNodes = new BList());
		if (node->FindPointer("allConnections",(void **)&gAllConnections))
			node->AddPointer("allConnections", gAllConnections = new BList());
		for (i=0;i<selected->CountItems();i++)
		{
			groupedNode = (BMessage *)selected->ItemAt(i);
			if (groupedNode != NULL)
			{
				allNodes->RemoveItem(groupedNode);
//				groupedNode->AddPointer("Parent",node);
				gAllNodes->AddItem(groupedNode);
				if (!groupFrame.IsValid())
					groupedNode->FindRect("Frame",&groupFrame);
				else
				{
					groupedNode->FindRect("Frame",&groupedNodeFrame);
					groupFrame = groupFrame | groupedNodeFrame;
				}
				if (!changed->HasItem(groupedNode))
					changed->AddItem(groupedNode);

			}
		}
		if (groupedNodeFrame.IsValid())
		{
			groupFrame.InsetBy(-5,-5);
			groupFrame.top = groupFrame.top-15;
			node->ReplaceRect("Frame",groupFrame);
		}
		else
		{
			node->FindRect("Frame",&groupFrame);
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
	}
	/*if ((deselect)|| (err != B_OK))
	{
		while (selected->CountItems()>0)
		{
			node	= (BMessage *)selected->RemoveItem((int32)0);
			if (node != NULL)
			{
				changed->AddItem(node);
				commandMessage->AddPointer("node",node);
				node->ReplaceBool("selected",0,false);
			}
		}
	}*/
	i = 0;
	/*while (settings->FindRect("frame",i,selectFrame) == B_OK)
	{
		DoGroup(doc,selectFrame);
		i++;
	}
	i = 0;
	while (settings->FindPointer("node",i,(void **)&node) == B_OK)
	{
		DoGroup(doc,node);
		i++;
	}
	if (settings->FindBool("selectAll",&selectAll) == B_OK)
		if (selectAll) DoGroupAll(doc);*/
	commandMessage	= PCommand::Do(doc,settings);
	doc->SetModified();
	return commandMessage;
}



void Group::AttachedToManager(void)
{
}

void Group::DetachedFromManager(void)
{
}
