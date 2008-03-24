#include "Delete.h"
#include "ProjectConceptorDefs.h"

Delete::Delete():PCommand()
{
}

void Delete::Undo(PDocument *doc,BMessage *undo)
{
	BMessage		*undoMessage		= new BMessage();
	BMessage		*settings			= new BMessage();
	BMessage		*node				= NULL;
	BMessage		*connection			= NULL;
	BList			*parentGroupList	= NULL;
	BList			*changed			= doc->GetChangedNodes();
	BList			*allNodes			= doc->GetAllNodes();
	BList			*allConnections		= doc->GetAllConnections();
	BMessage		*commandMessage		= new BMessage();
	int32			i					= 0;
	PCommand::Undo(doc,undo);
	undo->FindMessage("Delete::Undo" ,undoMessage);
	while (undoMessage->FindPointer("node",i,(void **)&node) == B_OK)
	{
		if (undoMessage->FindPointer("parentGroupList",i,(void **)&parentGroupList) == B_OK)
			parentGroupList->AddItem(node);
		else
		{
			if (node->what != P_C_CONNECTION_TYPE)
				allNodes->AddItem(node);
			else
				allConnections->AddItem(node);
		}
		//(doc->GetTrash())->RemoveItem(node);
		(doc->GetSelected())->AddItem(node);
		i++;
		if (!changed->HasItem(node))
			changed->AddItem(node);
	}
	doc->SetModified();
}

BMessage* Delete::Do(PDocument *doc, BMessage *settings)
{
/*	BMessage	*node				= new BMessage();
	BMessage	*commandMessage		= PCommand::Do(doc,settings);
	BList		*parentGroupList	= NULL;
	BMessage	*connection			= new BMessage();
	int32		i					= 0;
	while (settings->FindPointer("node",i,(void **)&node))
	{
		if (settings->FindPointer("parentGroupList",i,(void **)&parentGroupList))
			parentGroupList->RemoveItem(node);
		else
			(doc->GetAllNodes())->RemoveItem(node);
		i++;
	}
	i = 0;
	while (settings->FindPointer("connection",i,(void **)&connection))
	{
		i++;
		(doc->GetAllConnections())->RemoveItem(connection);
	}
	return commandMessage;*/
	//**Todo
	BMessage		*undoMessage		= new BMessage();
	BList			*selected			= doc->GetSelected();
	BList			*connections		= doc->GetAllConnections();
	BList			*allNodes			= doc->GetAllNodes();
	BList			*changed			= doc->GetChangedNodes();
	BMessage		*node				= NULL;
	BMessage		*connection			= NULL;
	BList			*outgoing			= NULL;
	BList			*incoming			= NULL;
	int32			i					= 0;
	while (	(node = (BMessage *)selected->RemoveItem(i)) != NULL)
	{
		allNodes->RemoveItem(node);
		connections->RemoveItem(node);
		changed->AddItem(node);
		undoMessage->AddPointer("node",node);
		if (node->FindPointer("Node::outgoing",(void **)&outgoing) == B_OK)
		{
			for (int32 i=0;i<outgoing->CountItems();i++)
			{
				connection= (BMessage *)outgoing->ItemAt(i);
				connections->RemoveItem(connection);
//				trash->AddItem(connection);
				changed->AddItem(connection);
				undoMessage->AddPointer("node",connection);
			}
		}
		if (node->FindPointer("Node::incoming",(void **)&incoming) == B_OK)
		{
			for (int32 i=0;i<incoming->CountItems();i++)
			{
				connection= (BMessage *)incoming->ItemAt(i);
				connections->RemoveItem(connection);
//				trash->AddItem(connection);
				changed->AddItem(connection);
				undoMessage->AddPointer("node",connection);
			}
		}
	}
	doc->SetModified();
	settings->RemoveName("Delete::Undo");
	settings->AddMessage("Delete::Undo",undoMessage);
	settings = PCommand::Do(doc,settings);
	return settings;
}



void Delete::AttachedToManager(void)
{
}

void Delete::DetachedFromManager(void)
{
}
