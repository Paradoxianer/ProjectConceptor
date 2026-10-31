#include <app/Messenger.h>
#include <app/PropertyInfo.h>
#include <app/Roster.h>
#include <interface/Alert.h>
#include <interface/Bitmap.h>
#include <interface/PrintJob.h>
#include <interface/TextView.h>
#include <interface/Screen.h>
#include <interface/View.h>
#include <support/Autolock.h>
#include <support/Debug.h>
#include <support/String.h>
#include <storage/NodeInfo.h>
#include <BitmapStream.h>
#include <TranslationUtils.h>
#include <TranslatorRoster.h>
#include <string.h>
#include <algorithm>
#include <math.h>
#include <FindDirectory.h>
#include <Catalog.h>


#include "BasePlugin.h"
#include "ConfigManager.h"
#include "Indexer.h"
#include "PCommandManager.h"
#include "PCSavePanel.h"
#include "PDocument.h"
#include "PDocLoader.h"
#include "PDocumentManager.h"
#include "PEditor.h"
#include "PEditorManager.h"
#include "PluginManager.h"
#include "ProjectConceptor.h"


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

// headless is only there to distinguish this overload - no window, no
// editorManager, just enough state (allNodes/allConnections/commandManager)
// for Indexer/PCommand tests to run without an app_server. See docs/notes.md.
PDocument::PDocument(PDocumentManager *initManager,bool headless):BLooper()
{
	TRACE();
	documentManager=initManager;
	_InitData();
	commandManager	= new PCommandManager(this);
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

	archive->AddInt32(P_C_DOC_FORMAT_VERSION_FIELD,P_C_DOC_FORMAT_VERSION);
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
		selectedMessage->AddInt32("node",indexer->IdFor((BMessage *)selected->ItemAt(i)));
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
		case MENU_FILE_PAGESETUP: {
			PageSetup();
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
			// interactive - PlayMacro() itself stays synchronous for
			// tests/programmatic callers, this spreads a GUI-triggered play
			// across one dispatch per command so the editors can redraw
			// after each step
			commandManager->PlayMacroInteractive(message);
			break;
		}
		case P_C_MACRO_PLAY_STEP: {
			commandManager->PlayMacroStep();
			break;
		}
		// Both forward the menu's own message as-is to whichever
		// registered editor's view is named "MacroEditor" - found by
		// view name only (not a dynamic_cast to a concrete plugin type),
		// so this core file never needs to #include a plugin header. The
		// actual Save/Open logic (which macro, what "no macro selected"
		// means) lives entirely in MacroEditor - see its MessageReceived()
		// (#55 follow-up: these were empty stubs before MacroEditor gave
		// "Save"/"Open" something concrete to mean).
		//
		// SelectEditorTab() first: a non-active tab's view has no Looper
		// in this app's MainView (confirmed live - only the selected tab
		// stays attached, see CreatEditorList()'s own comment on this),
		// so sending to it before switching to it silently goes nowhere.
		case MENU_MACRO_SAVE:
		case MENU_MACRO_NEW:
		case MENU_MACRO_RENAME:
		case MENU_MACRO_DUPLICATE:
		case MENU_MACRO_DELETE:
		case MENU_MACRO_RUN_ON_OPEN:
		case MENU_MACRO_OPEN: {
			if ((window != NULL) && (editorManager != NULL)) {
				window->SelectEditorTab("MacroEditor");
				for (int32 i = 0; i < editorManager->CountPEditors(); i++) {
					PEditor	*editor	= editorManager->PEditorAt(i);
					BView	*view	= (editor != NULL) ? editor->GetView() : NULL;
					if ((view != NULL) && (view->Name() != NULL) &&
							(strcmp(view->Name(),"MacroEditor") == 0)) {
						BMessenger(editor->GetHandler()).SendMessage(message);
						break;
					}
				}
			}
			break;
		}
		case  B_SAVE_REQUESTED: {
				BMessage	saveSettings;
				message->FindMessage("saveSettings",&saveSettings);
				entry_ref	directory;
				const char	*name	= NULL;
				if ((message->FindRef("directory",&directory) != B_OK)
					|| (message->FindString("name",&name) != B_OK))
					break;
				SetEntry(&directory,name);
				if (documentSetting->ReplaceMessage("saveSettings",&saveSettings) != B_OK)
					documentSetting->AddMessage("saveSettings",&saveSettings);
				Save();
			break;
		}
		case MENU_FILE_EXPORT: {
			ExportPanel();
			break;
		}
		case P_C_EXPORT_REQUESTED: {
			// a copy: the document keeps its own file and format
			BMessage	saveSettings;
			entry_ref	directory;
			const char	*name	= NULL;
			if ((message->FindMessage("saveSettings",&saveSettings) != B_OK)
				|| (message->FindRef("directory",&directory) != B_OK)
				|| (message->FindString("name",&name) != B_OK))
				break;
			BDirectory	dir(&directory);
			BEntry		entry(&dir,name);
			entry_ref	ref;
			if (entry.GetRef(&ref) == B_OK)
				WriteTo(ref,saveSettings,true);
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

		// the Edit menu item has always sent this straight here, but nothing
		// ever handled it - Select all silently did nothing. The Select
		// command's own selectAll path is what does the work; no "deselect"
		// field is added on purpose, so the command clears the old selection
		// first (its default when the field is absent).
		case B_SELECT_ALL: {
			// with a text field focused (the MacroEditor's, say), Alt+A means
			// "select the text" - it used to run the graph's Select all
			// instead, whose broadcast made the MacroEditor reload its
			// macro over whatever was being typed (user report: "Alt+A
			// deletes everything").
			if ((window != NULL) && (window->LockLooperWithTimeout(100000) == B_OK)) {
				BTextView	*focusText	= dynamic_cast<BTextView*>(window->CurrentFocus());
				BMessenger	target(focusText);
				window->UnlockLooper();
				if (focusText != NULL) {
					target.SendMessage(B_SELECT_ALL);
					break;
				}
			}
			BMessage	*selectAllMessage	= new BMessage(P_C_EXECUTE_COMMAND);
			selectAllMessage->AddString("Command::Name","Select");
			selectAllMessage->AddBool("selectAll",true);
			commandManager->Execute(selectAllMessage);
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
		// A resolved scripting request for the "suite/vnd.ProjectConceptor-
		// command" suite (see ResolveSpecifier()/GetSupportedSuites() below,
		// #55) - "property" is the command name (e.g. "Move" from
		// `hey $SIG DO Move of ... with dx=float(10) and dy=float(0)`).
		// Every other top-level field on `message` is the scripting
		// client's own command data and is forwarded as-is; only the
		// BMessage-internal specifier bookkeeping fields are skipped.
		case B_EXECUTE_PROPERTY: {
			// scripting clients that send with a reply pointer (`hey`'s own
			// "DO" verb does - target->SendMessage(&the_message, reply))
			// block waiting for one - confirmed live: `hey $SIG DO Sleep of
			// Document 0 with ...` hung indefinitely before this, nothing
			// here ever called message->SendReply(). Every return path
			// below replies now, matching the standard scripting
			// convention (BTextView::_SetProperty is the real, shipping
			// example this mirrors: reply->what = B_REPLY; "error" = the
			// status).
			BMessage	reply(B_REPLY);
			const char	*property	= NULL;
			// GetCurrentSpecifier()'s own real implementation
			// (Message.cpp) only fills in `property` when `specifier`
			// (its own 2nd parameter) is non-NULL too - passing NULL for
			// it, as this did before, made every "DO <command> of
			// Document N" scripting call silently resolve `property` to
			// NULL while still returning B_OK, so "Command::Name" was
			// never actually set below; GetPCommand(NULL) then found
			// nothing and fell into Execute()'s own "unknown command"
			// branch, which pops a BAlert::Go() - blocking forever with
			// no one there to click it. Confirmed live (had to kill both
			// the stuck `hey` process and the app to recover).
			BMessage	specifier;
			status_t	specErr	= message->GetCurrentSpecifier(NULL,&specifier,NULL,&property);
			if (specErr == B_OK) {
				BMessage	*settings	= new BMessage(P_C_EXECUTE_COMMAND);
				settings->AddString("Command::Name",property);
				char		*fieldName;
				type_code	fieldType;
				int32		fieldCount;
				int32		i			= 0;
				while (message->GetInfo(B_ANY_TYPE,i,&fieldName,&fieldType,&fieldCount) == B_OK) {
					if ( (strcmp(fieldName,"specifiers") != 0) &&
							(strcmp(fieldName,"property") != 0) &&
							(strcmp(fieldName,"current_specifier") != 0) ) {
						for (int32 j = 0; j < fieldCount; j++) {
							const void	*data;
							ssize_t		size;
							if (message->FindData(fieldName,fieldType,j,&data,&size) == B_OK)
								settings->AddData(fieldName,fieldType,data,size);
						}
					}
					i++;
				}
				reply.AddInt32("error",commandManager->Execute(settings));
			} else
				reply.AddInt32("error",B_BAD_VALUE);
			message->SendReply(&reply);
			break;
		}
		case P_C_AUTO_SAVE: {
			AutoSave();
			break;
		}
		case P_C_DOC_SETTINGS_CHANGED: {
			ApplyAutoSaveSettings();
			break;
		}
		case P_C_PLAY_MACRO_BY_NAME: {
			const char	*macroName	= NULL;
			if (message->FindString(P_C_MACRO_SHORTCUT_NAME_FIELD,&macroName) == B_OK)
				commandManager->PlayMacroByName(macroName);
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
			// FindWindow's attribute comparison (#134)
			const char	*criterionFields[]	= { P_C_FIND_ATTRIBUTE,
				P_C_FIND_OPERATOR, P_C_FIND_VALUE };
			for (int32 i = 0; i < 3; i++) {
				const char	*field	= NULL;
				if (message->FindString(criterionFields[i],&field) == B_OK)
					searchMessage->AddString(criterionFields[i],field);
			}
			// forwarded, not hardcoded - this handler serves both
			// FindWindow's 'live' case (shadow=true, one per keystroke -
			// must never land in a recording) and its 'ok' case (no
			// "shadow" field at all - the actual confirmed search). This
			// was commented out, so every keystroke recorded a real,
			// permanent macro entry instead of the one final search -
			// confirmed live: typing a 4-letter search while recording
			// added 4 separate Find entries.
			bool	shadow	= false;
			if (message->FindBool("shadow",&shadow)==B_OK)
				searchMessage->AddBool("shadow",shadow);
			commandManager->Execute(searchMessage);
			break;
		}

		default:
			BLooper::MessageReceived(message);
			break;
	}

}


status_t PDocument::GetSupportedSuites(BMessage *data) {
	data->AddString("suites","suite/vnd.ProjectConceptor-command");
	BPropertyInfo	*propertyInfo	= commandManager->BuildPropertyInfo();
	data->AddFlat("messages",propertyInfo);
	delete propertyInfo;
	return BLooper::GetSupportedSuites(data);
}


BHandler* PDocument::ResolveSpecifier(BMessage *message, int32 index,
	BMessage *specifier, int32 what, const char *property) {
	BPropertyInfo	*propertyInfo	= commandManager->BuildPropertyInfo();
	status_t		match			= propertyInfo->FindMatch(message,index,specifier,what,property);
	delete propertyInfo;
	if (match >= 0)
		return this;
	return BLooper::ResolveSpecifier(message,index,specifier,what,property);
}

void PDocument::_InitData(void){
	TRACE();
	allNodes		= new BList();
	allConnections	= new BList();
	selected		= new BList();
	valueChanged	= new set<BMessage*>();
	//Load Configs
	printerSetting	= NULL;
	documentSetting	= new BMessage();
	bounds			= BRect(0,0,600,800);
	savePanel		= NULL;
	exportPanel		= NULL;
	width			= 600;
	height			= 800;
	dirty			= true;
	entryRef		= NULL;
	autoSaveRef		= NULL;
	modified		= false;
	autoSaver		= NULL;
	editorManager	= NULL;
	commandManager	= NULL;
	window			= NULL;
	bool	autoSaveEnabled		= true;
	if (documentSetting->FindBool(P_C_DOC_AUTOSAVE_ENABLED,&autoSaveEnabled) != B_OK)
		documentSetting->AddBool(P_C_DOC_AUTOSAVE_ENABLED,autoSaveEnabled);
	int32	autoSaveSeconds		= 300;
	if (documentSetting->FindInt32(P_C_DOC_AUTOSAVE_INTERVAL,&autoSaveSeconds) != B_OK)
		documentSetting->AddInt32(P_C_DOC_AUTOSAVE_INTERVAL,autoSaveSeconds);
}

void PDocument::Init(){
	TRACE();
	bool locked = Lock();
	_InitData();
	editorManager	= new PEditorManager(this);
	commandManager	= new PCommandManager(this);
	//** to do.. helpManager		= new HelpManager();
	BScreen *tmpScreen	= new BScreen();
	BRect	windowRect	= tmpScreen->Frame();
	windowRect.InsetBy(50,50);
	// restore the window frame remembered from the last time a window was
	// closed, if the user hasn't turned that off and it still falls within
	// the current screen (a stored frame from a since-disconnected second
	// monitor would otherwise open the window off-screen)
	ConfigManager	*configManager	= ((ProjektConceptor*)be_app)->GetConfigManager();
	BMessage		*frameConfig	= configManager->GetConfigMessage(P_C_CONFIG_WINDOW_FRAME_FIELD);
	if (frameConfig != NULL) {
		bool	rememberEnabled	= true;
		BRect	rememberedFrame;
		if ( (frameConfig->FindBool(P_C_WINDOW_FRAME_REMEMBER_FIELD,&rememberEnabled) != B_OK || rememberEnabled)
			&& (frameConfig->FindRect(P_C_WINDOW_FRAME_RECT_FIELD,&rememberedFrame) == B_OK)
			&& (tmpScreen->Frame().Contains(rememberedFrame.LeftTop())) )
			windowRect	= rememberedFrame;
		delete frameConfig;
	}
	window			= new PWindow(windowRect,this);
	UpdateWindowTitle();
	ApplyAutoSaveSettings();
	if (locked)
		UnlockLooper();
}

void PDocument::Init(BMessage *archive)
{
	TRACE();
	bool locked = Lock();
	printerSetting	= new BMessage();
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
	BAutolock	autolock(this);
	if (!autolock.IsLocked())
		return NULL;
	if (entryRef)
		return entryRef->name;
	if (importedName.Length() > 0)
		return importedName.String();
	return B_TRANSLATE("Untitled");
}

void PDocument::UpdateWindowTitle(void)
{
	const char	*title	= Title();
	if ((window == NULL) || (title == NULL))
		return;
	BMessage	setTitle(B_SET_PROPERTY);
	setTitle.AddSpecifier("Title");
	setTitle.AddString("data",title);
	BMessenger(window).SendMessage(&setTitle);
}

void PDocument::Resize(float toX,float toY)
{
	TRACE();
	bool locked = Lock();
	bounds.right	= toX;
	bounds.bottom	= toY;
	// a document with no attached UI (a headless PDocument, e.g.
	// NewHeadlessTestDocument() in the test suite) has no editorManager -
	// same null-deref pattern already fixed in PCommandManager (#117)
	if (editorManager != NULL)
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
	configWin->SetConfigMessage(documentSetting,this);
	configWin->Show();
	if (locked)
		Unlock();
}


void PDocument::PageSetup(void)
{
	TRACE();
	BPrintJob	printJob(Title());
	if (printerSetting != NULL)
		// the job owns what it gets
		printJob.SetSettings(new BMessage(*printerSetting));
	if (printJob.ConfigPage() != B_OK)
		return;
	SetPrintSettings(printJob.Settings());
	SetModified();
}


void PDocument::SetPrintSettings(BMessage *settings)
{
	TRACE();
	BAutolock	autolock(this);
	if (settings == NULL)
		return;
	if (documentSetting->ReplaceMessage("Printer Setting",settings) != B_OK)
		documentSetting->AddMessage("Printer Setting",settings);
	delete printerSetting;
	printerSetting	= settings;
}


// the editor to print: the active one, else the first that can
static PEditor* PrintableEditor(PEditorManager *editorManager)
{
	PEditor	*active	= editorManager->GetActivPEditor();
	for (int32 i = -1; i < editorManager->CountPEditors(); i++) {
		PEditor	*editor	= (i < 0) ? active : editorManager->PEditorAt(i);
		BView	*view	= (editor != NULL) ? editor->GetView() : NULL;
		if ((view == NULL) || !view->LockLooper())
			continue;
		bool	printable	= editor->PrintFrame().IsValid();
		view->UnlockLooper();
		if (printable)
			return editor;
	}
	return NULL;
}


void PDocument::Print(void)
{
	TRACE();
	PEditor	*editor	= PrintableEditor(editorManager);
	if (editor == NULL) {
		(new BAlert(B_TRANSLATE("Print"),B_TRANSLATE("There is nothing to print."),
			B_TRANSLATE("OK")))->Go(NULL);
		return;
	}
	BPrintJob	printJob(Title());
	if (printerSetting != NULL)
		// the job owns what it gets
		printJob.SetSettings(new BMessage(*printerSetting));
	else if (printJob.ConfigPage() != B_OK)
		return;
	if (printJob.ConfigJob() != B_OK)
		return;
	SetPrintSettings(printJob.Settings());

	BView	*view	= editor->GetView();
	if (!view->LockLooper())
		return;
	// the whole graph, tiled over as many pages as it needs
	BRect	frame		= editor->PrintFrame();
	BRect	printable	= printJob.PrintableRect();
	float	pageWidth	= floorf(printable.Width());
	float	pageHeight	= floorf(printable.Height());
	int32	columns		= (int32)ceilf((frame.Width() + 1) / pageWidth);
	int32	rows		= (int32)ceilf((frame.Height() + 1) / pageHeight);
	view->UnlockLooper();
	int32	firstPage	= std::max((int32)1, printJob.FirstPage());
	int32	lastPage	= std::min(columns * rows, printJob.LastPage());

	printJob.BeginJob();
	for (int32 page = firstPage; (page <= lastPage) && printJob.CanContinue(); page++) {
		int32	column	= (page - 1) % columns;
		int32	row		= (page - 1) / columns;
		BRect	part(frame.left + column * pageWidth, frame.top + row * pageHeight,
			frame.left + (column + 1) * pageWidth - 1,
			frame.top + (row + 1) * pageHeight - 1);
		// the point is relative to the printable rectangle
		printJob.DrawView(view, part, B_ORIGIN);
		printJob.SpoolPage();
	}
	if (printJob.CanContinue())
		printJob.CommitJob();
	else
		printJob.CancelJob();
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
				entryRef = NULL;
		}
		Unlock();
	}
	else {
		new BAlert(B_TRANSLATE("Error"),B_TRANSLATE("Problems while trying to save the file"),"Ohh");
	}

}

