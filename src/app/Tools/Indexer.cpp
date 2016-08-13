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
		returnNode->AddPointer("this",node);
			// we need to check for the allNodes List
		if ((returnNode->FindPointer(P_C_NODE_ALLNODES,(void **)&allNodeList) == B_OK) && (allNodeList != NULL) )
		{
			for (i=0; i<allNodeList->CountItems();i++)
			{
				subNode =(BMessage *) allNodeList->ItemAt(i);
				returnNode->AddPointer("allNodesList",subNode);
			}
		}
		if ((returnNode->FindPointer(P_C_NODE_ALLCONNECTIONS,(void **)&allConnectionList) == B_OK) && (allConnectionList != NULL) )
		{
			for (i=0; i<allConnectionList->CountItems();i++)
			{
				subNode =(BMessage *) allConnectionList->ItemAt(i);
				returnNode->AddPointer("allConnectionsList",IndexConnection(subNode));
			}
		}

		//here we also run the node through the available Editors so that they can save all they need :-)
		BList		*editorList	= pluginManager->GetPluginsByType(P_C_EDITOR_PLUGIN_TYPE);
		BasePlugin	*plugin		= NULL;
		PEditor		*editor		= NULL;
		if (editorList)
		{
			for (i=0;i<editorList->CountItems();i++)
			{
				plugin = (BasePlugin *) editorList->ItemAt(i);
				if (plugin)
				{
					editor=(PEditor *)plugin->GetNewObject(NULL);
					editor->PreprocessBeforSave(returnNode);
				}

			}
		}
	}
	return returnNode;
}

BMessage*	Indexer::IndexConnection(BMessage *connection,bool includeNodes)
{
	TRACE();
	BMessage *returnNode	= new BMessage(*connection);
	returnNode->AddPointer("this",connection);
	BMessage *from			= NULL;
	BMessage *to			= NULL;
	if (includeNodes)
	{
		returnNode->FindPointer(P_C_NODE_CONNECTION_FROM,(void **)&from);
		if (!included->HasItem(from))
		{
			returnNode->RemoveName(P_C_NODE_CONNECTION_FROM);
			returnNode->AddMessage(P_C_NODE_CONNECTION_FROM,IndexNode(from));
			included->AddItem(from);
		}
		returnNode->FindPointer(P_C_NODE_CONNECTION_TO,(void **)&to);
		if (!included->HasItem(to))
		{
			returnNode->RemoveName(P_C_NODE_CONNECTION_TO);
			returnNode->AddMessage(P_C_NODE_CONNECTION_TO,IndexNode(to));
			included->AddItem(to);
		}
	}
	else
	{
		//**do we need a check if this node wich we are linking to is indexed??
	}
	BList		*editorList	= pluginManager->GetPluginsByType(P_C_EDITOR_PLUGIN_TYPE);
	BasePlugin	*plugin		= NULL;
	PEditor		*editor		= NULL;
	if (editorList)
	{
		for (int32 i=0;i<editorList->CountItems();i++)
		{
			plugin = (BasePlugin *) editorList->ItemAt(i);
			if (plugin)
			{
				editor=(PEditor *)plugin->GetNewObject(NULL);
				editor->PreprocessBeforSave(returnNode);
			}

		}
	}
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
		while (returnCommand->FindPointer("node",j,(void **)&node) == B_OK)
		{
			if (!included->HasItem(node))
			{
				included->AddItem(node);
				returnCommand->AddMessage("included_node",IndexNode(node));
			}
			j++;
		}
		char		*name	= NULL;
		type_code	type	= 0;
		int32		count	= 0;
		while (returnCommand->GetInfo(B_MESSAGE_TYPE,i ,(char **)&name, &type, &count) == B_OK) {
			if ( (returnCommand->FindMessage(name,count,&subCommand) == B_OK))
			{
				IndexCommand(&subCommand);
				returnCommand->ReplaceMessage(name,count,&subCommand);
			}
			i++;
		}
	}
	return returnCommand;
}


BMessage* Indexer::RegisterDeIndexNode(BMessage *node)
{
	void		*tmpPointer			= NULL;
	node->FindPointer("this",(void **)&tmpPointer);
	node->RemoveName("this");
	sorter[(int32)tmpPointer] = node;
	return node;
}

