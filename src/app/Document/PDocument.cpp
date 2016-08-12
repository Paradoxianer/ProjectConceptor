#include <interface/Alert.h>
#include <interface/PrintJob.h>
#include <interface/Screen.h>
#include <support/Debug.h>
#include <support/String.h>
#include <storage/NodeInfo.h>
#include <TranslationUtils.h>
#include <TranslatorRoster.h>
#include <string.h>
#include <FindDirectory.h>
#include <Catalog.h>


#include "BasePlugin.h"
#include "ConfigManager.h"
#include "Indexer.h"
#include "PCSavePanel.h"
#include "PDocument.h"
#include "PDocLoader.h"
#include "PDocumentManager.h"
#include "PluginManager.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Document"

#ifdef __BEOS__
 #ifdef find_directory
 	#undef find_directory
 #endif
#endif

PDocument::PDocument(PDocumentManager *initManager):BLooper()
{
	TRACE();
	documentManager=initManager;
	Init();
	Run();
}

PDocument::PDocument(PDocumentManager *initManager,entry_ref *openEntry):BLooper()
{
	TRACE();
	PRINT(("%s\n",openEntry->name));
	documentManager=initManager;
	Run();
	Init();
}

PDocument::PDocument(BMessage *archive):BLooper(archive)
{
	TRACE();
	Run();
	Init(archive);
}

PDocument::~PDocument()
{
	TRACE();
	documentManager->RemoveDocument(this);
}

status_t PDocument::Archive(BMessage* archive, bool deep) const
{
	TRACE();
	BLooper::Archive(archive, deep);
	Indexer		*indexer				= new Indexer((PDocument *)this);
	int32		i						= 0;
	BMessage	*commandManage			= new BMessage();
	BMessage	*tmpNode				= NULL;
	BMessage	*allNodesMessage		= new BMessage();
	BMessage	*allConnectionsMessage	= new BMessage();
	BMessage	*selectedMessage		= new BMessage();

	archive->AddMessage("PDocument::documentSetting",documentSetting);
	//save all Nodes
	for (i=0; i<allNodes->CountItems();i++) {
		tmpNode=(BMessage *)allNodes->ItemAt(i);
		allNodesMessage->AddMessage("node",indexer->IndexNode(tmpNode));
	}
	archive->AddMessage("PDocument::allNodes",allNodesMessage);
	//save all Connections
	for (i=0; i<allConnections->CountItems();i++) {
		tmpNode=(BMessage *)allConnections->ItemAt(i);
		allConnectionsMessage->AddMessage("node",indexer->IndexConnection(tmpNode));
	}
	archive->AddMessage("PDocument::allConnections",allConnectionsMessage);
	//save the selected List
	for (i=0; i<selected->CountItems();i++) {
		selectedMessage->AddPointer("node",selected->ItemAt(i));
	}
	archive->AddMessage("PDocument::selected",selectedMessage);
	//save all Command related Stuff like Undo/Makor
//	commandManager->Archive(commandManage);
	for (i=0;i<(commandManager->GetUndoList())->CountItems();i++) {
		commandManage->AddMessage("undo",indexer->IndexMacroCommand((BMessage *)(commandManager->GetUndoList())->ItemAt(i)));
	}
	for (i=0;i<(commandManager->GetMacroList())->CountItems();i++) {
		commandManage->AddMessage("macro",(BMessage *)(commandManager->GetMacroList())->ItemAt(i));

	}
	commandManage->AddInt32("undoStatus",commandManager->GetUndoIndex());
	archive->AddMessage("PDocument::commandManager", commandManage);

	delete	indexer;
	delete	commandManage;
	//delete	tmpNode;
	delete	allNodesMessage;
	delete	allConnectionsMessage;
	delete	selectedMessage;

	return B_OK;
}

BArchivable* PDocument::Instantiate(BMessage* message)
{
	TRACE();
	if (!validate_instantiation(message, "PDocument"))
		return NULL;
	else
		return new PDocument(message);
}