// The translator that reads a flattened document and writes outType.
static status_t FindSaveTranslator(uint32 outType, translator_id *_id)
{
	BTranslatorRoster	*roster			= BTranslatorRoster::Default();
	translator_id		*translators	= NULL;
	int32				count			= 0;
	status_t			err				= roster->GetAllTranslators(&translators,&count);
	if (err != B_OK)
		return err;
	err = B_NO_TRANSLATOR;
	for (int32 i = 0; (i < count) && (err != B_OK); i++) {
		const translation_format	*formats		= NULL;
		int32						formatCount		= 0;
		bool						readsDocument	= false;
		if (roster->GetInputFormats(translators[i],&formats,&formatCount) == B_OK) {
			for (int32 j = 0; j < formatCount; j++)
				readsDocument |= (formats[j].type == P_C_DOCUMENT_RAW_TYPE);
		}
		if (!readsDocument
			|| (roster->GetOutputFormats(translators[i],&formats,&formatCount) != B_OK))
			continue;
		for (int32 j = 0; j < formatCount; j++) {
			if (formats[j].type == outType) {
				*_id	= translators[i];
				err		= B_OK;
				break;
			}
		}
	}
	delete[] translators;
	return err;
}


void PDocument::Save(void)
{
	TRACE();
	if (entryRef == NULL) {
		// the panel's B_SAVE_REQUESTED sets the entry and calls Save() again
		SavePanel();
		return;
	}
	BMessage	saveSettings;
	documentSetting->FindMessage("saveSettings",&saveSettings);
	if (WriteTo(*entryRef,saveSettings) != B_OK)
		return;
	ResetModified();
	UpdateWindowTitle();
	be_roster->AddToRecentDocuments(entryRef,APP_SIGNATURE);
}


