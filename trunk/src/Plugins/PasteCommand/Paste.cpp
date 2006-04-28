#include "ProjectConceptorDefs.h"
#include "Paste.h"


Paste::Paste():PCommand()
{
}

void Paste::Undo(PDocument *doc,BMessage *undo)
{
	BList			*connections		= doc->GetAllConnections();
	BList			*allNodes			= doc->GetAllNodes();
	BList			*trash				= doc->GetTrash();
	BList			*changed			= doc->GetChangedNodes();
	BMessage		*node				= new BMessage();
	BMessage		*connection			= new BMessage();
	int32			i					= 0;
	PCommand::Undo(doc,undo);
	while (undo->FindPointer("node",i,(void **)&node) == B_OK)
	{
		allNodes->RemoveItem(node);
		trash->AddItem(node);
		changed->AddItem(node);
		i++;
	}
	i=0;
	while (undo->FindPointer("connection",i,(void **)&connection) == B_OK) 
	{
		i++;
		connections->RemoveItem(connection);
		changed->AddItem(connection);	
		trash->AddItem(connection);
	}
	doc->SetModified();
}

BMessage* Paste::Do(PDocument *doc, BMessage *settings)
{
	BMessage		*node				= NULL;
	BList			*parentGroupList	= NULL;
	BList			*changed			= doc->GetChangedNodes();
	BList			*allConnectinos		= doc->GetAllConnections();
	BList			*allNodes			= doc->GetAllNodes();
	int32			i					= 0;
	while (settings->FindPointer("node",i,(void **)&node) == B_OK)
	{
		if (settings->FindPointer("parentGroupList",i,(void **)&parentGroupList) == B_OK)
			parentGroupList->AddItem(node);
		else
		{
			if (node->what != P_C_CONNECTION_TYPE)
				allNodes->AddItem(node);
			else
				allConnectinos->AddItem(node);
			//recalc size
			BRect	insertFrame		= BRect(0,0,0,0);
			
			if (node->FindRect("Frame",&insertFrame))
			{
				BRect	docRect			= doc->Bounds();
				if (insertFrame.bottom >= docRect.Height())
				{
					docRect.bottom= insertFrame.bottom+20;
				}
				if (insertFrame.right >= docRect.Width())
				{
					docRect.right = insertFrame.right+20;
				}
				if (docRect != doc->Bounds())
				{
					doc->Resize(docRect.right,docRect.bottom);
				}
			}
		}
		i++;
		if (!changed->HasItem(node))
			changed->AddItem(node);
	}
	doc->SetModified();
	settings = PCommand::Do(doc,settings);
	return settings;
}



void Paste::AttachedToManager(void)
{
}

void Paste::DetachedFromManager(void)
{
}