void PDocument::MessageReceived(BMessage* message) {
	TRACE();
	switch(message->what) {
		case MENU_FILE_SAVE: {
			Save();
			break;
		}
		case MENU_FILE_SAVEAS: {
			SavePanel();
			break;
		}
		case MENU_FILE_PRINT: 	{
			Print();
			break;
		}
		case MENU_APP_SETTINGS:
				ShowSettings();
			break;
		case MENU_MACRO_START_RECORDING:{
			commandManager->StartMacro();
			break;
		}
		case MENU_MACRO_STOP_RECORDING:{
			commandManager->StopMacro();
			break;
		}
		case P_C_MACRO_TYPE: {
			commandManager->PlayMacro(message);
			break;
		}
		case  B_SAVE_REQUESTED: {
				BMessage	*saveSettings	= new BMessage();
				message->FindMessage("saveSettings",saveSettings);
				entry_ref *ref	= new entry_ref;
				const char* name;
				message->FindRef("directory",ref);
				message->FindString("name", &name);
				SetEntry(ref,name);
				if (documentSetting->ReplaceMessage("saveSettings",saveSettings) != B_OK)
					documentSetting->AddMessage("saveSettings",saveSettings);
				Save();
			break;
		}
		case B_COPY: {
			BMessage	*copyMessage	= new BMessage(P_C_EXECUTE_COMMAND);
			copyMessage->AddString("Command::Name","Copy");
			copyMessage->AddBool("shadow",true);
			commandManager->Execute(copyMessage);
			break;
		}
		case B_PASTE: {
			BMessage	*pasteMessage	= new BMessage(P_C_EXECUTE_COMMAND);
			pasteMessage->AddString("Command::Name","Paste");
			pasteMessage->AddBool("shadow",true);
			commandManager->Execute(pasteMessage);
			break;
		}

		case B_UNDO: {
			commandManager->Undo(NULL);
			break;
		}
		case B_REDO: {
			commandManager->Redo(NULL);
			break;
		}
		case P_C_EXECUTE_COMMAND: {
			commandManager->Execute(message);
			break;
		}
		case P_C_AUTO_SAVE: {
			AutoSave();
			break;
		}
		case P_C_RESTORE_SAVE: {
			//for the Moment we use autosave ;-)
			AutoSave();
			break;
		}
		case MENU_SEARCH_FIND:
		{
			BString		*searchString	= new BString();
			BMessage	*searchMessage	= new BMessage(P_C_EXECUTE_COMMAND);
			searchMessage->AddString("Command::Name","Find");
			if (message->FindString("searchString",searchString)==B_OK)
				searchMessage->AddString("searchString",*searchString);
		//	searchMessage->AddBool("shadow",true);
			commandManager->Execute(searchMessage);
			break;
		}

		default:
			BLooper::MessageReceived(message);
			break;
	}

}

void PDocument::Init(){
	TRACE();
	bool locked = Lock();
	allNodes		= new BList();
	allConnections	= new BList();
	selected		= new BList();
	valueChanged	= new set<BMessage*>();
	//Load Configs
	printerSetting	= NULL;
	documentSetting	= new BMessage();
	bounds			= BRect(0,0,600,800);
	printableRect	= NULL;
	paperRect		= NULL;
	savePanel		= NULL;
	width			= 600;
	height			= 800;
	dirty			= true;
	entryRef		= NULL;
	autoSaveRef		= NULL;
	modified		= false;
	//set autosave to 5 minutes for the beginning 5 * 60 = 300 *1000 (miliisek) *1000 =
	autoSaveIntervall	= 300000000;
	editorManager	= new PEditorManager(this);
	commandManager	= new PCommandManager(this);
	//** to do.. helpManager		= new HelpManager();
	BScreen *tmpScreen	= new BScreen();
	BRect	windowRect	= tmpScreen->Frame();
	windowRect.InsetBy(50,50);
	window			= new PWindow(windowRect,this);
	autoSaver		= new BMessageRunner(BMessenger(this),
										 new BMessage(P_C_AUTO_SAVE),
										 autoSaveIntervall);
	if (locked)
		UnlockLooper();
}

void PDocument::Init(BMessage *archive)
{
	TRACE();
	bool locked = Lock();
	printerSetting	= NULL;
	documentSetting	= new BMessage();
	archive->FindMessage("PDocument::documentSetting",documentSetting);
	if (documentSetting->FindMessage("Printer Setting",printerSetting) != B_OK) {
		delete printerSetting;
		printerSetting = NULL;
	}
	if (locked)
		Unlock();
}

