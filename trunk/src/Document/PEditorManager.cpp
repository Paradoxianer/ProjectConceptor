#include "PEditorManager.h"
#include "ShortCutFilter.h"
#include <support/ClassInfo.h>

PEditorManager::PEditorManager(PDocument *initDoc)
{
	TRACE();
	doc = initDoc;
	Init();
}

PEditorManager::PEditorManager(BMessage *archive)
{
	TRACE();
	Init();
}
void PEditorManager::Init(void)
{
	TRACE();
	editors			= new BList();
	editorMessenger	= new BList();
	activeEditor	= new BList();
}

void PEditorManager::BroadCast(BMessage *message)
{
	TRACE();
//	editors->DoForEach(SendMessage,message);
	editorMessenger->DoForEach(SendMessage,message);
}
void PEditorManager::UnregisterPEditor(PEditor *editor)
{
	TRACE();
	int32 index=editors->IndexOf(editor);
	if (editors->RemoveItem(index))
	{

		editorMessenger->RemoveItem(index);
		editor->SetManager(NULL);
	}
}

status_t PEditorManager::RegisterPEditor(PEditor *editor)
{
	TRACE();
	status_t err = B_OK;
	//tests if the passed value is valid
	if (editor)
	{
		//test´s if this editor ist already registered in this Manager
		if (!editors->HasItem(editor))
		{
			//try to add
			if (editors->AddItem(editor))
			{
				
				if ( ((editor->GetHandler())->Looper() == NULL) && (doc->Lock()) )
				{			
					doc->AddHandler(editor->GetHandler());
					doc->Unlock();	
				}
				BMessenger *messenger =			new BMessenger(editor->GetHandler(),NULL,&err);
				if (err!=B_OK)
				PRINT(("ERROR\t new BMessenger %s\n",strerror(err)));
				editorMessenger->AddItem(messenger);
				editor->SetManager(this);
				BMessage	*configMessage		= editor->GetConfiguration();
				BMessage	*shortcutMessage	= new BMessage();
				if (configMessage)
				{
					if ((configMessage->FindMessage("Shortcuts",shortcutMessage)==B_OK) && (shortcutMessage))
					{
							ShortCutFilter *shortcutFilter	= new ShortCutFilter(shortcutMessage);
							if ((editor->GetHandler())->LockLooper())
							{
								(editor->GetHandler())->AddFilter(shortcutFilter);
								editor->GetHandler()->UnlockLooper();
							}
					}
				}
				err=B_OK;
			}
			else
			{
				//something went wrong during the add of the PEditor
				err=B_ERROR;
			}
		}
	}
	else
		//wron Value was passed ;-)
		err=B_BAD_VALUE;
	return err;
}

PEditor* PEditorManager::GetActivPEditor(void)
{
	TRACE();
	return (PEditor *)editors->ItemAt((int32)activeEditor->LastItem());
}
void PEditorManager::SetActivePEditor(int32 index)
{
	TRACE();
	void *	value	= (void *)index;
	activeEditor->AddItem(value);
}

void PEditorManager::SetInactivePEditor(int32 index)
{
	TRACE();
	activeEditor->RemoveItem((void *)index);
}

PEditor* PEditorManager::InstantiatePEditor(BMessage *archive)
{
	TRACE();

/*	int32 	i				= 0;
	bool	instantiated	= false;
	PEditor*	newPEditor		= NULL;
	while ((i<editors->CountItems())&&(!instantiated))
	{
		PEditor* editor=(PEditor *)editors->ItemAt(i);
		newPEditor=(PEditor*)editor->Instantiate(archive);
		if (newPEditor!=NULL)
			instantiated=true;
		i++;	
	}*/
	PEditor *newInstance=NULL;
	BArchivable *base = instantiate_object(archive);
	if ( base ) 
		newInstance	= cast_as(base, PEditor);
 	return newInstance;
}

bool PEditorManager::SendMessage(void *arg,void *msg)
{
	TRACE();
	status_t	err	= B_OK;
	BMessenger* theMessenger=(BMessenger*)arg;
	err=theMessenger->SendMessage((BMessage*)msg);
/*	BHandler	*theHandler	= ((PEditor *)arg)->GetHandler();
	theHandler->MessageReceived((BMessage *)msg);*/
	return false;
}


