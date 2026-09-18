#include "PCommand.h"
#include "PCommandManager.h"
#include "PDocument.h"


PCommand::PCommand(void)
{
	TRACE();
	Init();
}


PCommand::~PCommand(void)
{
	TRACE();

}

void PCommand::Init(void)
{
	TRACE();
	manager			= NULL;
//	subPCommands		= new BList();
}

BMessage* PCommand::RunSubCommandsOnce(PDocument *doc, BMessage *settings)
{
	TRACE();
	BMessage	*record				= new BMessage();
	int32		i					= 0;
	BMessage	*subPCommandMessage	= new BMessage;
	PCommand	*subPCommand		= NULL;
	char		*commandName		= NULL;
	while (settings->FindMessage("PCommand::subPCommand",i,subPCommandMessage) == B_OK)
	{
		commandName	= NULL;
		subPCommandMessage->FindString("Command::Name",(const char **)&commandName);
		subPCommand	= manager->GetPCommand(commandName);
		if (subPCommand)
		{
			manager->ResolveBindings(subPCommandMessage,subPCommand);
			subPCommandMessage	= subPCommand->Do(doc,subPCommandMessage);
			record->AddMessage("PCommand::subPCommand",subPCommandMessage);
		}
		i++;
	}
	return record;
}

BMessage* PCommand::Do(PDocument *doc,BMessage *settings)
{
	TRACE();
	BMessage	*record	= RunSubCommandsOnce(doc,settings);
	int32		i		= 0;
	BMessage	subResult;
	// indexed - unindexed overload always hit slot 0 (#116)
	while (record->FindMessage("PCommand::subPCommand",i,&subResult) == B_OK)
	{
		settings->ReplaceMessage("PCommand::subPCommand",i,&subResult);
		i++;
		subResult.MakeEmpty();
	}
	delete record;
	//settings->AddPointer("ProjectConceptor::doc",doc);
	return settings;
}

void PCommand::Undo(PDocument *doc,BMessage *undo)
{
	TRACE();
	BMessage	*subPCommandMessage	= new BMessage();
	PCommand	*subPCommand		= NULL;
	char		*commandName		= NULL;
	int32		i					= 0;
	while (undo->FindMessage("PCommand::subPCommand",i,subPCommandMessage) == B_OK)
	{
		subPCommand	= NULL;
		commandName	= NULL;
		subPCommandMessage->FindString("Command::Name",(const char **)&commandName);
		subPCommand=manager->GetPCommand(commandName);
		if (subPCommand)
			subPCommand->Undo(doc,subPCommandMessage);
		i++;
	}
}

void PCommand::SetManager(PCommandManager *newManager)
{
	TRACE();
	if (newManager != NULL)
	{
		manager=newManager;
		AttachedToManager();
	}
	else
	{
		DetachedFromManager();
		manager=newManager;
	}
}