const char* PDocument::Title(void)
{
	TRACE();
	if (Lock()) {
		if (entryRef)
			return entryRef->name;
		else
			return "Untitled";
		Unlock();
	}
	else {
		PRINT(("\tDEBUG:\tTitle() - Cant Lock()\n"));
		return NULL;
	}
}

void PDocument::Resize(float toX,float toY)
{
	TRACE();
	bool locked = Lock();
	bounds.right	= toX;
	bounds.bottom	= toY;
	editorManager->BroadCast(new BMessage(P_C_DOC_BOUNDS_CHANGED));
	if (locked)
		Unlock();
}

void PDocument::SetModified(void)
{
	bool locked = Lock();
	BMenuItem *saveItem = GetMenuItem(P_MENU_FILE_SAVE);
	if ((saveItem) && (entryRef != NULL))
		saveItem->SetEnabled(true);
	modified=true;
	if (locked)
		Unlock();

}

void PDocument::ResetModified(void)
{
	bool locked = Lock();
	BMenuItem *saveItem = GetMenuItem(P_MENU_FILE_SAVE);
	if ((saveItem) && (entryRef != NULL))
		saveItem->SetEnabled(false);
	modified=false;
	if (locked)
		Unlock();
}

void PDocument::SetDocumentSettings(BMessage *settings)
{
	TRACE();
	bool locked = Lock();
	if (settings!=NULL) {
		delete documentSetting;
		documentSetting	= settings;
	}
	if (locked)
		Unlock();

}

void PDocument::ShowSettings(void)
{
	bool locked = Lock();
	ConfigWindow *configWin = documentManager->GetConfigWindow();
	configWin->SetConfigMessage(documentSetting);
	configWin->Show();
	if (locked)
		Unlock();
}


BMessage* PDocument::PrintSettings(void)
{
	TRACE();
	status_t result = B_OK;
	BPrintJob printJob(Title());
	if (! printerSetting) {
		result = printJob.ConfigPage();
		if ((result == B_OK) && (Lock())) {
			SetPrintSettings (printJob.Settings());
			paperRect 		= new BRect(printJob.PaperRect());
			printableRect	= new BRect(printJob.PrintableRect());
			SetModified();
			Unlock();
		}
	}
	return printerSetting;
}

void PDocument::SetPrintSettings(BMessage *settings)
{
	TRACE();
	if (Lock()){
		if (settings!=NULL) {
			BMessage	*tmpMessage	= new BMessage();
			if (documentSetting->FindMessage("Printer Setting",tmpMessage) == B_OK)
				documentSetting->ReplaceMessage("Printer Setting",settings);
			else
				documentSetting->AddMessage("Printer Setting",settings);
			printerSetting	= settings;
			delete tmpMessage;
		}
		Unlock();
	}
}


