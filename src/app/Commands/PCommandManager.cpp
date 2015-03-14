#include <set>

#include <string.h>
#include <interface/Alert.h>
#include <interface/MenuItem.h>
#include <interface/TextView.h>
#include <support/ClassInfo.h>
#include "PCommandManager.h"
#include "PDocument.h"
#include "PluginManager.h"
#include "PEditorManager.h"
#include "PDocumentManager.h"
#include "InputRequest.h"


#ifdef B_ZETA_VERSION_1_0_0
	#include <locale/Locale.h>
	#include <locale/LanguageNotifier.h>
#else
	#define _T(a) a
#endif

using namespace std;

PCommandManager::PCommandManager(PDocument *initDoc) {
	TRACE();
	doc	= initDoc;
	Init();
}

PCommandManager::~PCommandManager(void) {
	delete undoList;
	delete macroList;
}

void PCommandManager::Init(void) {
	commandMap		= map<BString,PCommand *>();
	undoList		= new BList();
	macroList		= new BList();
	undoStatus		= 0;
	recording		= NULL;

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
		int32 choice	= (new BAlert("Error!",_T("Macro Recording already started!"),_T("Restart Recording"),_T("Contiune Recording"),NULL, B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_STOP_ALERT))->Go();
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
	InputRequest	*inputAlert = new InputRequest(_T("Input Macroname"),_T("Name"), _T("Macro"), _T("OK"),_T("Cancel"));
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
		snooze(100000);
		i++;
	}

}

status_t PCommandManager::Execute(BMessage *settings) {
	TRACE();
	status_t	err	= B_OK;
	if (doc->Lock()) {
		//(doc->GetChangedNodes())->MakeEmpty();
		(doc->GetChangedNodes())->empty();
		bool		shadow				= false;
		char		*commandName		= NULL;
		PCommand	*command			= NULL;
		settings->FindString("Command::Name",(const char**)&commandName);
		command		= GetPCommand(commandName);
		if (command != NULL) {
			BMessage	*tmpMessage;
			try  {
				tmpMessage = command->Do(doc, settings);
			}
			catch(...){
				BAlert *alert = new BAlert(commandName, "Error on execution of Command.", "OK");
				alert->Go();
				err=B_ERROR;
			}
			if (err==B_OK){
				err				= settings->FindBool("shadow",&shadow);
				if ((err != B_OK) ) {
					if (recording)
						recording->AddMessage("Macro::Commmand", macroIndexer->IndexMacroCommand(settings));
					if (!shadow) {
						undoList->RemoveItems(undoStatus+1,undoList->CountItems()-undoStatus);
						undoList->AddItem(new BMessage(*tmpMessage));
						undoStatus	= undoList->CountItems()-1;
					}
					err=B_OK;
				}
				doc->SetModified();
				(doc->GetEditorManager())->BroadCast(new BMessage(P_C_VALUE_CHANGED));
			}
			doc->Unlock();
		}
		else
		{
			char	*error	= new char[255];
			sprintf(error,"%s: %s",_T("Coud not Find Command"),commandName);
			(new BAlert("Error!",error, "OK",NULL,NULL, B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_STOP_ALERT))->Go();
			delete error;
			err = B_ERROR;
		}
	}
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
	if (doc->Lock()) {
		(doc->GetChangedNodes())->empty();
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
		(doc->GetEditorManager())->BroadCast(new BMessage(P_C_VALUE_CHANGED));
		doc->Unlock();
	}
}

void PCommandManager::Redo(BMessage *redo) {
	TRACE();
	int32 			i				= undoStatus+1;
	int32			index			= undoList->IndexOf(redo);
	char			*commandName	= NULL;
	PCommand		*redoPCommand	= NULL;
	BMessage		*msg			= NULL;
	if (doc->Lock()) {
		(doc->GetChangedNodes())->empty();
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
		(doc->GetEditorManager())->BroadCast(new BMessage(P_C_VALUE_CHANGED));
		doc->Unlock();
	}
}


PCommand* PCommandManager::PCommandAt(int32 index) {
	map<BString, PCommand*>::iterator iter;
	iter=commandMap.begin();
	for (int i=0;i< index;i++)
		iter++;
  	return iter->second;
}