// the largest side of an exported picture, in pixels
static const float	kMaxBitmapSide	= 8000;

// the graph as a picture, through the system's bitmap translators
static status_t WriteBitmap(PEditorManager *editorManager, BPositionIO *out,
	uint32 type)
{
	PEditor	*editor	= PrintableEditor(editorManager);
	if (editor == NULL)
		return B_ERROR;
	BView	*view	= editor->GetView();
	if (!view->LockLooper())
		return B_ERROR;
	BRect	frame	= editor->PrintFrame();
	// two pixels per point stay sharp on any screen
	float	scale	= std::min(2.0f,
		kMaxBitmapSide / std::max(frame.Width(),frame.Height()));
	BBitmap	*bitmap	= editor->PaperBitmap(scale);
	view->UnlockLooper();
	if (bitmap == NULL)
		return B_NO_MEMORY;
	BBitmapStream	stream(bitmap);
	status_t		err	= BTranslatorRoster::Default()->Translate(&stream,NULL,NULL,
		out,type);
	stream.DetachBitmap(&bitmap);
	delete bitmap;
	return err;
}


status_t PDocument::WriteTo(const entry_ref &ref, const BMessage &saveSettings,
	bool exporting)
{
	BAutolock	autolock(this);
	BMessage	archived;
	Archive(&archived,true);
	BFile		file(&ref,B_WRITE_ONLY | B_ERASE_FILE | B_CREATE_FILE);
	status_t	err	= file.InitCheck();
	int32		outType		= 0;
	bool		useTranslator	= (err == B_OK)
		&& (saveSettings.FindInt32(P_C_FORMAT_TYPE,&outType) == B_OK);
	if (useTranslator && saveSettings.GetBool(P_C_FORMAT_BITMAP,false)) {
		err = WriteBitmap(editorManager,&file,(uint32)outType);
	} else if (useTranslator) {
		// translator ids change between sessions; the format type doesn't
		translator_info	translatorInfo;
		err = FindSaveTranslator((uint32)outType,&translatorInfo.translator);
		if (err == B_OK) {
			// FindString() would leave NULL behind for a missing name
			const char	*formatName	= saveSettings.GetString(P_C_FORMAT_NAME,"");
			translatorInfo.type			= P_C_DOCUMENT_RAW_TYPE;
			translatorInfo.group		= P_C_DOCUMENT_RAW_TYPE;
			translatorInfo.quality		= 0.9;
			translatorInfo.capability	= 0.9;
			strlcpy(translatorInfo.name,formatName,sizeof(translatorInfo.name));
			strlcpy(translatorInfo.MIME,P_C_DOCUMENT_MIMETYPE,sizeof(translatorInfo.MIME));
			BMallocIO	input;
			archived.Flatten(&input);
			// the document's name without extension, e.g. for a page title
			BString		title(Title());
			int32		dot	= title.FindLast('.');
			if (dot > 0)
				title.Truncate(dot);
			BMessage	extension;
			extension.AddString(P_C_FORMAT_TITLE,title);
			err = BTranslatorRoster::Default()->Translate(&input,&translatorInfo,
				&extension,&file,outType);
		}
	} else if (err == B_OK)
		err = archived.Flatten(&file);

	if (err != B_OK) {
		BString	text;
		text.SetToFormat(B_TRANSLATE("Could not save the document: %s"),strerror(err));
		(new BAlert(B_TRANSLATE("Error"),text.String(),B_TRANSLATE("OK"),
			NULL,NULL,B_WIDTH_AS_USUAL,B_STOP_ALERT))->Go(NULL);
		return err;
	}
	BNodeInfo	nodeInfo(&file);
	const char	*mime	= NULL;
	if (exporting && (saveSettings.FindString(P_C_FORMAT_MIME,&mime) == B_OK)) {
		// an export opens in the app made for its format
		nodeInfo.SetType(mime);
	} else {
		nodeInfo.SetType(P_C_DOCUMENT_MIMETYPE);
		nodeInfo.SetPreferredApp(APP_SIGNATURE);
	}
	return B_OK;
}

