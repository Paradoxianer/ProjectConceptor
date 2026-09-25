#include "Indexer.h"

#include "PDocument.h"
#include "PluginManager.h"
#include "PDocumentManager.h"
#include "BasePlugin.h"
#include "PEditor.h"


Indexer::Indexer(PDocument *document)
{
	TRACE();
	doc = document;
	Init();
}

Indexer::~Indexer(void)
{
	TRACE();
	delete included;
	if (cachedEditors != NULL) {
		for (int32 i = 0; i < cachedEditors->CountItems(); i++)
			delete (PEditor*)cachedEditors->ItemAt(i);
		delete cachedEditors;
	}
}

BList* Indexer::GetCachedEditors(void)
{
	if (cachedEditors != NULL)
		return cachedEditors;
	cachedEditors = new BList();
	BList		*editorPlugins	= pluginManager->GetPluginsByType(P_C_EDITOR_PLUGIN_TYPE);
	BasePlugin	*plugin			= NULL;
	PEditor		*editor			= NULL;
	if (editorPlugins) {
		for (int32 i = 0; i < editorPlugins->CountItems(); i++) {
			plugin = (BasePlugin *) editorPlugins->ItemAt(i);
			if (plugin) {
				editor = (PEditor *)plugin->GetNewObject(NULL);
				if (editor)
					cachedEditors->AddItem(editor);
			}
		}
	}
	return cachedEditors;
}

BMessage*	Indexer::IndexNode(BMessage *node)
{
	TRACE();
	BMessage	*returnNode	= NULL;
	int32		i			= 0;
	if (node!=NULL)
	{
		BList		*allNodeList		= NULL;
		BList		*allConnectionList	= NULL;

		BMessage	*subNode			= NULL;

		if (!included->HasItem(node))
			included->AddItem(node);
		returnNode = new BMessage(*node);
		returnNode->AddInt32("this",IdFor(node));
			// we need to check for the allNodes List
		if ((returnNode->FindPointer(P_C_NODE_ALLNODES,(void **)&allNodeList) == B_OK) && (allNodeList != NULL) )
		{
			returnNode->RemoveName(P_C_NODE_ALLNODES);
			for (i=0; i<allNodeList->CountItems();i++)
			{
				subNode =(BMessage *) allNodeList->ItemAt(i);
				returnNode->AddInt32("allNodesList",IdFor(subNode));
			}
		}
		if ((returnNode->FindPointer(P_C_NODE_ALLCONNECTIONS,(void **)&allConnectionList) == B_OK) && (allConnectionList != NULL) )
		{
			returnNode->RemoveName(P_C_NODE_ALLCONNECTIONS);
			for (i=0; i<allConnectionList->CountItems();i++)
			{
				subNode =(BMessage *) allConnectionList->ItemAt(i);
				returnNode->AddInt32("allConnectionsList",IdFor(subNode));
			}
		}
		void	*parent	= NULL;
		if (returnNode->FindPointer(P_C_NODE_PARENT,&parent) == B_OK)
		{
			returnNode->RemoveName(P_C_NODE_PARENT);
			returnNode->AddInt32(P_C_NODE_PARENT,IdFor((BMessage*)parent));
		}

		//here we also run the node through the available Editors so that they can save all they need :-)
		BList	*editors	= GetCachedEditors();
		for (i=0;i<editors->CountItems();i++)
			((PEditor *)editors->ItemAt(i))->PreprocessBeforSave(returnNode);
	}
	return returnNode;
}

