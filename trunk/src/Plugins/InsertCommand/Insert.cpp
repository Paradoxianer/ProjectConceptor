#include "ProjectConceptorDefs.h"
#include "Insert.h"


Insert::Insert():PCommand()
{
}

void Insert::Undo(PDocument *doc,BMessage *undo)
{
	BList			*allConnectinos		= doc->GetAllConnections();
	BList			*allNodes			= doc->GetAllNodes();
	BList			*trash				= doc->GetTrash();
	BList			*changed			= doc->GetChangedNodes();
	BMessage		*node				= new BMessage();
	BMessage		*connection			= new BMessage();
	int32			i					= 0;
	PCommand::Undo(doc,undo);
	while (undo->FindPointer("node",i,(void **)&node) == B_OK)
	{
		if (node->what != P_C_CONNECTION_TYPE)
			allNodes->RemoveItem(node);
		else
			allConnectinos->RemoveItem(node);
		trash->AddItem(node);
		changed->AddItem(node);
		i++;
	}
	i=0;

	doc->SetModified();
}

BMessage* Insert::Do(PDocument *doc, BMessage *settings)
{
	BMessage		*node				= NULL;
	BList			*parentGroupList	= NULL;
	BList			*changed			= doc->GetChangedNodes();
	BList			*allConnections		= NULL;
	BList			*allNodes			= NULL;
	int32			i					= 0;
	if 	((settings->FindPointer("allNodes",(void **)&allNodes)!=B_OK) || (allNodes==NULL))
		allNodes = doc->GetAllNodes();
	if ((settings->FindPointer("allConnections",(void **)&allConnections) != B_OK) || (allConnections == NULL))
		allConnections = doc->GetAllConnections();

	while (settings->FindPointer("node",i,(void **)&node) == B_OK)
	{
		if (node->what != P_C_CONNECTION_TYPE)
			allNodes->AddItem(node);
		else
			allConnections->AddItem(node);
		//recalc size
		//**check if there is a passed "docRect"
		BRect	insertFrame		= BRect(0,0,0,0);
		if (node->FindRect("Frame",&insertFrame))
		{
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



void Insert::AttachedToManager(void)
{
}

void Insert::DetachedFromManager(void)
{
}