BMessage* Indexer::DeIndexNode(BMessage *node)
{
	TRACE();
	void		*subContainerEntry	= NULL;
	BList		*allNodesList		= new BList();
	BList		*allConnectionsList	= new BList();
	void		*tmpPointer			= NULL;
	int32		i					= 0;
	map<int32,BMessage*>::iterator	nodeIndex;
	if (node->FindPointer(P_C_NODE_ALLNODES,&tmpPointer) == B_OK)
			node->RemoveName(P_C_NODE_ALLNODES);
	
	while (node->FindPointer("allNodesList",i,&subContainerEntry) == B_OK) {
		nodeIndex = sorter.find((int32)subContainerEntry);
		if (nodeIndex != sorter.end()){
			allNodesList->AddItem(nodeIndex->second);
			//delete just this entry from the Message
			node->RemoveData("allNodesList",i);
		}
	}
	if (allNodesList->CountItems()>0){
			node->AddPointer(P_C_NODE_ALLNODES,allNodesList);
	}
	if (node->FindPointer(P_C_NODE_PARENT,(void **)&tmpPointer) == B_OK) {
		nodeIndex = sorter.find((int32)tmpPointer);
		if (nodeIndex != sorter.end()) {
			node->RemoveName(P_C_NODE_PARENT);
			node->AddPointer(P_C_NODE_PARENT,nodeIndex->second);
		}
	}
	BList		*editorList	= pluginManager->GetPluginsByType(P_C_EDITOR_PLUGIN_TYPE);
	BasePlugin	*plugin		= NULL;
	PEditor		*editor		= NULL;
	if (editorList) {
		for (i=0;i<editorList->CountItems();i++) {
			plugin = (BasePlugin *) editorList->ItemAt(i);
			if (plugin){
				editor=(PEditor *)plugin->GetNewObject(NULL);
				editor->PreprocessAfterLoad(node);
			}
		}
	}
	return node;
}

BMessage* Indexer::DeIndexConnection(BMessage *connection)
{
	TRACE();
	void		*tmpPointer			= NULL;
	if (connection)
	{
		void		*fromPointer		= NULL;
		void		*toPointer			= NULL;
		BMessage	*fromMessage		= new BMessage();
		BMessage	*toMessage			= new BMessage();
		// need to check this for every Pointer individual
		if (connection->FindPointer(P_C_NODE_CONNECTION_FROM,(void **)&fromPointer) != B_OK)
		{
			connection->FindMessage(P_C_NODE_CONNECTION_FROM,fromMessage);
			fromPointer = DeIndexNode(fromMessage);
		}
		if (connection->FindPointer(P_C_NODE_CONNECTION_TO,(void **)&toPointer) != B_OK)
		{
			connection->FindMessage(P_C_NODE_CONNECTION_TO,toMessage);
			toPointer	= DeIndexNode(toMessage);
		}
		connection->RemoveName(P_C_NODE_CONNECTION_FROM);
		connection->RemoveName(P_C_NODE_CONNECTION_TO);
		BList		*editorList	= pluginManager->GetPluginsByType(P_C_EDITOR_PLUGIN_TYPE);
		BasePlugin	*plugin		= NULL;
		PEditor		*editor		= NULL;
		if (editorList!=NULL)
		{
			for (int32 i=0;i<editorList->CountItems();i++)
			{
				plugin = (BasePlugin *) editorList->ItemAt(i);
				if (plugin)
				{
					editor=(PEditor *)plugin->GetNewObject(NULL);
					editor->PreprocessAfterLoad(connection);
				}
			}
		}

//		if we cant find the right Pointer then add the old one
		map<int32,BMessage*>::iterator indexFrom;
		indexFrom=sorter.find((int32)fromPointer);
		if (indexFrom != sorter.end())
			connection->AddPointer(P_C_NODE_CONNECTION_FROM,indexFrom->second);
		else
			connection->AddPointer(P_C_NODE_CONNECTION_FROM,fromPointer);
//		if we cant find the right Pointer then add the old one
		map<int32,BMessage*>::iterator indexTo;
		indexTo=sorter.find((int32)toPointer);
		if (indexTo != sorter.end())
			connection->AddPointer(P_C_NODE_CONNECTION_TO,indexTo->second);
		else
			connection->AddPointer(P_C_NODE_CONNECTION_TO,toPointer);
	}
	connection->FindPointer("this",(void **)&tmpPointer);
	connection->RemoveName("this");
	sorter[(int32)tmpPointer]=connection;
	return connection;
}

BMessage* Indexer::DeIndexCommand(BMessage *command)
{
	TRACE();
	BMessage	*node		= new BMessage();
	BMessage	*subCommand	= new BMessage();
	int			i			= 0;
	// extract all included Nodes :-)
	while (command->FindMessage("included_node",i,node) == B_OK)
	{
		if (node->what == P_C_CLASS_TYPE)
			DeIndexNode(node);
		else if (node->what == P_C_CONNECTION_TYPE)
			DeIndexConnection(node);
		i++;
		node = new BMessage();
	}
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
		if ( (command->FindMessage(name,count-1,subCommand) == B_OK) && (subCommand) )
		{
			DeIndexCommand(subCommand);
			command->ReplaceMessage(name,count-1,subCommand);
		}
		i++;
	}
	i = 0;
	//replace the old Pointer with the new ones
	while (command->FindPointer("node",i,(void **)&node) == B_OK)
	{
		command->ReplacePointer("node",i,sorter[(int32)node]);
		i++;
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
	sorter				= map<int32,BMessage*>();
	included			= new BList();
	pluginManager		= (doc->BelongTo())->GetPluginManager();
}
