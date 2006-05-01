#include "PDocLoader.h"

#include <support/Archivable.h>

PDocLoader::PDocLoader(BEntry *openEntry)
{
	Init();
	toLoad	= new BFile (openEntry,B_READ_ONLY);
//	Preprocess();
	Load();
}

PDocLoader::~PDocLoader(void)
{
}

BList* PDocLoader::GetAllNodes(void)
{
	return allNodes;
}


BList* PDocLoader::GetAllConnections(void)
{
	return allConnections;
}

BList* PDocLoader::GetSelectedNodes(void)
{
	return selectedNodes;
}


BMessage* PDocLoader::GetPrinterSetting(void)
{
	return printerSettings;
}
BMessage* PDocLoader::GetSettings(void)
{
	return settings;
}
WindowManager* PDocLoader::GetWindowManager(void)
{
	return windowManager;
}

PEditorManager* PDocLoader::GetEditorManager(void)
{
	return editorManager;
}

void PDocLoader::Init(void)
{
	allNodes				= NULL;
	selectedNodes			= NULL;
	allConnections			= NULL;
	settings				= new BMessage();
	
	printerSettings			= new BMessage();
	commandManagerMessage	= new BMessage();
	
	editorManager			= NULL;
	windowManager			= NULL;
	
	toLoad					= NULL;
	loadedStuff				= new BMessage();
	sorter					= new BKeyedVector<int32,BMessage*>();
	
}


void PDocLoader::Load(void)
{
	BMessage	*loadedStuff			= new BMessage();
	BMessage	*allNodesMessage		= new BMessage();
	if (toLoad->InitCheck() == B_OK)
	{
		if (loadedStuff->Unflatten(toLoad) == B_OK)
		{
			loadedStuff->FindMessage("PDocument::printerSetting",printerSettings);
			loadedStuff->FindMessage("PDocument::documentSetting",settings);
			loadedStuff->FindMessage("PDocument::allNodes",allNodesMessage);
			loadedStuff->FindMessage("PDocument::commandManager",commandManagerMessage);
			allNodes		=	Spread(allNodesMessage);
			allConnections	=	ReIndexConnections(loadedStuff);
			selectedNodes	=	ReIndexSelected(loadedStuff);
			ReIndexUndo(commandManagerMessage);
			ReIndexMacro(commandManagerMessage);
		}
	}
}

BList* PDocLoader::Spread(BMessage *allNodeMessage)
{
	BMessage	*subContainerList	= new BMessage();
	BMessage	*node				= new BMessage();
	BMessage	*newNode			= NULL;
	BList		*newAllNodes		= new BList();
	void		*tmpPointer			= NULL;
	int32		i					= 0;
	while (allNodeMessage->FindMessage("node",i,node) == B_OK)
	{
		if (node->FindMessage("SubContainerList",subContainerList) == B_OK)
		{
			if (node->FindPointer("SubContainer",&tmpPointer) == B_OK)
			{
				node->ReplacePointer("SubContainer",Spread(subContainerList));
			}
			else
				node->AddPointer("SubContainer",Spread(subContainerList));
			//Find alle Pointer und löschen
		}
		node->FindPointer("this",(void **)&tmpPointer);
		node->RemoveName("this");
		//we shoud do this over the GraphEditor->BeforeLoad()
		char *name; 
		uint32 type; 
		int32 count; 
		while (node->GetInfo(B_POINTER_TYPE,0 ,(const char **)&name, &type, &count) == B_OK)
		{
			node->RemoveName(name);
		}
		newNode	= new BMessage(*node);
		newAllNodes->AddItem(newNode);
		sorter->AddItem((int32)tmpPointer,newNode);
		i++;
	}
	return newAllNodes;
}


BList* PDocLoader::ReIndexConnections(BMessage *allConnectionsMessage)
{
	BMessage	*connection			= new BMessage();
	BList		*newConnections		= new BList();
	int32		i					= 0;
	void		*fromPointer		= NULL;
	void		*toPointer			= NULL;
	
	while (allConnectionsMessage->FindMessage("PDocument::allConnections",i,connection) == B_OK)
	{
		connection->FindPointer("From",(void **)&fromPointer);
		connection->FindPointer("To",(void **)&toPointer);
		char *name; 
		uint32 type; 
		int32 count; 
		while (connection->GetInfo(B_POINTER_TYPE,0 ,(const char **)&name, &type, &count) == B_OK)
		{
			connection->RemoveName(name);
		}
		connection->AddPointer("From",sorter->ValueFor((int32)fromPointer));
		connection->AddPointer("To",sorter->ValueFor((int32)toPointer));
		newConnections->AddItem(new BMessage(*connection));
		i++;
	}
	return newConnections;
}

BList* PDocLoader::ReIndexSelected(BMessage *selectionMessage)
{
	BList		*newSelection		= new BList();
	int32		i					= 0;
	void		*selectPointer		= NULL;
	
	while (selectionMessage->FindPointer("PDocument::selected",i,&selectPointer) == B_OK)
	{
		newSelection->AddItem(sorter->ValueFor((int32)selectPointer));
		i++;
	}
	return newSelection;
}

void PDocLoader::ReIndexUndo(BMessage *reindexUndo)
{
	BMessage	*undoCommand		= new BMessage();
	int32		i					= 0;

	while (reindexUndo->FindMessage("undo",i,undoCommand)==B_OK)
	{
		ReIndexCommand(undoCommand);
		reindexUndo->ReplaceMessage("undo",i,undoCommand);
		i++;
	}
}

void PDocLoader::ReIndexMacro(BMessage *reindexMacro)
{
	BMessage	*macroCommand		= new BMessage();
	int32		i					= 0;
	while (reindexMacro->FindMessage("Macro::Commmand",i,macroCommand)==B_OK)
	{
		ReIndexCommand(macroCommand);
		reindexMacro->ReplaceMessage("Macro::Commmand",i,macroCommand);
		i++;
	}
}

void PDocLoader::ReIndexCommand(BMessage *commandMessage)
{
	BMessage	*subCommand			= new BMessage();
	void		*nodePointer		= NULL;
	int32		i					= 0;
	char 		*name				= NULL; 
	uint32		type				= B_ANY_TYPE; 
	int32		count				= 0; 
	while (commandMessage->FindPointer("node",i,&nodePointer) == B_OK)
	{
		commandMessage->ReplacePointer("node",i,sorter->ValueFor((int32)nodePointer));
		i++;
	}
	i = 0;
	while (commandMessage->GetInfo(B_MESSAGE_TYPE,i ,(const char **)&name, &type, &count) == B_OK)
	{
		if ( (commandMessage->FindMessage(name,i,subCommand) == B_OK) && (subCommand) )
		{
			ReIndexCommand(subCommand);
			commandMessage->ReplaceMessage(name,count-1,subCommand);
		}
		i++;
	}
/*	while(commandMessage->FindMessage("PCommand::subPCommand",i,subCommand) == B_OK)
	{
		if (subCommand)
		{
			ReIndexCommand(subCommand);
			commandMessage->ReplaceMessage("PCommand::subPCommand",i,subCommand);
		}
		i++;
	}*/
}
