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

BMessage* PCommand::Do(PDocument *doc,BMessage *settings)
{
	TRACE();
	int32		i					= 0;
	BMessage	*subPCommandMessage	= new BMessage;
	PCommand	*subPCommand			= NULL;
	char		*commandName		= NULL;
	//Check if there are settings vor SubCommands
	while 	(settings->FindMessage("PCommand::subPCommand",i,subPCommandMessage) == B_OK)
	{		
		subPCommandMessage->FindString("Command::Name",(const char **)&commandName);
		subPCommand=manager->GetPCommand(commandName);
		if (subPCommand)
		{
			subPCommandMessage = subPCommand->Do(doc,subPCommandMessage);
			settings->ReplaceMessage("PCommand::subPCommand",subPCommandMessage);
		}
		i++;
	}
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
