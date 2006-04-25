#include "Select.h"


Select::Select():PCommand()
{
}

void Select::Undo(PDocument *doc,BMessage *undo)
{
	int32 			i					= 0;
	BMessage		*undoMessage		= new BMessage();
	BMessage		*currentContainer	= NULL;
	BList			*changed			= doc->GetChangedNodes();
	BList			*selected			= doc->GetSelected();
	PCommand::Undo(doc,undo);
	undo->FindMessage("Select::Undo" ,undoMessage);
	while (selected->CountItems()>0)
	{
		currentContainer	= (BMessage *)selected->RemoveItem((int32)0);
		currentContainer->ReplaceBool("selected",0,false);
		changed->AddItem(currentContainer);
	}
	i = 0;
	while (	undoMessage->FindPointer("node",i,(void **)&currentContainer) == B_OK)
	{
		i++;
		if (currentContainer)
			currentContainer->ReplaceBool("selected",0,true);
		changed->AddItem(currentContainer);
		selected->AddItem(currentContainer);
	}
	doc->SetModified();
}

BMessage* Select::Do(PDocument *doc, BMessage *settings)
{
	BRect			*selectFrame		= new BRect();
	BMessage		*node				= NULL;
	BMessage		*undoMessage		= new BMessage();
	bool			selectAll			= false;
	bool			deselect			= true;
	int32			i					= 0;
	//sore the old selection and unselect them
	BList			*selected			= doc->GetSelected();
	BList			*changed			= doc->GetChangedNodes();
	status_t		err					= settings->FindBool("deselect",&deselect);
	if ((deselect)|| (err != B_OK))
	{
		while (selected->CountItems()>0)
		{
			node	= (BMessage *)selected->RemoveItem((int32)0);
			if (node != NULL)
			{
				changed->AddItem(node);
				undoMessage->AddPointer("node",node);
				node->ReplaceBool("selected",0,false);
			}
		}
	}
	i = 0;
	while (settings->FindRect("frame",i,selectFrame) == B_OK)
	{
		DoSelect(doc,selectFrame);	
		i++;
	}
	i = 0;
	while (settings->FindPointer("node",i,(void **)&node) == B_OK)
	{
		DoSelect(doc,node);
		i++;
	}
	if (settings->FindBool("selectAll",&selectAll) == B_OK)
		if (selectAll) DoSelectAll(doc);
	settings->RemoveName("Select::Undo");
	settings->AddMessage("Select::Undo",undoMessage);
	settings = PCommand::Do(doc,settings);
	doc->SetModified();
	return settings;
}



void Select::AttachedToManager(void)
{
}

void Select::DetachedFromManager(void)
{
}

void Select::DoSelect(PDocument *doc,BRect *rect)
{
	BList			*all				= doc->GetAllNodes();
	BList			*selected			= doc->GetSelected();
	BList			*changed			= doc->GetChangedNodes();
	BMessage		*currentContainer	= NULL;
	BRect			*frame				= new BRect(0,0,0,0);
	int32 			i					= 0;

	for (i=0;i<all->CountItems();i++)
	{
		currentContainer =(BMessage *) all->ItemAt(i);
		currentContainer->FindRect("Frame",0,frame);
		if (rect->Contains(*frame))
		{
			currentContainer->ReplaceBool("selected",0,true);
			selected->AddItem(currentContainer);
			changed->AddItem(currentContainer);			
		}

	}
}

void Select::DoSelect(PDocument *doc,BMessage *container)
{
	BList			*selected			= doc->GetSelected();
	BList			*changed			= doc->GetChangedNodes();
	bool			selectTester		= false;
	status_t		err					= B_OK;
	err= container->FindBool("selected",&selectTester);
	if (err == B_OK)
		err = container->ReplaceBool("selected",0,true);
	else
		err = container->AddBool("selected",true);
	selected->AddItem(container);
	changed->AddItem(container);
	//container->(new BoolContainer(true));
}

void Select::DoSelectAll(PDocument *doc)
{
	BList			*selected			= doc->GetSelected();
	BList			*all				= doc->GetAllNodes();
	BList			*changed			= doc->GetChangedNodes();

	BMessage		*currentContainer	= NULL;
	int32 			i					= 0;
	for (i=0;i<all->CountItems();i++)
	{
		currentContainer =(BMessage *) all->ItemAt(i);
		currentContainer->ReplaceBool("selected",0,false);
		selected->AddItem(currentContainer);
		changed->AddItem(currentContainer);
	}
}
