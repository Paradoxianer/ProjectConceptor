#include "PDocumentManager.h"



PDocumentManager::PDocumentManager(void)
{
	TRACE();
	Init();
	CreateDocument();
}

PDocumentManager::PDocumentManager(BMessage *archive)
{
	Init();
	TRACE();
}

void PDocumentManager::Init()
{
	app_info	info;
	BPath		path;
	pluginManager	= new PluginManager();;
	documentList	= new BList();
	be_app->GetAppInfo(&info); 
    BEntry entry(&info.ref); 
    entry.GetPath(&path); 
    path.GetParent(&path);
    //in das Unterverzeichniss "Plugins wechseln"
	path.Append("Plugins");
	BDirectory *dir	= new BDirectory(path.Path());
	pluginManager->LoadPlugins(dir);
}
PDocumentManager::~PDocumentManager(void)
{
	PDocument *currentDocument	= NULL;
	for (int32 i=0; i<documentList->CountItems();i++)
	{
		currentDocument		= (PDocument *)documentList->ItemAt(i);
		if (currentDocument->Lock())
		{
			currentDocument->Quit();
			delete currentDocument;
		}
	}
	delete pluginManager;
}
/**
 *@todo set the docname;
 */
void PDocumentManager::AddDocument(PDocument *doc)
{
	TRACE();
	if (doc)
	{
		if (documentList->AddItem(doc))
		{
			//**Doc Namen setzen..
		}
		else
			PRINT(("ERROR:\tPDocumentManager Cant Add Document\n"));
	}
	else
		PRINT(("ERROR:\tPDocumentManager","Wrong Value passed\n"));
}


void PDocumentManager::RemoveDocument(PDocument *doc)
{
	TRACE();
	documentList->RemoveItem(doc);
	if (documentList->CountItems()==0)
	{
		be_app_messenger.SendMessage(B_QUIT_REQUESTED);
	}
}


PDocument* PDocumentManager::CreateDocument(void)
{
	TRACE();
	PDocument *doc = new PDocument(this);
	AddDocument(doc);
	return doc;
}
