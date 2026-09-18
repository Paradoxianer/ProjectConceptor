#include <set>
#include <Catalog.h>

#include <string.h>
#include <interface/Alert.h>
#include <interface/MenuItem.h>
#include <interface/TextView.h>
#include <support/ClassInfo.h>
#include <Catalog.h>

#include "PCommandManager.h"
#include "PDocument.h"
#include "PluginManager.h"
#include "PEditorManager.h"
#include "PDocumentManager.h"
#include "InputRequest.h"
#include "ProjectConceptorDefs.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "CommandManager"

using namespace std;

PCommandManager::PCommandManager(PDocument *initDoc) {
	TRACE();
	doc	= initDoc;
	Init();
}

PCommandManager::~PCommandManager(void) {
	delete undoList;
	delete macroList;
	delete[] fPropertyInfoArray;
}

void PCommandManager::Init(void) {
	commandMap		= map<BString,PCommand *>();
	undoList		= new BList();
	macroList		= new BList();
	undoStatus		= 0;
	recording		= NULL;
	fPropertyInfoArray	= NULL;

	PluginManager	*pluginManager	= (doc->BelongTo())->GetPluginManager();
	BList 			*commands		= pluginManager->GetPluginsByType(P_C_COMMANDO_PLUGIN_TYPE);
	if (commands) {
		for (int32 i=0; i<commands->CountItems();i++)
			RegisterPCommand((BasePlugin *)commands->ItemAt(i));
	}
}

status_t PCommandManager::Archive(BMessage *archive, bool deep) {
	int32	i	= 0;
	for (i = 0; i<undoList->CountItems(); i++) {
		BMessage	*undoMessage	= (BMessage*)undoList->ItemAt(i);
		archive->AddMessage("undo",undoMessage);
	}
	for (i = 0; i<macroList->CountItems(); i++) {
		BMessage	*macroMessage	= (BMessage*)macroList->ItemAt(i);
		archive->AddMessage("macro",macroMessage);
	}
	if (deep)
		archive->AddInt32("undoStatus",undoStatus);
	//** need a good Errorcheck
	return B_OK;
}

status_t PCommandManager::RegisterPCommand(BasePlugin *commandPlugin) {
	TRACE();
	status_t err	= B_OK;
	if (commandPlugin) {
		PCommand *command	= NULL;
		command = (PCommand *)commandPlugin->GetNewObject(NULL);
		if (command) {
			command->SetManager(this);
			PRINT(("Register Command %s",command->Name()));
			commandMap[BString(command->Name())]=command;
//			err=commandVector->AddItem(BString(command->Name()),command);
		}
		else
			err = B_ERROR;
	}
	else
		err = B_BAD_VALUE;
	return err;
}

status_t PCommandManager::SetMacroList(BList *newMacroList) {
	status_t	err			= B_OK;
	if (newMacroList) {
//		BMenu 		*macroPlay 		= doc->GetMenu(P_MENU_MACRO_PLAY);
		BMessage	*macro			= NULL;
		char		*name			= NULL;
/*		while (macroPlay!=NULL)
		{
			singleMacro=macroPlay->RemoveItem((int32)0);
			while (singleMacro)
			{
				delete singleMacro;
				singleMacro=macroPlay->RemoveItem((int32)0);
			}
			macroPlay++;
		}*/
		delete macroList;
		macroList = newMacroList;
		for (int32 i=0;i<macroList->CountItems();i++) {
			macro =(BMessage *) macroList->ItemAt(i);
			macro->FindString("Name",(const char**)&name);
			BMenuItem	*item	= new BMenuItem(name,macro);
			item->SetTarget(doc);
			doc->AddMenuItem(P_MENU_MACRO_PLAY,item);
		}
	}
	else
		err = B_BAD_VALUE;
	return err;
}

