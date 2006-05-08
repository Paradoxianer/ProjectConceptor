#include <app/Clipboard.h>
#include "ProjectConceptorDefs.h"
#include "Copy.h"
#include "Indexer.h"


Copy::Copy():PCommand()
{
}

void Copy::Undo(PDocument *doc,BMessage *undo)
{
	// nothing to do :-)
}

BMessage* Copy::Do(PDocument *doc, BMessage *settings)
{
	BMessage 	*clip				= NULL;
	BMessage	*node				= NULL;
	BMessage	*connect			= NULL;
	bool		connectselect		= false;
	BMessage	*copyMessage		= new BMessage();
	BList		*selected			= doc->GetSelected();
	BList		*allConnections		= doc->GetAllConnections();

	int32		i					= 0;
	Indexer		*indexer			= new Indexer(doc);
	if (doc->ReadLock())
	{
		for (i=0;i<selected->CountItems();i++)
		{
			if (node=(BMessage *)selected->ItemAt(i))
			{
				if (node->what == P_C_CONNECTION_TYPE)
					copyMessage->AddMessage("connection",indexer->IndexConnection(node,true));
				else
					copyMessage->AddMessage("node",indexer->IndexNode(node));
			}

		}
/*		for (i=0;i<allConnections->CountItems();i++ )
		{
			copyMessage->AddMessage("connection",indexer->IndexConnection((BMessage *)allConnections->ItemAt(i),true));
		}		*/
		doc->ReadUnlock();
	}
	if (be_clipboard->Lock()) 
	{
		be_clipboard->Clear();
		if (clip = be_clipboard->Data()) 
		{
			clip->AddData("application/x-vnd.projectconceptor-document", B_MIME_TYPE, copyMessage, sizeof(copyMessage));
			be_clipboard->Commit();
		}
		be_clipboard->Unlock();
	} 
	settings = PCommand::Do(doc,settings);
	return settings;
}



void Copy::AttachedToManager(void)
{
}

void Copy::DetachedFromManager(void)
{
}
