#include "Indexer.h"

#include "PDocument.h"
#include "PluginManager.h"
#include "PDocumentManager.h"
#include "BasePlugin.h"
#include "PEditor.h"

Indexer::Indexer(PDocument *document)
{
	doc = document;
	Init();
}

Indexer::~Indexer(void)
{
	delete sorter;
	delete included;

}

BMessage*	Indexer::IndexNode(BMessage *node)
{
	int32	i	= 0;
	if (node!=NULL)
	{
		BList		*subNodeList	= NULL;
		BMessage	*subNode		= NULL;
		node->AddPointer("this",node);
		if ((node->FindPointer("SubContainer",(void **)&subNodeList) == B_OK) && (subNodeList != NULL) )
		{
			for (int32 i=0; i<subNodeList->CountItems();i++)
			{
				subNode =(BMessage *) subNodeList->ItemAt(i);
				node->AddMessage("SubContainerList",IndexNode(subNode));
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
					editor->PreprocessBeforSave(node);
				}
				
			}
		}
	}
	return node;
}

BMessage*	Indexer::IndexConnection(BMessage *connection,bool includeNodes=false)
{
	BMessage *from	= NULL;
	BMessage *to	= NULL;
	if (includeNodes)
	{
		connection->FindPointer("From",(void **)&from);
		if (!included->HasItem(from))
		{
			connection->RemoveName("From");
			connection->AddMessage("From",IndexNode(from));
			included->AddItem(from);
		}
		connection->FindPointer("To",(void **)&to);
		if (!included->HasItem(to))
		{
			connection->RemoveName("To");
			connection->AddMessage("To",IndexNode(to));
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
				editor->PreprocessAfterLoad(connection);
			}
			
		}
	}
	return connection;
}

BMessage*	Indexer::IndexUndo(BMessage *undo,bool includeNodes=false)
{
	int32 i	= 0;
	int32 j = 0;
	if (includeNodes)
	{
		IndexCommand(undo, true);
	}
	return undo;
}

BMessage*	Indexer::IndexMacro(BMessage *macro,bool includeNodes=false)
{
	BMessage	*macroCommand	= new BMessage;
	if (includeNodes)
	{
		while (macro->FindMessage("Macro::Commmand",macroCommand) == B_OK)
		{
			macroCommand = IndexCommand(macroCommand,true);
			macro->ReplaceMessage("Macro::Commmand", macroCommand);
		}
	}
	return macro;
}

BMessage*	Indexer::IndexCommand(BMessage *command,bool includeNodes=false)
{
	BMessage	*node		= NULL;
	BMessage	*subCommand	= NULL;
	int32		i		= 0;
	int32		j		= 0;
	if (includeNodes)
	{
		while (command->FindPointer("node",j,(void **)&node) == B_OK)
		{
			if (!included->HasItem(node))
			{
				included->AddItem(node);
				command->AddMessage("included_node",IndexNode(node));
			}
		}
		char		*name	= NULL;
		type_code	type	= 0;
		int32		count	= 0;
		while (command->GetInfo(B_MESSAGE_TYPE,i ,(const char **)&name, &type, &count) == B_OK)
		{
			if ( (command->FindMessage(name,i,subCommand) == B_OK) && (subCommand) )
			{
				IndexCommand(subCommand);
				command->ReplaceMessage(name,count-1,subCommand);
			}
			i++;
		}
	}
	return command;
}

			
BMessage* Indexer::DeIndexNode(BMessage *node)
{
	BMessage	*subContainerEntry	= new BMessage();
	BList		*subContainerList	= new BList();
	int32		i					= 0;
	void		*tmpPointer			= NULL;
	if (node->FindPointer("SubContainer",&tmpPointer) == B_OK)
			node->RemoveName("SubContainer");
	while (node->FindMessage("SubContainerList",i,subContainerEntry) == B_OK)
	{
		subContainerList->AddItem(DeIndexNode(subContainerEntry));
		subContainerEntry	= new BMessage();
		i++;
	}
	if (subContainerList->CountItems()>0)
		node->AddPointer("SubContainer",subContainerList);
	node->FindPointer("this",(void **)&tmpPointer);
	node->RemoveName("this");
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
	sorter->AddItem((int32)tmpPointer,node);
	return node;
}

BMessage* Indexer::DeIndexConnection(BMessage *connection)
{
	if (connection)
	{
		void		*fromPointer		= NULL;
		void		*toPointer			= NULL;
		BMessage	*fromMessage		= new BMessage();
		BMessage	*toMessage			= new BMessage();	
		// need to check this for every Pointer individual 
		if (connection->FindPointer("From",(void **)&fromPointer) != B_OK)
		{
			connection->FindMessage("From",fromMessage);
			connection->RemoveName("From");
			fromPointer = DeIndexNode(fromMessage);
		}
		if (connection->FindPointer("To",(void **)&toPointer) != B_OK)
		{
			connection->FindMessage("To",toMessage);
			connection->RemoveName("To");
			toPointer	= DeIndexNode(toMessage);
		}
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
		connection->AddPointer("From",sorter->ValueFor((int32)fromPointer));
		connection->AddPointer("To",sorter->ValueFor((int32)toPointer));
	}
	return connection;
}

BMessage* Indexer::DeIndexCommand(BMessage *command)
{
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
	while (command->GetInfo(B_MESSAGE_TYPE,i ,(const char **)&name, &type, &count) == B_OK)
	{
		if ( (command->FindMessage(name,i,subCommand) == B_OK) && (subCommand) )
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
		command->ReplacePointer("node",i,sorter->ValueFor((int32)node));
		i++;
	}
	return command;
}

BMessage* Indexer::DeIndexUndo(BMessage *undo)
{
	// extract saved nodes and because undo is only a saved Commandstructure with some undo messages (wich DeIndexCommand can handle we use DeindexCommand )
	DeIndexCommand(undo);
	return undo;
}
BMessage* Indexer::DeIndexMacro(BMessage *macro)
{
	BMessage	*macroCommand	= new BMessage();
	while (macro->FindMessage("Macro::Commmand",macroCommand) == B_OK)
	{
		macroCommand = DeIndexCommand(macroCommand);
		macro->ReplaceMessage("Macro::Commmand", macroCommand);
	}
	return macro;
}


void Indexer::Init(void)
{
	sorter				= new BKeyedVector<int32,BMessage*>();
	included			= new BList;
	pluginManager		= (doc->BelongTo())->GetPluginManager();
}