status_t PCommandManager::SetUndoList(BList *newUndoList) {
	status_t	err			= B_OK;
	if (newUndoList) {
		delete undoList;
		undoList	= newUndoList;
	}
	else
		err = B_BAD_VALUE;
	return err;
}


void PCommandManager::UnregisterPCommand(char* name) {
	TRACE();
	if (name)
		commandMap.erase(name);
//		commandVector->RemoveItemFor(name);
}


void PCommandManager::StartMacro(void) {
	TRACE();
	if (!recording) {
		recording		= new BMessage(P_C_MACRO_TYPE);
		macroIndexer	= new Indexer(doc);
	}
	else {
		int32 choice	= (new BAlert(B_TRANSLATE("Error!"),B_TRANSLATE("Macro recording already started!"),B_TRANSLATE("Restart recording"),B_TRANSLATE("Continue recording"),NULL, B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_STOP_ALERT))->Go();
		if (choice == 0) {
			delete macroIndexer;
			macroIndexer	= new Indexer(doc);
			delete recording;
			recording	= new BMessage(P_C_MACRO_TYPE);
		}
	}

}

void PCommandManager::StopMacro() {
	TRACE();
	InputRequest	*inputAlert = new InputRequest(B_TRANSLATE("Input macro name"),B_TRANSLATE("Name"), B_TRANSLATE("Macro"), B_TRANSLATE("OK"),B_TRANSLATE("Cancel"));
	char			*input		= NULL;
	char			*inputstr	= NULL;
	if  (recording)	{
		if (inputAlert->Go(&input)<1) {
			inputstr	= new char[strlen(input)+1];
			strcpy(inputstr,input);
			recording->AddString("Name",input);
			// a littel trick because we need a pointer to the text wich isnt deleted :))
			macroList->AddItem(new BMessage(*recording));
			BMenuItem	*item	= new BMenuItem(input,(BMessage *)macroList->LastItem());
			item->SetTarget(doc);
			doc->AddMenuItem(P_MENU_MACRO_PLAY,item);
		}
		inputAlert->Lock();
		inputAlert->Quit();
		delete recording;
		delete macroIndexer;
		recording		= NULL;
		macroIndexer	= NULL;
	}

}

void PCommandManager::PlayMacro(BMessage *makro) {
	int32 		i				= 0;
	BMessage	*message		= new BMessage();
	Indexer		*playDeIndexer	= new Indexer(doc);
	status_t	err				= B_OK;
	while ( (makro->FindMessage("Macro::Commmand", i,message) == B_OK) && (err==B_OK) )
	{
		err = Execute(playDeIndexer->DeIndexCommand(message));
		// Meant to give GraphEditor's own thread a chance to catch up
		// visually between steps - confirmed live this doesn't actually
		// work: 400ms (like the original 100ms) still shows nothing
		// incrementally, everything appears at once only once the whole
		// macro finishes. Tried and reverted: making BroadCast()
		// synchronous instead of fire-and-forget (PEditorManager::
		// SendMessage()) - deadlocked PCommandManager::Undo()/Redo(),
		// which broadcast while still holding the document lock, and
		// GraphEditor::ValueChanged() needs that same lock to process the
		// synchronous send. The real reason nothing renders incrementally
		// during PlayMacro() is still open.
		snooze(400000);
		i++;
	}

}

void PCommandManager::PlayMacroByName(const char *name) {
	BMessage	*macro	= NULL;
	bool		found	= false;
	for (int32 i = 0; (!found) && (i < macroList->CountItems()); i++) {
		macro = (BMessage *)macroList->ItemAt(i);
		const char	*macroName	= NULL;
		if ((macro->FindString("Name",&macroName) == B_OK) && (strcmp(macroName,name) == 0))
			found = true;
	}
	if (found)
		PlayMacro(macro);
	else
		PRINT(("PCommandManager::PlayMacroByName - no macro named \"%s\" in this document\n",name));
}

