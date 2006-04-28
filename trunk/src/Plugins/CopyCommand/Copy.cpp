#include "ProjectConceptorDefs.h"
#include "Copy.h"


Copy::Copy():PCommand()
{
}

void Copy::Undo(PDocument *doc,BMessage *undo)
{
	//** nothing to do :-)
}

BMessage* Copy::Do(PDocument *doc, BMessage *settings)
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



void Copy::AttachedToManager(void)
{
}

void Copy::DetachedFromManager(void)
{
}
