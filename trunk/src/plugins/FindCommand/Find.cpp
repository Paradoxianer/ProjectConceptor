#include "Find.h"


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
		currentContainer->ReplaceBool("Node::selected",0,false);
	}
	i = 0;
	while (	undo->FindPointer("node",i,(void **)&currentContainer) == B_OK)
	{
		i++;
		if (currentContainer)
			currentContainer->ReplaceBool("Node::selected",0,true);	
	}
}

BMessage* Find::Do(PDocument *doc, BMessage *settings)
{
	BRect			*selectFrame		= new BRect();
	BMessage		*node				= NULL;
	BMessage		*commandMessage		= new BMessage();
	bool			selectAll			= false;
	bool			deselect			= true;
	int32			i					= 0;
	//sore the old selection and unselect them
	BList			*selected			= doc->GetSelected();
	set<BMessage*>		*changed			= doc->GetChangedNodes();
	status_t		err					= settings->FindBool("deselect",&deselect);
	if ((deselect)|| (err != B_OK))
	{
		while (selected->CountItems()>0)
		{
			node	= (BMessage *)selected->RemoveItem((int32)0);
			if (node != NULL)
			{
				changed->insert(node);
				commandMessage->AddPointer("node",node);
				node->ReplaceBool("Node::selected",0,false);
			}
		}
	}
	i = 0;
	while (settings->FindRect("frame",i,selectFrame) == B_OK)
	{
		DoFind(doc,selectFrame);	
		i++;
	}
	i = 0;
	while (settings->FindPointer("node",i,(void **)&node) == B_OK)
	{
		DoFind(doc,node);
		i++;
	}
	if (settings->FindBool("selectAll",&selectAll) == B_OK)
		if (selectAll) DoFindAll(doc);
	commandMessage	= PCommand::Do(doc,settings);
	doc->SetModified();
	return commandMessage;
}



void Find::AttachedToManager(void)
{
}

void Find::DetachedFromManager(void)
{
}

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
		currentContainer->FindRect("Node::frame",0,frame);
		if (rect->Contains(*frame))
		{
			currentContainer->ReplaceBool("Node::selected",0,true);
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
	err= container->FindBool("Node::selected",&selectTester);
	if (err == B_OK)
		err = container->ReplaceBool("Node::selected",0,true);
	else
		err = container->AddBool("Node::selected",true);
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
		currentContainer->ReplaceBool("Node::selected",0,false);
		selected->AddItem(currentContainer);
		changed->insert(currentContainer);
	}
}
