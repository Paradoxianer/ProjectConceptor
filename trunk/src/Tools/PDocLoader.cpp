#include "PDocLoader.h"
#include "PDocument.h"

#include <support/Archivable.h>

PDocLoader::PDocLoader(PDocument *doc,BEntry *openEntry)
{
	Init();
	toLoad	= new BFile (openEntry,B_READ_ONLY);
	indexer	= new Indexer(doc);
//	Preprocess();
	Load();
}

PDocLoader::~PDocLoader(void)
{
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
	BMessage	*allConnectionsMessage	= new BMessage();
	BMessage	*selectedMessage		= new BMessage();
	if (toLoad->InitCheck() == B_OK)
	{
		if (loadedStuff->Unflatten(toLoad) == B_OK)
		{
			loadedStuff->FindMessage("PDocument::printerSetting",printerSettings);
			loadedStuff->FindMessage("PDocument::documentSetting",settings);
			loadedStuff->FindMessage("PDocument::allNodes",allNodesMessage);
			loadedStuff->FindMessage("PDocument::commandManager",commandManagerMessage);
			loadedStuff->FindMessage("PDocument::allConnections",allConnectionsMessage);
			loadedStuff->FindMessage("PDocument::selected",selectedMessage);
			
			allNodes		=	Spread(allNodesMessage);
			allConnections	=	ReIndexConnections(allConnectionsMessage);
			selectedNodes	=	ReIndexSelected(selectedMessage);
			undoList		=	ReIndexUndo(commandManagerMessage);
			macroList		=	ReIndexMacro(commandManagerMessage);
		}
	}
}

BList* PDocLoader::Spread(BMessage *allNodeMessage)
{
	BList		*newAllNodes		= new BList();
	BMessage	*node				= new BMessage();
	int32		i					= 0;
	while (allNodeMessage->FindMessage("node",i,node) == B_OK)
	{
		newAllNodes->AddItem(indexer->DeIndexNode(node));
		node =	new BMessage();
		i++;		
	}
	return newAllNodes;
}


BList* PDocLoader::ReIndexConnections(BMessage *allConnectionsMessage)
{
	BMessage	*connection			= new BMessage();
	BList		*newConnections		= new BList();
	int32		i					= 0;
	while (allConnectionsMessage->FindMessage("node",i,connection) == B_OK)
	{
		newConnections->AddItem(indexer->DeIndexConnection(connection));
		connection =	new BMessage();
		i++;
	}
	return newConnections;
}

BList* PDocLoader::ReIndexSelected(BMessage *selectionMessage)
{
	BList		*newSelection		= new BList();
	int32		i					= 0;
	void		*selectPointer		= NULL;
	
	while (selectionMessage->FindPointer("node",i,&selectPointer) == B_OK)
	{
		newSelection->AddItem(indexer->PointerForIndex((int32)selectPointer));
		i++;
	}
	return newSelection;
}

BList* PDocLoader::ReIndexUndo(BMessage *reindexUndo)
{
	BList		*mewUndoList		= new BList();
	BMessage	*undoCommand		= new BMessage();
	int32		i					= 0;
	while (reindexUndo->FindMessage("undo",i,undoCommand)==B_OK)
	{
		mewUndoList->AddItem(indexer->DeIndexUndo(undoCommand));
		undoCommand = new BMessage();
		i++;
	}
	return mewUndoList;
}

BList* PDocLoader::ReIndexMacro(BMessage *reindexMacro)
{
	BList		*newMacroList	= new BList();
	BMessage	*macro			= new BMessage();
	int32		i				= 0;
	while (reindexMacro->FindMessage("macro",i,macro)==B_OK)
	{
		newMacroList->AddItem(indexer->DeIndexMacro(macro));
		macro = new BMessage();
		i++;
	}
	return newMacroList;
}