void PDocument::Print(void)
{
	TRACE();
	status_t	result	= B_OK;
	BPrintJob	printJob("doc_name");
	if (Lock()) {
		if (printerSetting == NULL) {
				result = printJob.ConfigPage();
				if (result == B_OK) {
					// Get the user Settings
					SetPrintSettings(printJob.Settings());
					//	 Use the new settings for your internal use
					paperRect = new BRect(printJob.PaperRect());
					printableRect = new BRect(printJob.PrintableRect());
				}
			}
			// Setup the driver with user settings
			printJob.SetSettings(printerSetting);

			result = printJob.ConfigJob();
			if (result == B_OK) {
				// WARNING: here, setup CANNOT be NULL.
	         if (printerSetting == NULL) {
				(new BAlert(B_TRANSLATE("Error"),B_TRANSLATE("I had problems while setting up the Printer!"),"Ohh"))->Go();
	         }
			//**dont know why this was here :-)      		 delete printerSetting;
			// Get the user Settings
			SetPrintSettings( printJob.Settings());
			paperRect 		= new BRect(printJob.PaperRect());
			printableRect	= new BRect(printJob.PrintableRect());

			// Now you have to calculate the number of pages
			// (note: page are zero-based)
			int32 firstPage = printJob.FirstPage();
			int32 lastPage = printJob.LastPage();

			// Verify the range is correct
			// 0 ... LONG_MAX -> Print all the document
			// n ... LONG_MAX -> Print from page n to the end
			// n ... m -> Print from page n to page m
			PEditor *activePEditor=editorManager->GetActivPEditor();
			if (activePEditor != NULL) {
				BView *editorView=activePEditor->GetView();
				if (editorView != NULL) {
					// calculate the Number of Pages
					bool locked	= editorView->LockLooper();
					int32 xPages,yPages;
					BRect editorRect = editorView->Bounds();
					xPages=editorRect.IntegerWidth()/printableRect->IntegerWidth();
					if (editorRect.IntegerWidth()%printableRect->IntegerWidth() > 0)
						xPages++;
					yPages=editorRect.IntegerHeight()/printableRect->IntegerHeight();
					if (editorRect.IntegerHeight()% printableRect->IntegerHeight() > 0)
						yPages++;
					int32 document_last_page= xPages*yPages;
					if (lastPage > document_last_page)
						lastPage = document_last_page;
					int32 nbPages = lastPage - firstPage + 1;
					if (nbPages <= 0)
						;
						//**return B_ERROR;
					// Now we can print the page
					printJob.BeginJob();
					// Print all pages
					bool can_continue = printJob.CanContinue();
					for (int i=firstPage-1; can_continue && i<lastPage ; i++) {
						float xStart	= (i%xPages)*printableRect->Width();
						float yStart	= (i/xPages)*printableRect->Height();
						BRect viewRect(xStart,yStart,xStart+printableRect->Width(),yStart+printableRect->Height());
//						printJob.DrawView(editorView, viewRect, BPoint(0,0));
						printJob.DrawView(editorView, viewRect, BPoint(printableRect->left,printableRect->top));
						printJob.SpoolPage();
						/*if (user_has_canceled)
						{
		       	        // tell the print_server to cancel the printjob
   	 	   	        	printJob.CancelJob();
							can_continue = false;
							break;
   	         		}*/
						can_continue = printJob.CanContinue();
					}
					if (can_continue)
						printJob.CommitJob();
					else
						result = B_ERROR;
					if (locked) editorView->UnlockLooper();
				}
			}
			else{
				(new BAlert(B_TRANSLATE("Error"),B_TRANSLATE("No active Editor Pleas install a Editor Plugin."),"Ohh"))->Go();
			}
		}
		Unlock();
	}
}


void PDocument::SetEntry(entry_ref *saveEntry,const char *name)
{
	TRACE();
	status_t err	= B_OK;
	if (Lock()) {
		if (saveEntry!=NULL) {
			BEntry entry;
			if (name != NULL){
				BDirectory dir(saveEntry);
				err = entry.SetTo(&dir, name);
			}
			else
				err = entry.SetTo(saveEntry);
			if (err == B_OK){
				if (entryRef == NULL)
						entryRef= new entry_ref;
				entry.GetRef(entryRef);
			}
			else
				entryRef == NULL;
		}
		Unlock();
	}
	else {
		new BAlert(B_TRANSLATE("Error"),B_TRANSLATE("Problems while trying to save the file"),"Ohh");
	}

}

