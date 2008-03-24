#include <app/Clipboard.h>
#include <support/Debug.h>
#include "ProjectConceptorDefs.h"
#include "Paste.h"
#include "Indexer.h"


Paste::Paste():PCommand()
{
}

void Paste::Undo(PDocument *doc,BMessage *undo)
{
	// nothing to do :-)
}

BMessage* Paste::Do(PDocument *doc, BMessage *settings)
{
	BMessage 	*clip				= NULL;
	BMessage	*node				= new BMessage();
	BMessage	*deIndexedNode		= NULL;
	BMessage	*connect			= NULL;
	bool		connectselect		= false;
	BMessage	*copyMessage		= new BMessage;
	int32		messagelen			= 0;
	int32		i					= 0;
	Indexer		*indexer			= new Indexer(doc);
	if (be_clipboard->Lock())
	{
		if (clip = be_clipboard->Data())
		{
//			clip->FindData("application/x-vnd.projectconceptor-document", B_MIME_TYPE, (const void **)&copyMessage, &messagelen);
			clip->FindMessage("test",copyMessage);
			copyMessage->PrintToStream();
		}
		be_clipboard->Unlock();
	}

	if (copyMessage)
	{
		BMessage	*inserter	= new BMessage(P_C_EXECUTE_COMMAND);
		BMessage	*select		= new BMessage(P_C_EXECUTE_COMMAND);
		inserter->AddString("Command::Name","Insert");
		select->AddString("Command::Name","Select");
		while (copyMessage->FindMessage("node",i,node) == B_OK)
		{
			if (node->what == P_C_CONNECTION_TYPE)
				deIndexedNode		= indexer->DeIndexConnection(node);
			else
			{
				deIndexedNode		= indexer->RegisterDeIndexNode(node);
				//only select nodes.. because es the copy and paste funktion with selected nodes dosent work proper
				select->AddPointer("node",deIndexedNode);
			}
			inserter->AddPointer("node",deIndexedNode);

			i++;
			node = new BMessage();
		}
		i=0;
		while (inserter->FindPointer("node",i,(void **)&node) == B_OK)
		{
			if (node->what != P_C_CONNECTION_TYPE)
				indexer->DeIndexNode(node);
			i++;
		}
		inserter->AddMessage("PCommand::subPCommand",select);
		PRINT_OBJECT(*inserter);
		(new BMessenger(NULL,doc))->SendMessage(inserter);
	}
	return settings;
}



void Paste::AttachedToManager(void)
{
}

void Paste::DetachedFromManager(void)
{
}
