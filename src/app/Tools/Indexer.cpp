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
		if ((returnNode->FindPointer("Node::allNodes",(void **)&allNodeList) == B_OK) && (allNodeList != NULL) )
		{
			for (i=0; i<allNodeList->CountItems();i++)
			{
				subNode =(BMessage *) allNodeList->ItemAt(i);
				returnNode->AddPointer("allNodesList",subNode);
			}
		}
		if ((returnNode->FindPointer("Node::allConnections",(void **)&allConnectionList) == B_OK) && (allConnectionList != NULL) )
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
		returnNode->FindPointer("Node::from",(void **)&from);
		if (!included->HasItem(from))
		{
			returnNode->RemoveName("Node::from");
			returnNode->AddMessage("Node::from",IndexNode(from));
			included->AddItem(from);
		}
		returnNode->FindPointer("Node::to",(void **)&to);
		if (!included->HasItem(to))
		{
			returnNode->RemoveName("Node::to");
			returnNode->AddMessage("Node::to",IndexNode(to));
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
#ifdef B_ZETA_VERSION_1_0_0
		while (returnCommand->GetInfo(B_MESSAGE_TYPE,i ,(const char **)&name, &type, &count) == B_OK)
#else
		while (returnCommand->GetInfo(B_MESSAGE_TYPE,i ,(char **)&name, &type, &count) == B_OK)
#endif

		{
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
	//@todo introduce a noneindexd list as soon as we are index this node
	//we should scan throught the none_indexed list to find all
	//nodes where this node pointer wasnt replaced yet
	return node;
}


BMessage* Indexer::DeIndexNode(BMessage *node)
{
	TRACE();
	void		*subContainerEntry	= NULL;
	BList		*allNodesList		= new BList();
	BList		*allConnectionsList	= new BList();
	int32		i					= 0;
	void		*tmpPointer			= NULL;
	map<int32,BMessage*>::iterator	nodeIndex;
	i = 0;
	if (node->FindPointer("Node::allNodes",&tmpPointer) == B_OK)
			node->RemoveName("Node::allNodes");
	while (node->FindPointer("allNodesList",i,&subContainerEntry) == B_OK)
	{
		nodeIndex = sorter.find((int32)subContainerEntry);
		if (nodeIndex != sorter.end())
			allNodesList->AddItem(nodeIndex->second);
		else 
			notIndexed.insert(pair<int32,BMessage*>((int32)tmpPointer,node));

		i++;
	}
	if (allNodesList->CountItems()>0)
	{
		node->AddPointer("Node::allNodes",allNodesList);
		node->RemoveName("allNodesList");
	}
	i = 0;
	if (node->FindPointer("Node::allConnections",&tmpPointer) == B_OK)
			node->RemoveName("Node::allConnections");
	while (node->FindMessage("allConnectionsList",i,(BMessage *)subContainerEntry) == B_OK)
	{
		allConnectionsList->AddItem(DeIndexNode((BMessage *)subContainerEntry));
		subContainerEntry	= new BMessage();
		i++;
	}
	if (allConnectionsList->CountItems()>0)
	{
		node->AddPointer("Node::allConnections",allConnectionsList);
		node->RemoveName("allConnectionsList");
	}
	if (node->FindPointer("Node::parent",(void **)&tmpPointer) == B_OK)
	{
		nodeIndex = sorter.find((int32)subContainerEntry);
		if (nodeIndex != sorter.end())
		{
			node->RemoveName("Node::parent");
			node->AddPointer("Node::parent",nodeIndex->second);
		}
		else
			notIndexed.insert(pair<int32,BMessage*>((int32)tmpPointer,node));
	}
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
				editor->PreprocessAfterLoad(node);
			}
		}
	}
	return RegisterDeIndexNode(node);;
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
		if (connection->FindPointer("Node::from",(void **)&fromPointer) != B_OK)
		{
			connection->FindMessage("Node::from",fromMessage);
			fromPointer = DeIndexNode(fromMessage);
		}
		if (connection->FindPointer("Node::to",(void **)&toPointer) != B_OK)
		{
			connection->FindMessage("Node::to",toMessage);
			toPointer	= DeIndexNode(toMessage);
		}
		connection->RemoveName("Node::from");
		connection->RemoveName("Node::to");
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
			connection->AddPointer("Node::from",indexFrom->second);
		else
			connection->AddPointer("Node::from",fromPointer);
//		if we cant find the right Pointer then add the old one
		map<int32,BMessage*>::iterator indexTo;
		indexTo=sorter.find((int32)toPointer);
		if (indexTo != sorter.end())
			connection->AddPointer("Node::to",indexTo->second);
		else
			connection->AddPointer("Node::to",toPointer);
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
	notIndexed			= multimap<int32,BMessage*>();
	included			= new BList();
	pluginManager		= (doc->BelongTo())->GetPluginManager();
}