// #132: the same logical edit is recorded two different, incompatible ways
// depending purely on which UI path triggered it - the GraphEditor toolbar
// (fill color, pen size, connection style/arrows) already sends
// Node::selected=true, portable and independent of the document it plays
// back into, while direct manipulation (ClassRenderer's inline name/
// attribute editing, NavigatorEditor's field editor) sends an explicit
// "node" pointer, tying the recorded macro to this exact document's live
// objects. In practice the node direct manipulation touches is always the
// current selection (you can't type into a node without it being focused/
// selected) - so this normalizes that case right before macro recording:
// a command whose "node" pointers exactly match what was selected right
// before it ran gets recorded the portable way too.
static const char* const kSelectionNormalizableCommands[] = {
	"ChangeValue", "AddAttribute", "RemoveAttribute", "Copy", "Move", NULL
};

static bool IsSelectionNormalizable(const char *commandName)
{
	if (commandName == NULL)
		return false;
	for (int32 i = 0; kSelectionNormalizableCommands[i] != NULL; i++)
		if (strcmp(commandName,kSelectionNormalizableCommands[i]) == 0)
			return true;
	return false;
}

// true (with every top-level "node" pointer field on `settings` removed and
// replaced by Node::selected=true) only if those pointers are exactly the
// set in `selectionSnapshot` - same members, same count. False, `settings`
// left untouched, if it has no "node" field at all (nothing to normalize -
// already the portable form) or the sets don't match exactly: a genuinely
// document-local target (e.g. a group's parent chain in ClassRenderer::
// AdjustParents()'s own ChangeValue subcommand, computed per-node and never
// equal to the top-level selection) must keep its real pointer, or replay
// would silently apply the edit to the wrong node(s).
static bool NormalizeToSelection(BMessage *settings, BList *selectionSnapshot)
{
	set<BMessage*>	settingsNodes;
	BMessage		*node	= NULL;
	int32			i		= 0;
	while (settings->FindPointer("node",i,(void **)&node) == B_OK) {
		settingsNodes.insert(node);
		i++;
	}
	if (settingsNodes.empty())
		return false;
	if ((int32)settingsNodes.size() != selectionSnapshot->CountItems())
		return false;
	for (i=0; i<selectionSnapshot->CountItems(); i++)
		if (settingsNodes.find((BMessage*)selectionSnapshot->ItemAt(i)) == settingsNodes.end())
			return false;
	settings->RemoveName("node");
	settings->AddBool(P_C_NODE_SELECTED,true);
	return true;
}