BMessage*	Indexer::IndexConnection(BMessage *connection,bool includeNodes)
{
	TRACE();
	BMessage *returnNode	= new BMessage(*connection);
	returnNode->AddInt32("this",IdFor(connection));
	BMessage *from			= NULL;
	BMessage *to			= NULL;
	if (includeNodes)
	{
		// either branch must always remove the raw pointer field - an
		// endpoint already indexed elsewhere (e.g. its own node came
		// first in the same command's "node" list, the common case: you
		// draw the nodes, then connect them) used to leave the field
		// completely untouched here, still a live pointer nothing on the
		// replay side can resolve - the same unconvertable-pointer crash
		// this whole function exists to prevent, just for the "already
		// included" case instead of the "not yet included" one.
		returnNode->FindPointer(P_C_NODE_CONNECTION_FROM,(void **)&from);
		returnNode->RemoveName(P_C_NODE_CONNECTION_FROM);
		if (!included->HasItem(from))
		{
			returnNode->AddMessage(P_C_NODE_CONNECTION_FROM,IndexNode(from));
			included->AddItem(from);
		}
		else
			returnNode->AddInt32(P_C_NODE_CONNECTION_FROM,IdFor(from));

		returnNode->FindPointer(P_C_NODE_CONNECTION_TO,(void **)&to);
		returnNode->RemoveName(P_C_NODE_CONNECTION_TO);
		if (!included->HasItem(to))
		{
			returnNode->AddMessage(P_C_NODE_CONNECTION_TO,IndexNode(to));
			included->AddItem(to);
		}
		else
			returnNode->AddInt32(P_C_NODE_CONNECTION_TO,IdFor(to));
	}
	else
	{
		void	*fromPointer	= NULL;
		void	*toPointer		= NULL;
		if (returnNode->FindPointer(P_C_NODE_CONNECTION_FROM,&fromPointer) == B_OK)
		{
			returnNode->RemoveName(P_C_NODE_CONNECTION_FROM);
			returnNode->AddInt32(P_C_NODE_CONNECTION_FROM,IdFor((BMessage*)fromPointer));
		}
		if (returnNode->FindPointer(P_C_NODE_CONNECTION_TO,&toPointer) == B_OK)
		{
			returnNode->RemoveName(P_C_NODE_CONNECTION_TO);
			returnNode->AddInt32(P_C_NODE_CONNECTION_TO,IdFor((BMessage*)toPointer));
		}
	}
	BList	*editors	= GetCachedEditors();
	for (int32 i=0;i<editors->CountItems();i++)
		((PEditor *)editors->ItemAt(i))->PreprocessBeforSave(returnNode);
	return returnNode;
}

BMessage*	Indexer::IndexUndo(BMessage *undo,bool includeNodes)
{
	TRACE();
	if (includeNodes)
	{
		IndexCommand(undo);
	}
	return undo;
}

BMessage*	Indexer::IndexMacroCommand(BMessage *macro)
{
	char 		*commandName		= NULL;
	if (macro->FindString("Command::Name",(const char **)&commandName) == B_OK)
	{
		BMessage	*macroCommand		= new BMessage(*macro);
		macroCommand = IndexCommand(macroCommand,true);
		return macroCommand;
	}
	else
		return macro;
}


BMessage*	Indexer::IndexCommand(BMessage *command,bool includeNodes)
{
	TRACE();
	BMessage	*returnCommand	= new BMessage(*command);
	BMessage	*node			= NULL;
	BMessage	subCommand;
	int32		i				= 0;
	int32		j				= 0;
	if (includeNodes)
	{
		BList	nodePointers;
		while (returnCommand->FindPointer("node",j,(void **)&node) == B_OK)
		{
			if (!included->HasItem(node))
			{
				included->AddItem(node);
				// a command's "node" field holds both plain nodes and
				// connections (see e.g. Insert::Do()) - IndexNode() has no
				// idea how to convert a connection's own
				// P_C_NODE_CONNECTION_FROM/TO pointer fields, so a
				// connection indexed through it here used to keep those as
				// live, unconvertable pointers all the way into the stored
				// macro text - confirmed via a live crash on replay:
				// ConnectionRenderer::Init() dereferencing the (never
				// resolved) from/to.
				if (node->what == P_C_CONNECTION_TYPE)
					returnCommand->AddMessage("included_node",IndexConnection(node,true));
				else
					returnCommand->AddMessage("included_node",IndexNode(node));
			}
			nodePointers.AddItem(node);
			j++;
		}
		if (nodePointers.CountItems() > 0)
		{
			returnCommand->RemoveName("node");
			for (j=0; j<nodePointers.CountItems(); j++)
				returnCommand->AddInt32("node",IdFor((BMessage*)nodePointers.ItemAt(j)));
		}
		char		*name	= NULL;
		type_code	type	= 0;
		int32		count	= 0;
		while (returnCommand->GetInfo(B_MESSAGE_TYPE,i ,(char **)&name, &type, &count) == B_OK) {
			if ( (returnCommand->FindMessage(name,count-1,&subCommand) == B_OK))
			{
				BMessage	*indexedSub	= IndexCommand(&subCommand,true);
				returnCommand->ReplaceMessage(name,count-1,indexedSub);
				delete indexedSub;
			}
			i++;
		}
	}
	return returnCommand;
}


BMessage* Indexer::RegisterDeIndexNode(BMessage *node)
{
	int32		id				= 0;
	node->FindInt32("this",&id);
	node->RemoveName("this");
	sorter[id] = node;
	return node;
}

