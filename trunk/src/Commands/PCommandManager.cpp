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

#ifdef B_ZETA_VERSION_BETA
	#include <locale/Locale.h>
	#include <locale/LanguageNotifier.h>
#else
	#define _T(a) 
#endif 

PCommandManager::PCommandManager(PDocument *initDoc)
{
	TRACE();
	document	= initDoc;
	Init();
}


void PCommandManager::Init(void)
{
	commandVector	= new BKeyedVector<BString,PCommand *>();
	undoList	= new BList();
	macroList	= new BList();
	undoStatus	= 0;
	recording	= NULL;

	PluginManager	*pluginManager	= (document->BelongTo())->GetPluginManager();
	BList 			*commands		= pluginManager->GetPluginsByType(P_C_COMMANDO_PLUGIN_TYPE);
	for (int32 i=0; i<commands->CountItems();i++)
	{
		RegisterPCommand((BasePlugin *)commands->ItemAt(i));
	}
}

status_t PCommandManager::Archive(BMessage *archive, bool deep = true)
{
	for (int32 i=0;i<undoList->CountItems();i++)
	{
		BMessage	*undoMessage	= (BMessage*)undoList->ItemAt(i);
/*		BMessage	*deIndexed		= new BMessage(*undoMessage);
		if (DeIndex(deIndexed) == B_OK)
			archive->AddMessage("undo",deIndexed);*/
		archive->AddMessage("undo",undoMessage);
	}
	for (int32 i=0;i<macroList->CountItems();i++)
	{
		BMessage	*macroMessage	= (BMessage*)macroList->ItemAt(i);
//		BMessage	*deIndexed		= new BMessage(*macroMessage);
/*		BMessage	*macroCommand	= new BMessage();
		int32 		j				= 0;
		while (deIndexed->FindMessage("Macro::Command",j,macroCommand) == B_OK)
		{
			if (DeIndex(macroCommand) == B_OK)	
				deIndexed->ReplaceMessage("Macro::Command",j,macroCommand);
			j++;
		}
		archive->AddMessage("macro",deIndexed);*/
		archive->AddMessage("macro",macroMessage);
	}
	archive->AddInt32("undoStatus",undoStatus);
	//** need a good Errorcheck
	return B_OK;
}

status_t PCommandManager::RegisterPCommand(BasePlugin *commandPlugin)
{
	TRACE();
	status_t err	= B_OK;
	if (commandPlugin)
	{
		PCommand *command	= NULL;
		command = (PCommand *)commandPlugin->GetNewObject(NULL);
		if (command)
		{
			command->SetManager(this);
			PRINT(("Register Command %s",command->Name()));
			err=commandVector->AddItem(BString(command->Name()),command);
		}
		else
			err = B_ERROR;
	}
	else
		err = B_BAD_VALUE;
	return err;
}

status_t PCommandManager::LoadMacros(BMessage *archive)
{
	status_t	err			= B_OK;
	BMessage	*tmpMessage	= new BMessage();
	int32		i			= 0;
	//** todo Error Check
	while (archive->FindMessage("macro",i,tmpMessage) == B_OK)
	{
//		if ( ReIndex(tmpMessage) == B_OK)
			macroList->AddItem(tmpMessage);
		tmpMessage = new BMessage();
		i++;
	}
	return err;
}

status_t PCommandManager::LoadUndo(BMessage *archive)
{
	status_t	err			= B_OK;	
	//** todo Error Check
	undoList->MakeEmpty();
	BMessage	*tmpMessage	= new BMessage();
	int32		i			= 0;
	while (archive->FindMessage("undo",i,tmpMessage) == B_OK)
	{
//		if ( ReIndex(tmpMessage) == B_OK)
			undoList->AddItem(tmpMessage);
		tmpMessage = new BMessage();
		i++;
	}
	archive->FindInt32("undoStatus",&undoStatus);
	return err;
}


void PCommandManager::UnregisterPCommand(char* name)
{
	TRACE();
	if (name)
		commandVector->RemoveItemFor(name);
}


void PCommandManager::StartMacro(void)
{
	TRACE();
	if (!recording)	
		recording	= new BMessage(P_C_MACRO_TYPE);
	else
	{
		//**alter that we still recording
		delete recording;
		recording	= new BMessage(P_C_MACRO_TYPE);
	}
	
}

void PCommandManager::StopMacro()
{
	TRACE();
	if (recording)
	{
		macroList->AddItem(new BMessage(*recording));
		delete recording;
		recording = NULL;
		BMenuItem	*item	= new BMenuItem("Macro",(BMessage *)macroList->LastItem());
		item->SetTarget(document);
		document->AddMenuItem(P_MENU_MACRO_PLAY,item);
	}
	
}

