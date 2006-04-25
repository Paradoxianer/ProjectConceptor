#include "Move.h"


Move::Move():PCommand()
{
}

void Move::Undo(PDocument *doc,BMessage *undo)
{
	BMessage		*undoMessage		= new BMessage();
	BList			*changed			= doc->GetChangedNodes();
	BRect			*oldFrame			= new BRect(0,0,100,100);
	BMessage		*node				= NULL;
	BMessage		*settings			= new BMessage();
	int32			i					= 0;
	PCommand::Undo(doc,undoMessage);
	undo->FindMessage("Move::Undo" ,undoMessage);
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

BMessage* Move::Do(PDocument *doc, BMessage *settings)
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
			*newFrame=*oldFrame;
			newFrame->OffsetBy(dx,dy);
			node->ReplaceRect("Frame",*newFrame);
			undoMessage->AddPointer("node",node);
			changed->AddItem(node);
			BRect	docRect			= doc->Bounds();
			if (newFrame->bottom >= docRect.Height())
				docRect.bottom= newFrame->bottom+20;
			if (newFrame->right >= docRect.Width())
				docRect.right = newFrame->right+20;
			if (docRect != doc->Bounds())
				doc->Resize(docRect.right,docRect.bottom);
			
		}
	}
	doc->SetModified();
	settings->RemoveName("Move::Undo");
	settings->AddMessage("Move::Undo", undoMessage);
	settings = PCommand::Do(doc,settings);
	return settings;
}



void Move::AttachedToManager(void)
{
}

void Move::DetachedFromManager(void)
{
}

