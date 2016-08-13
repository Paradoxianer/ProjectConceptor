#include <String.h>

#include "Find.h"
#include "FindWindow.h"


Find::Find():PCommand()
{
}

void Find::Undo(PDocument *doc,BMessage *undo)
{
	PCommand::Undo(doc,undo);
	int32 			i					= 0;
	BMessage		*currentContainer	= NULL;
	BList			*selected			= doc->GetSelected();
	for (i=0;i<selected->CountItems();i++)
	{
		currentContainer	= (BMessage *)selected->ItemAt(i);
		currentContainer->ReplaceBool(P_C_NODE_SELECTED,0,false);
	}
	i = 0;
	while (	undo->FindPointer("node",i,(void **)&currentContainer) == B_OK)
	{
		i++;
		if (currentContainer)
			currentContainer->ReplaceBool(P_C_NODE_SELECTED,0,true);	
	}
}

BMessage* Find::Do(PDocument *doc, BMessage *settings)
{
	BRect			*selectFrame		= new BRect();
	BMessage		*node				= NULL;
	BMessage		*commandMessage		= new BMessage();
	bool			selectAll			= false;
	int32			i					= 0;
	BString			*findString			= new BString();
	//sore the old selection and unselect them
	BList			*selected			= doc->GetSelected();
	set<BMessage*>	*changed			= doc->GetChangedNodes();
	status_t		err					= B_OK;
	
	//if there is no findString we will show a window wich will generate a proper "searchString command on its own :)
	if (settings->FindString("searchString",findString)!=B_OK)
	{
		FindWindow *findWindow = new FindWindow(doc);
		findWindow->Show();
		return NULL;
	}	
	else
	{
		//first deselct all nodes
		while (selected->CountItems()>0)
		{
			node	= (BMessage *)selected->RemoveItem((int32)0);
			if (node != NULL)
			{
				changed->insert(node);
				commandMessage->AddPointer("node",node);
				node->ReplaceBool(P_C_NODE_SELECTED,0,false);
			}
		}
		//then find all nodes
		BList *foundList=FindNodes(doc, findString);
		//select them all
		for (int i =0 ; i<foundList->CountItems();i++)
		{
			node = (BMessage *)foundList->ItemAt(i);	
			node->ReplaceBool(P_C_NODE_SELECTED,0,true);
			selected->AddItem(node);
			changed->insert(node);
		}
		commandMessage	= PCommand::Do(doc,settings);
		doc->SetModified();	
		return commandMessage;
	}
}

void Find::AttachedToManager(void)
{
}

void Find::DetachedFromManager(void)
{
}

BList* Find::FindNodes(PDocument *doc,BString *search)
{
	BList		*nodesFound			= new BList();
	BList		*all				= doc->GetAllNodes();
	BMessage	*currentContainer	= NULL;
	int32		i					= 0;
	for (i=0;i<all->CountItems();i++)
	{
		currentContainer =(BMessage *) all->ItemAt(i);
		if (FindInNode(currentContainer, search)== true)
			nodesFound->AddItem(currentContainer);
	}
	return nodesFound;
}

bool Find::FindInNode(BMessage *node,BString *search)
{
	char		*attribName		= NULL;
	BMessage	*attribMessage	= new BMessage();
	BString		*dataString		= new BString();	
	uint32		type			= B_ANY_TYPE;
	int32		count			= 0;
	bool		found			= false;
	int32		i				= 0;
	//first iterate through all Strings
	while ((node->GetInfo(B_STRING_TYPE, i,(char **) &attribName, &type, &count) == B_OK) && !found)
	{
		if (node->FindString(attribName,count-1,dataString)==B_OK)
		{
			found = dataString->FindFirst(*search)!=B_ERROR;
		}
		i++;
	}
	//check all subnodes / sub bmessages
	i=0;
	while ((node->GetInfo(B_MESSAGE_TYPE, i,(char **) &attribName, &type, &count) == B_OK) && !found)
	{
		if ((node->FindMessage(attribName,count-1,attribMessage) == B_OK) && (attribMessage != NULL))
			found = FindInNode(attribMessage, search);
		i++;
	}
	return found;
}


/*
void Find::DoFind(PDocument *doc,BRect *rect)
{
	BList			*all				= doc->GetAllNodes();
	BList			*selected			= doc->GetSelected();
	set<BMessage*>		*changed			= doc->GetChangedNodes();
	BMessage		*currentContainer	= NULL;
	BRect			*frame				= new BRect(0,0,0,0);
	int32 			i					= 0;

	for (i=0;i<all->CountItems();i++)
	{
		currentContainer =(BMessage *) all->ItemAt(i);
		currentContainer->FindRect(P_C_NODE_FRAME,0,frame);
		if (rect->Contains(*frame))
		{
			currentContainer->ReplaceBool(P_C_NODE_SELECTED,0,true);
			selected->AddItem(currentContainer);
			changed->insert(currentContainer);
		}

	}
}

void Find::DoFind(PDocument *doc,BMessage *container)
{
	BList			*selected			= doc->GetSelected();
	set<BMessage*>		*changed			= doc->GetChangedNodes();
	bool			selectTester		= false;
	status_t		err					= B_OK;
	err= container->FindBool(P_C_NODE_SELECTED,&selectTester);
	if (err == B_OK)
		err = container->ReplaceBool(P_C_NODE_SELECTED,0,true);
	else
		err = container->AddBool(P_C_NODE_SELECTED,true);
	selected->AddItem(container);
	changed->insert(container);
	//container->(new BoolContainer(true));
}

void Find::DoFindAll(PDocument *doc)
{
	BList			*selected			= doc->GetSelected();
	BList			*all				= doc->GetAllNodes();
	set<BMessage*>		*changed			= doc->GetChangedNodes();

	BMessage		*currentContainer	= NULL;
	int32 			i					= 0;
	for (i=0;i<all->CountItems();i++)
	{
		currentContainer =(BMessage *) all->ItemAt(i);
		currentContainer->ReplaceBool(P_C_NODE_SELECTED,0,false);
		selected->AddItem(currentContainer);
		changed->insert(currentContainer);
	}
}
*/
