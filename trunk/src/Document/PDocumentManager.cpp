#include "PDocumentManager.h"



PDocumentManager::PDocumentManager(void) : BArchivable()
{
	TRACE();
	Init();
	CreateDocument();
}

PDocumentManager::PDocumentManager(BMessage *archive) : BArchivable(archive)
{
	Init(archive);
	TRACE();
}

void PDocumentManager::Init()
{
	app_info	info;
	BPath		path;
	pluginManager	= new PluginManager();;
	documentList	= new BList();
	configWindow	= NULL;
	be_app->GetAppInfo(&info); 
    BEntry entry(&info.ref); 
    entry.GetPath(&path); 
    path.GetParent(&path);
    //in das Unterverzeichniss "Plugins wechseln"
	path.Append("Plugins");
	BDirectory *dir	= new BDirectory(path.Path());
	pluginManager->LoadPlugins(dir);
}

void PDocumentManager::Init(BMessage *archive)
{
	BMessage	*docMessage	= new BMessage();
	PDocument	*doc		= NULL;
	int32		i			= 0;
	Init();
	while (archive->FindMessage("document",i,docMessage) == B_OK)
	{
		doc = new PDocument(docMessage); 
		documentList->AddItem(doc);
		i++	;
	}
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
 
status_t PDocumentManager::Archive(BMessage *archive, bool deep) const
{
	BMessage	docArchive;
	status_t	err = B_OK;
	err = BArchivable::Archive(archive, deep);
	for (int32 i; i < documentList->CountItems(); i++)
	{
		if (((PDocument*)documentList->ItemAt(i))->Archive(&docArchive, deep) == B_OK )
			err = err | archive->AddMessage("PDocumentManager::document", &docArchive);
	}
	err = err | archive->AddString("class", "PDocumentManager"); 
	if (err != B_OK)
		return B_ERROR;
	else
		return B_OK;
} 

BArchivable *PDocumentManager::Instantiate(BMessage *archive)
{
	if ( validate_instantiation(archive, "PDocumentManager"))
		return new PDocumentManager(archive);
	else
	   	return NULL;
} 

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

//
PDocument* PDocumentManager::CreateDocument(void)
{
	TRACE();
	PDocument *doc = new PDocument(this);
	AddDocument(doc);
	return doc;
}
ConfigWindow *PDocumentManager::GetConfigWindow(void)
{
	if (configWindow == NULL)
		configWindow = new ConfigWindow(NULL);
	return configWindow;
}