void PCommandManager::PlayMacro(BMessage *makro)
{
	int32 		i			= 0;
	BMessage	*message	= new BMessage();
	while (makro->FindMessage("Macro::Commmand", i,message) == B_OK)
	{
		Execute(message);
		snooze(100000);
		i++;
	}

}

void PCommandManager::Execute(BMessage *settings)
{
	TRACE();
	settings->PrintToStream();
	if (document->Lock())
	{
		(document->GetChangedNodes())->MakeEmpty();
		(document->GetTrash())->MakeEmpty();
		bool		shadow				= false;
		char		*commandName		= NULL;
		PCommand	*command			= NULL;
		settings->FindString("Command::Name",(const char**)&commandName);
		command		= GetPCommand(commandName);
		if (command != NULL)
		{
			BMessage	*tmpMessage		= command->Do(document, settings);
			status_t	err				= settings->FindBool("shadow",&shadow);
			if ((err != B_OK) && (!shadow))
			{
				if (recording)
					recording->AddMessage("Macro::Commmand", settings);
				undoList->RemoveItems(undoStatus+1,undoList->CountItems()-undoStatus);
				undoList->AddItem(new BMessage(*tmpMessage));
				undoStatus	= undoList->CountItems()-1;
			}
			(document->GetEditorManager())->BroadCast(new BMessage(P_C_VALUE_CHANGED));
			document->Unlock();
		}
	}
}


PCommand* PCommandManager::GetPCommand(char* name)
{
	TRACE();
/*	BasePlugin	*plugin		= NULL;
	PCommand	*command	= NULL;
	bool 	found			= false;
	int32 	i				= 0;
	while ((i<commandVector->CountItems())&&(!found))
	{
		plugin	= (BasePlugin*)commandVector->ItemAt(i);
		if (strcmp(plugin->GetName(),name) == B_OK)
		{
			command = (PCommand *)plugin->GetNewObject(NULL);
			command->SetManager(this);
			found=true;
		}
		i++;
	}
	return command;	*/
	return commandVector->ValueFor(BString(name));
}



void PCommandManager::Undo(BMessage *undo)
{
	TRACE();
	int32 			i					= undoStatus;
	int32 			index				= undoList->IndexOf(undo);
	char			*commandName		= NULL;
	PCommand		*undoPCommand		= NULL;
	BMessage		*msg				= NULL;	
	if (document->Lock())
	{
		(document->GetChangedNodes())->MakeEmpty();
		(document->GetTrash())->MakeEmpty();
		if (index<0) 
			index=undoStatus;
		while (i>=index)
		{
			msg	= (BMessage *) undoList->ItemAt(i);
			if (msg != NULL)
			{
				msg->FindString("Command::Name",(const char**)&commandName);
				undoPCommand	= GetPCommand(commandName);
				if (undoPCommand != NULL)
					undoPCommand->Undo(document,msg);
				else
					PRINT(("ERROR:\t PCommandManager","Didnt found the PCommand\n"));
				undoStatus--;
				if (undoStatus<0)
					undoStatus = -1;
			}
			i--;
		}
		(document->GetEditorManager())->BroadCast(new BMessage(P_C_VALUE_CHANGED));
		document->Unlock();
	}
}

void PCommandManager::Redo(BMessage *redo)
{
	TRACE();
	int32 			i				= undoStatus+1;
	int32			index			= undoList->IndexOf(redo);
	char			*commandName	= NULL;
	PCommand		*redoPCommand	= NULL;
	BMessage		*msg			= NULL;
	if (document->Lock())
	{	
		(document->GetChangedNodes())->MakeEmpty();
		(document->GetTrash())->MakeEmpty();
		if (index<0)	
			index=undoStatus+1;
		while (i<=index)
		{
			msg	= (BMessage *) undoList->ItemAt(i);
			if 	(msg != NULL)
			{
				msg->FindString("Command::Name",(const char**)&commandName);
				redoPCommand	= GetPCommand(commandName);
				if (redoPCommand)
					redoPCommand->Do(document,msg);
				else
					PRINT(("ERROR\tPCommandManager","Coudn´t find the PCommand\n"));
			}
			i++;
			undoStatus++;
			if (undoStatus > (undoList->CountItems()-1))
				undoStatus = undoList->CountItems()-1;
		}
		(document->GetEditorManager())->BroadCast(new BMessage(P_C_VALUE_CHANGED));
		document->Unlock();
	}
}