status_t PCommandManager::Execute(BMessage *settings) {
	TRACE();
	DEBUG_ONLY(settings->PrintToStream());
	status_t	err	= doc->LockWithTimeout(TIMEOUT_LOCK);
	if (err == B_OK) {
		(doc->GetChangedNodes())->clear();
		bool		shadow				= false;
		char		*commandName		= NULL;
		PCommand	*command			= NULL;
		settings->FindString("Command::Name",(const char**)&commandName);
		command		= GetPCommand(commandName);
		// snapshotted BEFORE Do() runs, not after - Find/Select change
		// doc->GetSelected() as part of what they themselves do, so a
		// post-Do() snapshot would compare (say) ChangeValue's settings
		// against a selection state ChangeValue never actually saw.
		// Building the snapshot at all costs one BList walk - skipped
		// entirely unless something is even recording right now and this
		// command is one NormalizeToSelection() ever applies to.
		BList	selectionSnapshot;
		bool	wantsNormalization	= (recording != NULL)
			&& IsSelectionNormalizable(commandName);
		if (wantsNormalization) {
			BList	*selected	= doc->GetSelected();
			for (int32 i=0;i<selected->CountItems();i++)
				selectionSnapshot.AddItem(selected->ItemAt(i));
		}
		if (command != NULL) {
			BMessage	*tmpMessage;
			try  {
				tmpMessage = command->Do(doc, settings);
			}
			catch(...){
				BAlert *alert = new BAlert(commandName, B_TRANSLATE("Error on execution of Command."), B_TRANSLATE("OK"));
				alert->Go();
				err=B_ERROR;
			}
			if (err==B_OK){
				err				= settings->FindBool("shadow",&shadow);
				if ((err != B_OK) ) {
					if (recording) {
						// a *separate* copy, never settings itself - Do()
						// commonly returns the very same object as
						// tmpMessage, which the undo list below copies from
						// verbatim; stripping "node" out of settings in
						// place would silently corrupt that undo entry,
						// leaving Undo() with no node to act on at all.
						BMessage	normalized(*settings);
						BMessage	*forRecording	= settings;
						if (wantsNormalization
								&& NormalizeToSelection(&normalized,&selectionSnapshot))
							forRecording	= &normalized;
						recording->AddMessage("Macro::Commmand", macroIndexer->IndexMacroCommand(forRecording));
					}
					if (!shadow) {
						undoList->RemoveItems(undoStatus+1,undoList->CountItems()-undoStatus);
						if (tmpMessage!= NULL)
							undoList->AddItem(new BMessage(*tmpMessage));
						undoStatus	= undoList->CountItems()-1;
					}
					err=B_OK;
				}
				doc->SetModified();
				doc->Unlock();
				// a document with no attached UI (currently: any headless
				// PDocument, e.g. NewHeadlessTestDocument() in the test
				// suite) has no PEditorManager to notify - nothing to
				// broadcast to, not an error (#117). Deterministic null
				// dereference otherwise: reproduced by sending a real
				// P_C_EXECUTE_COMMAND via BMessenger to a headless doc.
				if (doc->GetEditorManager() != NULL)
					doc->GetEditorManager()->BroadCast(doc->BuildChangedNodesMessage());
			}
		}
		else
		{
			char	*error	= new char[255];
			sprintf(error,"%s: %s",B_TRANSLATE("Could not find command"),commandName);
			(new BAlert(B_TRANSLATE("Error!"),error, B_TRANSLATE("OK"),NULL,NULL, B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_STOP_ALERT))->Go();
			delete error;
			err = B_ERROR;
		}
	}
	else
		printf("Error Locking PDocument - %s\n",strerror(err));
	return err;
}


PCommand* PCommandManager::GetPCommand(char* name)
{
	TRACE();
	return commandMap[BString(name)];
}



void PCommandManager::Undo(BMessage *undo) {
	TRACE();
	int32 			i					= undoStatus;
	int32 			index				= undoList->IndexOf(undo);
	char			*commandName		= NULL;
	PCommand		*undoPCommand		= NULL;
	BMessage		*msg				= NULL;
	status_t		err					= doc->LockWithTimeout(TIMEOUT_LOCK);
	if (err == B_OK) {
		(doc->GetChangedNodes())->clear();
		if (index<0)
			index=undoStatus;
		while (i>=index) {
			msg	= (BMessage *) undoList->ItemAt(i);
			if (msg != NULL) {
				msg->FindString("Command::Name",(const char**)&commandName);
				undoPCommand	= GetPCommand(commandName);
				if (undoPCommand != NULL)
					undoPCommand->Undo(doc,msg);
				else
					PRINT(("ERROR:\t PCommandManager - Didnt found the PCommand\n"));
				undoStatus--;
				if (undoStatus<0)
					undoStatus = -1;
			}
			i--;
		}
		// see the same guard/comment in Execute() above
		if (doc->GetEditorManager() != NULL)
			doc->GetEditorManager()->BroadCast(doc->BuildChangedNodesMessage());
		doc->Unlock();
	}
	else
		printf("Error Locking PDocument - %s\n",strerror(err));
}