void PDocument::Save(void)
{
	TRACE();
	//we ned to mkae this think blocking somehowe if quit is reuqw
	if (entryRef == NULL)
		SavePanel();
	status_t err 				= B_OK;
	BTranslatorRoster	*roster				= BTranslatorRoster::Default();
	BMessage			*archived			= new BMessage();
	BMessage			*saveSettings		= new BMessage();
	char				*formatName			= NULL;
	translator_info		*translatorInfo		= new translator_info;
	int32				tmpInt				= 0;
	int32				outType				= 0;

	bool locked = Lock();
	documentSetting->FindMessage("saveSettings",saveSettings);
	if (saveSettings->FindInt32("translator_id",&tmpInt) == B_OK){
		translatorInfo->translator	= tmpInt;
		saveSettings->FindString("format::name",(const char**)&formatName);
		//saveSettings->FindString("format::MIME",(const char**)&formatMIME);
		strcpy((translatorInfo->name),formatName);
		strcpy((translatorInfo->MIME),P_C_DOCUMENT_MIMETYPE);
		saveSettings->FindInt32("format::type",(int32 *)&outType);
		translatorInfo->type				= P_C_DOCUMENT_RAW_TYPE;
//		saveSettings->FindInt32("format::group",(int32 *)&tmpInt);
		translatorInfo->group				= P_C_DOCUMENT_RAW_TYPE;
//		saveSettings->FindFloat("format::quality",(float *)&tmpFloat);
		translatorInfo->quality				= 0.9;
//		saveSettings->FindFloat("format::capability",(float *)&tmpFloat);
		translatorInfo->capability			= 0.9;
		Archive(archived,true);
		BPositionIO		*input	= new BMallocIO();
		BFile			*file	= new BFile(entryRef,B_WRITE_ONLY | B_ERASE_FILE | B_CREATE_FILE);
		archived->Flatten(input);
		err=	roster->Translate(input,translatorInfo,NULL,file,outType);
		if (err == B_OK) {
			BNodeInfo nodeInfo(file);
			nodeInfo.SetType(P_C_DOCUMENT_MIMETYPE);
			nodeInfo.SetPreferredApp(APP_SIGNATURE);
		}
	}
	else {
		Archive(archived,true);
		if (entryRef) {
			BFile *file=	new BFile(entryRef,B_WRITE_ONLY | B_ERASE_FILE | B_CREATE_FILE);
			err=file->InitCheck();
			PRINT(("ERROR\tSave file error %s\n",strerror(err)));
			err = archived->Flatten(file);
			if (err == B_OK) {
				BNodeInfo nodeInfo(file);
				nodeInfo.SetType(P_C_DOCUMENT_MIMETYPE);
				nodeInfo.SetPreferredApp(APP_SIGNATURE);
			}
		}
	}
	if (err==B_OK) {
			ResetModified();
			window->SetTitle(Title());
	}
	else
		PRINT(("ERROR:\tPDocument -Save error %s\n",strerror(err)));
	if (locked)
		Unlock();
}

void PDocument::Load(void)
{
	TRACE();
	status_t			err				= B_OK;
	BFile				*file			= new BFile(entryRef,B_READ_ONLY);
	BMessage			*node			= NULL;
	BTranslatorRoster	*roster			= NULL;
	BMallocIO			*output			= new BMallocIO();
	BMessage			*loaded			= new BMessage();
	int32				i				= 0;
	translator_info		*indentifed		= new translator_info;
	bool locked = Lock();

	if (file->InitCheck() == B_OK) {
		roster	= BTranslatorRoster::Default();
		roster->Identify(file,NULL,indentifed,P_C_DOCUMENT_RAW_TYPE );
		err 	= roster->Translate(file,indentifed,NULL,output,P_C_DOCUMENT_RAW_TYPE);
		//buffer = (void *)output->Buffer();
		if (err == B_OK) {
			err = loaded->Unflatten(output);
			if (err != B_OK)
				printf("Loading error: %s\n",strerror(err));
			ResetModified();
		}
	}
	else
	 //**error handling
	;
	//docloader handles the Format and Stuff also the input translation
	PDocLoader	*docLoader	= new PDocLoader(this,loaded);
	delete allNodes;
	delete allConnections;
	delete printerSetting;
	delete selected;

	allNodes 		= docLoader->GetAllNodes();
	for (i = 0; i<allNodes->CountItems(); i++) {
		node=((BMessage*)allNodes->ItemAt(i));
		node->AddPointer("ProjectConceptor::doc",this);
		valueChanged->insert(node);
	}
	allConnections	= docLoader->GetAllConnections();
	for (i = 0; i<allConnections->CountItems(); i++) {
		node= (BMessage *)allConnections->ItemAt(i);
		node->AddPointer("ProjectConceptor::doc",this);
		valueChanged->insert(node);
	}
	selected = docLoader->GetSelectedNodes();
	delete commandManager;
	commandManager= new PCommandManager(this);
	commandManager->SetMacroList(docLoader->GetMacroList());
	commandManager->SetUndoList(docLoader->GetUndoList());
	commandManager->SetUndoIndex(docLoader->GetUndoIndex());
//	commandManager->LoadMacros(docLoader->GetCommandManagerMessage());
//	commandManager->LoadUndo(docLoader->GetCommandManagerMessage());
	SetPrintSettings( docLoader->GetPrinterSetting());
	editorManager->BroadCast(new BMessage(P_C_VALUE_CHANGED));
	if (locked)
		Unlock();
}