void PDocument::Load(void)
{
	TRACE();
	BAutolock			autolock(this);
	BMessage			*node			= NULL;
	int32				i				= 0;
	BMessage			loaded;
	BFile				file(entryRef,B_READ_ONLY);
	status_t			err				= file.InitCheck();
	bool				imported		= false;
	if (err == B_OK) {
		BTranslatorRoster	*roster	= BTranslatorRoster::Default();
		translator_info		identified;
		BMallocIO			output;
		err = roster->Identify(&file,NULL,&identified,P_C_DOCUMENT_RAW_TYPE);
		if (err == B_OK)
			err = roster->Translate(&file,&identified,NULL,&output,P_C_DOCUMENT_RAW_TYPE);
		if (err == B_OK) {
			output.Seek(0,SEEK_SET);
			err = loaded.Unflatten(&output);
		}
		// read by another translator than the one writing documents
		translator_id	native;
		imported	= (err == B_OK)
			&& ((FindSaveTranslator(P_C_DOCUMENT_TYPE,&native) != B_OK)
				|| (identified.translator != native));
	}
	if (err != B_OK) {
		BString	text;
		text.SetToFormat(B_TRANSLATE("Could not open the document: %s"),strerror(err));
		(new BAlert(B_TRANSLATE("Error"),text.String(),B_TRANSLATE("OK"),
			NULL,NULL,B_WIDTH_AS_USUAL,B_STOP_ALERT))->Go(NULL);
		return;
	}
	if (imported) {
		// Save must not write the document over the file it came from:
		// it asks for a name, offering the file's without its extension
		importedName	= entryRef->name;
		int32	dot	= importedName.FindLast('.');
		if (dot > 0)
			importedName.Truncate(dot);
		BEntry	imported(entryRef);
		BEntry	directory;
		if ((imported.GetParent(&directory) != B_OK)
			|| (directory.GetRef(&importedDirectory) != B_OK))
			importedDirectory	= entry_ref();
		delete entryRef;
		entryRef	= NULL;
	}
	ResetModified();
	UpdateWindowTitle();

	int32	fileVersion	= 0;
	if ((loaded.FindInt32(P_C_DOC_FORMAT_VERSION_FIELD,&fileVersion) != B_OK)
		|| (fileVersion != P_C_DOC_FORMAT_VERSION)) {
		(new BAlert(B_TRANSLATE("Error"),
			B_TRANSLATE("This file was saved by an incompatible version of ProjectConceptor and cannot be opened."),
			B_TRANSLATE("OK")))->Go();
		return;
	}

	// copies everything it hands out, so it can go when Load() returns
	PDocLoader	loader(this,&loaded);
	PDocLoader	*docLoader	= &loader;
	delete printerSetting;
	delete selected;

	// allNodes/allConnections are kept as the same BList instances (cleared
	// and refilled) instead of being replaced - editors like NavigatorEditor
	// cache the BList pointer for their lifetime, and a delete+reassign here
	// left them pointing at freed memory on the next reload.
	//
	// Any nodes/connections already in these lists (e.g. from editing
	// before this Load(), reusing the same window) have to be marked
	// changed *before* MakeEmpty() drops them - otherwise GraphEditor's
	// diffing (ProcessChangedNode(), driven off valueChanged) never learns
	// they're gone and keeps their now-stale renderers around: still
	// drawn, but no longer selectable/editable since they're not in
	// allNodes anymore. Same changed->insert() pattern Delete::Do() uses,
	// and same as there, never actually deleted - node lifetime here
	// relies on process teardown, not on Load() freeing anything.
	for (i = 0; i<allNodes->CountItems(); i++)
		valueChanged->insert((BMessage*)allNodes->ItemAt(i));
	for (i = 0; i<allConnections->CountItems(); i++)
		valueChanged->insert((BMessage*)allConnections->ItemAt(i));

	BList		*loadedNodes		= docLoader->GetAllNodes();
	allNodes->MakeEmpty();
	for (i = 0; i<loadedNodes->CountItems(); i++) {
		node=((BMessage*)loadedNodes->ItemAt(i));
		node->AddPointer("ProjectConceptor::doc",this);
		valueChanged->insert(node);
		allNodes->AddItem(node);
	}
	delete loadedNodes;
	BList		*loadedConnections	= docLoader->GetAllConnections();
	allConnections->MakeEmpty();
	for (i = 0; i<loadedConnections->CountItems(); i++) {
		node= (BMessage *)loadedConnections->ItemAt(i);
		node->AddPointer("ProjectConceptor::doc",this);
		valueChanged->insert(node);
		allConnections->AddItem(node);
	}
	delete loadedConnections;
	selected = docLoader->GetSelectedNodes();
	delete commandManager;
	commandManager= new PCommandManager(this);
	commandManager->SetMacroList(docLoader->GetMacroList());
	commandManager->SetUndoList(docLoader->GetUndoList());
	commandManager->SetUndoIndex(docLoader->GetUndoIndex());
//	commandManager->LoadMacros(docLoader->GetCommandManagerMessage());
//	commandManager->LoadUndo(docLoader->GetCommandManagerMessage());
	SetPrintSettings( docLoader->GetPrinterSetting());
	// see the same guard/comment in Resize() above - this is the exact
	// crash docs/notes.md already documented as blocking a headless
	// Load() test; the fix is the same one #117 already applied to
	// PCommandManager's three call sites
	if (editorManager != NULL)
		editorManager->BroadCast(BuildChangedNodesMessage());
	commandManager->PlayMacrosOnOpen();
}