BMessage* Indexer::DeIndexNode(BMessage *node)
{
	TRACE();
	int32		subId				= 0;
	int32		parentId			= 0;
	BList		*allNodesList		= new BList();
	int32		i					= 0;
	std::map<int32,BMessage*>::iterator	nodeIndex;

	while (node->FindInt32("allNodesList",i,&subId) == B_OK) {
		nodeIndex = sorter.find(subId);
		if (nodeIndex != sorter.end()){
			allNodesList->AddItem(nodeIndex->second);
			//delete just this entry from the Message
			node->RemoveData("allNodesList",i);
		} else {
			PRINT(("ERROR:\tDeIndexNode - unresolved allNodesList id %ld\n",(long)subId));
			i++;
		}
	}
	if (allNodesList->CountItems()>0){
			node->AddPointer(P_C_NODE_ALLNODES,allNodesList);
	}
	if (node->FindInt32(P_C_NODE_PARENT,&parentId) == B_OK) {
		node->RemoveName(P_C_NODE_PARENT);
		nodeIndex = sorter.find(parentId);
		if (nodeIndex != sorter.end()) {
			node->AddPointer(P_C_NODE_PARENT,nodeIndex->second);
		} else {
			PRINT(("ERROR:\tDeIndexNode - unresolved parent id %ld\n",(long)parentId));
		}
	}
	BList	*editors	= GetCachedEditors();
	for (i=0;i<editors->CountItems();i++)
		((PEditor *)editors->ItemAt(i))->PreprocessAfterLoad(node);
	return node;
}

BMessage* Indexer::DeIndexConnection(BMessage *connection)
{
	TRACE();
	int32		id				= 0;
	if (connection)
	{
		int32		fromId			= 0;
		int32		toId			= 0;
		BMessage	*fromMessage	= new BMessage();
		BMessage	*toMessage		= new BMessage();
		BMessage	*resolvedFrom	= NULL;
		BMessage	*resolvedTo		= NULL;
		// two distinct cases: an embedded, already-indexed node (includeNodes
		// path, e.g. clipboard copy) or a plain id reference into "sorter"
		// (the normal save-to-file path) - never both for the same field.
		bool		haveFromId		= (connection->FindInt32(P_C_NODE_CONNECTION_FROM,&fromId) == B_OK);
		bool		haveToId		= (connection->FindInt32(P_C_NODE_CONNECTION_TO,&toId) == B_OK);
		// RegisterDeIndexNode() first, same reason as everywhere else this
		// pattern appears: this embedded node's own id has to be in
		// `sorter` before it's resolved, both for its own parent/children
		// (DeIndexNode()) and in case something else later in this same
		// macro command references it by plain id.
		if (!haveFromId && connection->FindMessage(P_C_NODE_CONNECTION_FROM,fromMessage) == B_OK)
			resolvedFrom = DeIndexNode(RegisterDeIndexNode(fromMessage));
		if (!haveToId && connection->FindMessage(P_C_NODE_CONNECTION_TO,toMessage) == B_OK)
			resolvedTo = DeIndexNode(RegisterDeIndexNode(toMessage));
		connection->RemoveName(P_C_NODE_CONNECTION_FROM);
		connection->RemoveName(P_C_NODE_CONNECTION_TO);
		BList	*editors	= GetCachedEditors();
		for (int32 i=0;i<editors->CountItems();i++)
			((PEditor *)editors->ItemAt(i))->PreprocessAfterLoad(connection);

		if (haveFromId) {
			std::map<int32,BMessage*>::iterator indexFrom = sorter.find(fromId);
			if (indexFrom != sorter.end())
				resolvedFrom = indexFrom->second;
			else
				PRINT(("ERROR:\tDeIndexConnection - unresolved from id %ld\n",(long)fromId));
		}
		if (resolvedFrom)
			connection->AddPointer(P_C_NODE_CONNECTION_FROM,resolvedFrom);

		if (haveToId) {
			std::map<int32,BMessage*>::iterator indexTo = sorter.find(toId);
			if (indexTo != sorter.end())
				resolvedTo = indexTo->second;
			else
				PRINT(("ERROR:\tDeIndexConnection - unresolved to id %ld\n",(long)toId));
		}
		if (resolvedTo)
			connection->AddPointer(P_C_NODE_CONNECTION_TO,resolvedTo);
	}
	connection->FindInt32("this",&id);
	connection->RemoveName("this");
	sorter[id]=connection;
	return connection;
}