void PDocument::SavePanel()
{
	TRACE();
	if (!savePanel) {
//		savePanel = new PCSavePanel(B_SAVE_PANEL, new BMessenger(this),NULL,0,false);
		savePanel = new PCSavePanel(NULL,new BMessenger(this));
	}
	if (entryRef) savePanel->SetPanelDirectory(entryRef);
	savePanel->Window()->SetWorkspaces(B_CURRENT_WORKSPACE);
	savePanel->Show();
}


bool PDocument::QuitRequested(void)
{
	TRACE();
	//check modified if there are changes wich we need to save
	bool	returnValue = true;
	bool	readLock	= false;
	if (modified) {
		readLock = Lock();
		BString saveText = BString(B_TRANSLATE("Save changes to  "));
		saveText.Append(Title());
		BAlert *myAlert = new BAlert(B_TRANSLATE("Save before close"),  saveText.String(), B_TRANSLATE("Cancel"), B_TRANSLATE("Don't save"), B_TRANSLATE("Save"), B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
		myAlert->SetShortcut(0, B_ESCAPE);
		int32 button_index = myAlert->Go();
		if (button_index == 0)
			returnValue = false;
		else if (button_index == 1)
			returnValue =  true;
		else {
			//check if there is a ref set if not show a Save file dialog
			Save();
			Unlock();
			returnValue	= true;
		}
		if (readLock)
			Unlock();
	}
	if (returnValue == true) {
		window->Lock();
		window->Quit();
	}
	return returnValue;
}

BMessage* PDocument::FindObject(BPoint *where)
{
	TRACE();
	BRect		*frame	= new BRect(-10,-10,-10,-10);
	BMessage	*node	= NULL;
	bool		found	= false;
	for (int32 i=0;( (i<allNodes->CountItems())&&(!found) );i++) {
		node=(BMessage *)allNodes->ItemAt(i);
		node->FindRect("Node::frame",frame);
		if (frame->Contains(*where))
			found=true;
	}
	if (found)
		return node;
	else
		return NULL;
}

BMenu *PDocument::GetMenu(const char* signatur)
{
	BMenu *alleMenus	= NULL;
	if (window)
		alleMenus=window->GetMenu(signatur);
	return alleMenus;
}

BMenuItem*	PDocument::GetMenuItem(const char* signatur)
{
	BMenuItem *alleMenuItems	= NULL;
	if(window)
		alleMenuItems=window->GetMenuItem(signatur);
	return alleMenuItems;
}

ToolBar* PDocument::GetToolBar(const char *signatur)
{
	ToolBar *alleToolBars		= NULL;
	if(window)
		alleToolBars=window->GetToolBar(signatur);
	return alleToolBars;
}

ToolMenu* PDocument::GetToolMenu(const char* toolbarSignature,const char *signature)
{
	ToolMenu *alleToolMenus	= NULL;
	if(window)
		alleToolMenus=window->GetToolMenu(toolbarSignature,signature);
	return alleToolMenus;
}

ToolItem* PDocument::GetToolItem(const char* toolbarSignature,const char *signature)
{
	ToolItem *alleToolItems	= NULL;
	if(window)
		alleToolItems=window->GetToolItem(toolbarSignature,signature);
	return alleToolItems;
}

status_t PDocument::AddMenu(BMenu *menu,int32 index)
{
	status_t 	err	= B_OK;
	if (window)
		err = err || window->AddMenu(menu,index);
	return err;
}

status_t PDocument::AddMenuItem(const char *menuSignatur, BMenuItem *menuItem,int32 index)
{
	status_t 	err	= B_OK;
	if(window)
		err = err || window->AddMenuItem(menuSignatur,menuItem,index);
	return err;
}

status_t PDocument::AddToolBar(ToolBar *toolbar)
{
	status_t 	err	= B_OK;
	if(window)
		err = err || window->AddToolBar(toolbar);
	return err;
}

status_t PDocument::AddToolMenu(const char *toolbarSignatur,ToolMenu *toolMenu)
{
	status_t 	err	= B_OK;
	if(window)
		err = err || window->AddToolMenu(toolbarSignatur,toolMenu);
	return err;
}

status_t PDocument::AddToolItem(const char *toolbarSignatur,const char *toolmenuSignatur,ToolItem *toolItem,int32 index)
{
	status_t 	err	= B_OK;
	if (window)
		err = err || window->AddToolItem(toolbarSignatur,toolmenuSignatur,toolItem,index);
	return err;
}

status_t PDocument::RemoveMenu(const char* menuSignature)
{
	status_t 	err	= B_OK;
	if(window)
		err = err || window->RemoveMenu(menuSignature);
	return err;
}

status_t PDocument::RemoveMenuItem(const char* menuItemSignature)
{
	status_t 	err	= B_OK;
	if(window)
		err = err || window->RemoveMenuItem(menuItemSignature);
	return err;
}

status_t PDocument::RemoveToolBar(const char* signature)
{
	status_t 	err	= B_OK;
	if(window)
		err = err || window->RemoveToolBar(signature);
	return err;
}

status_t PDocument::RemoveToolMenu(const char* toolbarSignature,const char* toolmenuSignature)
{
	status_t 	err	= B_OK;
	if(window)
		err = err || window->RemoveToolMenu(toolbarSignature,toolmenuSignature);
	return err;
}

status_t PDocument::RemoveToolItem(const char* toolbarSignature,const char* toolitemSignature)
{
	status_t 	err	= B_OK;
	if(window)
		err = err || window->RemoveToolItem(toolbarSignature,toolitemSignature);
	return err;
}


void PDocument::AutoSave(void)
{
	TRACE();
	//** Use Settingsfolder to
	if (autoSaveRef == NULL) {
		BPath settings;
		find_directory(B_USER_SETTINGS_DIRECTORY, &settings, true);
		char p[PATH_MAX];
		sprintf(p,"%s/ProjectConceptor/AutoSave/%s",settings.Path(),Title());
		//** check if this file exist already
		printf("autosave to %s\n",p);
		autoSaveRef=new BEntry(p);
	}
	BFile	*autoSave	= new BFile((const BEntry *)autoSaveRef,B_READ_WRITE | B_ERASE_FILE | B_CREATE_FILE);
	PushToStream(autoSave);
	delete autoSave;
}

void PDocument::PushToStream(BPositionIO *pushTo) {
	Indexer		*indexer			= new Indexer((PDocument *)this);
	BMessage	*tmpNode			= NULL;
	BMessage	*commandManage		= new BMessage();
	BMessage	*selectedMessage	= new BMessage();
	int			i					= 0;
	//**security check if the passed BPositionIO ok is
	documentSetting->Flatten(pushTo);
	for (i=0; i<allNodes->CountItems();i++) {
		tmpNode=(BMessage *)allNodes->ItemAt(i);
		BMessage *indexed = indexer->IndexNode(tmpNode);
		indexed->Flatten(pushTo);
	}
	for (i=0; i<allConnections->CountItems();i++) {
		tmpNode=(BMessage *)allConnections->ItemAt(i);
		BMessage *indexed = indexer->IndexConnection(tmpNode);
		indexed->Flatten(pushTo);
	}
	for (i=0; i<selected->CountItems();i++) {
		selectedMessage->AddPointer("node",selected->ItemAt(i));
	}
	selectedMessage->Flatten(pushTo);
	for (i=0;i<(commandManager->GetMacroList())->CountItems();i++) {
			BMessage *macro =(BMessage *)(commandManager->GetMacroList()->ItemAt(i));
			macro->Flatten(pushTo);
	}
	for (i=0;i<(commandManager->GetUndoList())->CountItems();i++) {
		BMessage *indexed = indexer->IndexMacroCommand((BMessage *)(commandManager->GetUndoList()->ItemAt(i)));
		indexed->Flatten(pushTo);
	}
	//**add the UndoIndex
	commandManage->AddInt32("undoStatus",commandManager->GetUndoIndex());
	//add the commandManage
	commandManage->Flatten(pushTo);
	delete	indexer;
	delete	commandManage;
	delete	selectedMessage;
}