BMessage* PDocument::BuildChangedNodesMessage(void)
{
	BMessage	*message	= new BMessage(P_C_VALUE_CHANGED);
	set<BMessage*>::iterator	it;
	for (it = valueChanged->begin(); it != valueChanged->end(); ++it)
		message->AddPointer("node",*it);
	return message;
}


void PDocument::ExportPanel()
{
	TRACE();
	if (!exportPanel) {
		// the panel copies both
		BMessage	request(P_C_EXPORT_REQUESTED);
		BMessenger	target(this);
		exportPanel = new PCSavePanel(&request,&target,true);
	}
	BString	name(Title());
	if (entryRef != NULL) {
		// next to the document, named like it without its extension
		BEntry		entry(entryRef);
		BEntry		directory;
		if (entry.GetParent(&directory) == B_OK)
			exportPanel->SetPanelDirectory(&directory);
		int32	dot	= name.FindLast('.');
		if (dot > 0)
			name.Truncate(dot);
	}
	exportPanel->SetSaveText(name.String());
	exportPanel->FormatChanged();
	exportPanel->Window()->SetWorkspaces(B_CURRENT_WORKSPACE);
	exportPanel->Show();
}


void PDocument::SavePanel()
{
	TRACE();
	if (!savePanel) {
//		savePanel = new PCSavePanel(B_SAVE_PANEL, new BMessenger(this),NULL,0,false);
		savePanel = new PCSavePanel(NULL,new BMessenger(this));
	}
	BEntry	entry;
	BEntry	directory;
	if ((entryRef != NULL) && (entry.SetTo(entryRef) == B_OK)
		&& (entry.GetParent(&directory) == B_OK))
		savePanel->SetPanelDirectory(&directory);
	else if (importedName.Length() > 0) {
		if (importedDirectory.name != NULL)
			savePanel->SetPanelDirectory(&importedDirectory);
		savePanel->SetSaveText(importedName.String());
		savePanel->FormatChanged();
	}
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
		ConfigManager	*configManager		= ((ProjektConceptor*)be_app)->GetConfigManager();
		BMessage		*existingConfig		= configManager->GetConfigMessage(P_C_CONFIG_WINDOW_FRAME_FIELD);
		bool			rememberEnabled		= true;
		if (existingConfig != NULL) {
			if (existingConfig->FindBool(P_C_WINDOW_FRAME_REMEMBER_FIELD,&rememberEnabled) != B_OK)
				rememberEnabled = true;
			delete existingConfig;
		}
		// a disabled toggle just stops updating the remembered frame - it
		// doesn't erase whatever was last stored, so turning it back on
		// later resumes from that position rather than losing it
		if (rememberEnabled) {
			BMessage	frameConfig;
			frameConfig.AddRect(P_C_WINDOW_FRAME_RECT_FIELD,window->Frame());
			configManager->SetConfigMessage(P_C_CONFIG_WINDOW_FRAME_FIELD,&frameConfig);
			configManager->SaveConfig();
		}
		window->SetClosing(true);
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
		node->FindRect(P_C_NODE_FRAME,frame);
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

void PDocument::ApplyAutoSaveSettings(void)
{
	TRACE();
	bool	enabled		= true;
	int32	seconds		= 300;
	if (documentSetting->FindBool(P_C_DOC_AUTOSAVE_ENABLED,&enabled) != B_OK)
		enabled = true;
	if (documentSetting->FindInt32(P_C_DOC_AUTOSAVE_INTERVAL,&seconds) != B_OK)
		seconds = 300;
	if (seconds <= 0)
		seconds = 300;
	autoSaveIntervall	= (bigtime_t)seconds * 1000000;

	delete autoSaver;
	autoSaver = NULL;
	if (enabled) {
		autoSaver = new BMessageRunner(BMessenger(this),
										new BMessage(P_C_AUTO_SAVE),
										autoSaveIntervall);
	}
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