BMessage* Indexer::DeIndexCommand(BMessage *command)
{
	TRACE();
	BMessage	*node		= new BMessage();
	BMessage	*subCommand	= new BMessage();
	int			i			= 0;
	// extract all included Nodes - register every included node's own id
	// into `sorter` before resolving any of them (RegisterDeIndexNode()
	// then DeIndexNode(), per the contract documented on the class above,
	// and the same two-pass shape Paste.cpp/PDocLoader.cpp already use).
	// A node can be included alongside a sibling that references it as
	// its parent (e.g. a group and its child recorded in the same
	// command), so resolving in the same pass as registering could hit an
	// unregistered forward reference - and, more basically, this command's
	// own top-level "node" id (resolved further below) can never find this
	// node in `sorter` at all without the registration happening first.
	// Connections need the same deferral for the same reason, one level
	// further out: DeIndexConnection() resolves its endpoints against
	// `sorter` too, so a connection listed before the plain nodes it
	// connects (a real, unremarkable ordering - included_node just follows
	// whatever order IndexCommand() first saw each "node" pointer in) used
	// to hit the exact same unresolved-id failure - confirmed via a live
	// crash: ConnectionRenderer::Init() dereferences the (then absent)
	// P_C_NODE_CONNECTION_FROM/TO fields unconditionally.
	BList	includedNodes;
	BList	includedConnections;
	while (command->FindMessage("included_node",i,node) == B_OK)
	{
		if (node->what == P_C_CONNECTION_TYPE)
			includedConnections.AddItem(node);
		else {
			RegisterDeIndexNode(node);
			includedNodes.AddItem(node);
		}
		i++;
		node = new BMessage();
	}
	for (int32 k = 0; k < includedNodes.CountItems(); k++)
		DeIndexNode((BMessage*)includedNodes.ItemAt(k));
	for (int32 k = 0; k < includedConnections.CountItems(); k++)
		DeIndexConnection((BMessage*)includedConnections.ItemAt(k));
	i = 0;
	// go through all added Subcommands and Undo Messagefields
	command->RemoveName("included_node");
	char		*name	= NULL;
	type_code	type	= 0;
	int32		count	= 0;
#ifdef B_ZETA_VERSION_1_0_0
	while (command->GetInfo(B_MESSAGE_TYPE,i ,(const char **)&name, &type, &count) == B_OK)
#else
	while (command->GetInfo(B_MESSAGE_TYPE,i ,(char **)&name, &type, &count) == B_OK)
#endif

	{
		// every entry of a repeated field (PCommand::subPCommand has one per
		// child) - only the last one used to be resolved, so the earlier
		// children of a Batch/Repeat kept unresolved ids and did nothing
		for (int32 entry = 0; entry < count; entry++) {
			if ( (command->FindMessage(name,entry,subCommand) == B_OK) && (subCommand) )
			{
				DeIndexCommand(subCommand);
				command->ReplaceMessage(name,entry,subCommand);
			}
		}
		i++;
	}
	i = 0;
	// "node" is stored as int32 ids on disk (AddInt32 in IndexCommand) but
	// consumers like PCommand::Undo() expect live BMessage* pointers -
	// ReplacePointer() would fail with B_BAD_TYPE against an int32 field, so
	// the field is rebuilt from scratch rather than replaced in place.
	int32			nodeId	= 0;
	BList			resolvedNodes;
	while (command->FindInt32("node",i,&nodeId) == B_OK)
	{
		std::map<int32,BMessage*>::iterator	nodeIndex	= sorter.find(nodeId);
		if (nodeIndex != sorter.end())
			resolvedNodes.AddItem(nodeIndex->second);
		else {
			unresolvedCount++;
			PRINT(("ERROR:\tDeIndexCommand - unresolved node id %ld\n",(long)nodeId));
		}
		i++;
	}
	if (i > 0)
	{
		command->RemoveName("node");
		for (int32 k=0; k<resolvedNodes.CountItems(); k++)
			command->AddPointer("node",resolvedNodes.ItemAt(k));
	}
	return command;
}

BMessage* Indexer::DeIndexUndo(BMessage *undo)
{
	TRACE();
	// extract saved nodes and because undo is only a saved Commandstructure with some undo messages (wich DeIndexCommand can handle we use DeindexCommand )
	return DeIndexCommand(undo);
}


void Indexer::Init(void)
{
	TRACE();
	sorter				= std::map<int32,BMessage*>();
	ids					= std::map<BMessage*,int32>();
	nextId				= 1;
	unresolvedCount		= 0;
	included			= new BList();
	pluginManager		= (doc->BelongTo())->GetPluginManager();
	cachedEditors		= NULL;
}

int32 Indexer::IdFor(BMessage *node)
{
	if (node == NULL)
		return 0;
	std::map<BMessage*,int32>::iterator it = ids.find(node);
	if (it != ids.end())
		return it->second;
	int32	id	= nextId++;
	ids[node]	= id;
	return id;
}

bool Indexer::ResolveId(int32 id,BMessage **node)
{
	std::map<int32,BMessage*>::iterator it = sorter.find(id);
	if (it == sorter.end())
		return false;
	*node = it->second;
	return true;
}
