#include "ProjectConceptorDefs.h"
#include "Resize.h"


Resize::Resize():PCommand()
{
}

void Resize::Undo(PDocument *doc,BMessage *undo)
{
	BMessage		*undoMessage		= new BMessage();
	BList			*changed			= doc->GetChangedNodes();
	BRect			*oldFrame			= new BRect(0,0,100,100);
	BMessage		*node				= NULL;
	BMessage		*settings			= new BMessage();
	int32			i					= 0;
	PCommand::Undo(doc,undo);
	undo->FindMessage("Resize::Undo" ,undoMessage);
	//settings is placed by the default implentation of PCommand::Do(...) into the undo MEssage
	while (undoMessage->FindPointer("node",i,(void **)&node) == B_OK)
	{
		if (undoMessage->FindRect("oldFrame",i,oldFrame) == B_OK)
		{
			node->ReplaceRect("Frame",*oldFrame);
			changed->AddItem(node);
		}
		i++;
	}
	doc->SetModified();
}

BMessage* Resize::Do(PDocument *doc, BMessage *settings)
{
	BMessage		*undoMessage		= new BMessage();
	BList			*selected			= doc->GetSelected();
	BList			*changed			= doc->GetChangedNodes();
	float			dx,dy;
	BRect			*newFrame			= new BRect(0,0,100,100);
	BRect			*oldFrame			= new BRect(0,0,100,100);
	BMessage		*node				= new BMessage();
	int32			i					= 0;
	if ( (settings->FindFloat("dx",&dx)==B_OK) && (settings->FindFloat("dy",&dy)==B_OK) )
	{
		for (i=0;i<selected->CountItems();i++)
		{
			node=(BMessage *)selected->ItemAt(i);
			node->FindRect("Frame",oldFrame);
			undoMessage->AddRect("oldFrame",*oldFrame);
			undoMessage->AddPointer("node",node);
			*newFrame			=	*oldFrame;
			newFrame->right		+=	dx;
			newFrame->bottom	+=	dy;
			if ( (newFrame->IsValid()) && (newFrame->Width()>20) && ((newFrame->Height()>20)) )
			{
				node->ReplaceRect("Frame",*newFrame);
				changed->AddItem(node);
			}
		}
	}
	doc->SetModified();
	settings->RemoveName("Resize::Undo");
	settings->AddMessage("Resize::Undo",undoMessage);
	settings= PCommand::Do(doc,settings);
	return settings;
}



void Resize::AttachedToManager(void)
{
}

void Resize::DetachedFromManager(void)
{
}

void Resize::DoResize(PDocument *doc,BRect *rect,BMessage *settings)
{
	type_code		type;
	settings->FindInt32("type",(int32 *)&type);
}