void PCommandManager::Redo(BMessage *redo) {
	TRACE();
	int32 			i				= undoStatus+1;
	int32			index			= undoList->IndexOf(redo);
	char			*commandName	= NULL;
	PCommand		*redoPCommand	= NULL;
	BMessage		*msg			= NULL;
	status_t		err				= doc->LockWithTimeout(TIMEOUT_LOCK);
	if (err == B_OK) {
		(doc->GetChangedNodes())->clear();
		if (index<0)
			index=undoStatus+1;
		while (i<=index) {
			msg	= (BMessage *) undoList->ItemAt(i);
			if 	(msg != NULL) {
				msg->FindString("Command::Name",(const char**)&commandName);
				redoPCommand	= GetPCommand(commandName);
				if (redoPCommand)
					redoPCommand->Do(doc,msg);
				else
					PRINT(("ERROR\tPCommandManager - Coudn´t find the PCommand\n"));
			}
			i++;
			undoStatus++;
			if (undoStatus > (undoList->CountItems()-1))
				undoStatus = undoList->CountItems()-1;
		}
		// see the same guard/comment in Execute() above
		if (doc->GetEditorManager() != NULL)
			doc->GetEditorManager()->BroadCast(doc->BuildChangedNodesMessage());
		doc->Unlock();
	}
	else
		printf("Error Locking PDocument - %s\n",strerror(err));
}


PCommand* PCommandManager::PCommandAt(int32 index) {
	if ((index < 0) || (index >= (int32)commandMap.size()))
		return NULL;
	map<BString, PCommand*>::iterator iter;
	iter=commandMap.begin();
	for (int i=0;i< index;i++)
		iter++;
  	return iter->second;
}


BPropertyInfo* PCommandManager::BuildPropertyInfo(void) {
	int32			total		= 0;
	int32			i			= 0;
	int32			commandCount	= CountPCommand();
	// two passes - first count, so the array is allocated exactly once.
	// PCommandAt(i) is guarded (not just trusted to be non-NULL) - this
	// walks every index up to CountPCommand() in one go, unlike any
	// pre-existing caller, and PCommandAt() itself is a raw std::map
	// iterator walk with no bounds check of its own.
	for (i = 0; i < commandCount; i++) {
		PCommand	*command	= PCommandAt(i);
		if (command == NULL)
			continue;
		int32	n	= 0;
		command->PropertyInfo(&n);
		total	+= n;
	}
	// BPropertyInfo's own constructor doesn't take a count - it scans the
	// array itself until it finds an all-zero "name" sentinel entry (see
	// BPropertyInfo::BPropertyInfo() in the Haiku source: "while
	// (fPropInfo[fPropCount].name) fPropCount++;"), the same convention
	// BListView's own sProperties[] table ends with ({ 0 }). One extra,
	// value-initialized (all-zero) slot at the end provides that sentinel
	// - without it, the constructor reads past this array into whatever
	// heap memory follows, corrupting fPropCount and crashing later.
	delete[] fPropertyInfoArray;
	fPropertyInfoArray	= new property_info[total+1]();
	int32			offset		= 0;
	for (i = 0; i < commandCount; i++) {
		PCommand	*command	= PCommandAt(i);
		if (command == NULL)
			continue;
		int32				n		= 0;
		const property_info	*props	= command->PropertyInfo(&n);
		for (int32 j = 0; j < n; j++)
			fPropertyInfoArray[offset+j]	= props[j];
		offset	+= n;
	}
	// freeOnDelete=false: every name/usage/field-name string in this
	// array is a pointer into some command's own `static const`
	// property_info array (a string literal), not individually malloc()d
	// - BPropertyInfo's freeOnDelete=true calls plain free() on each one
	// of those pointers (and on the array itself via free(), not
	// delete[]) to match how Unflatten() builds one; doing that to a
	// string literal's address is a crash (confirmed live - see #55
	// commit history). This array is owned by PCommandManager
	// (fPropertyInfoArray) instead, across the life of the returned
	// BPropertyInfo - callers must delete the BPropertyInfo, never touch
	// the array directly.
	return new BPropertyInfo(fPropertyInfoArray, NULL, false);
}
