#include <interface/ScrollView.h>

#include "ConfigView.h"
#include "ProjectConceptorDefs.h"
#include "MessageItem.h"

ConfigView::ConfigView(BRect rect,BMessage *forMessage,uint32 resizingMode, uint32 flags):BViewSplitter(rect,B_VERTICAL,resizingMode,flags)
{
	TRACE();
	configMessage	= forMessage;
	Init();
}

void ConfigView::Init()
{
	configList		= NULL;
	showMessage		= NULL;
	seperator = Bounds().Width()/3;
	configList = new BOutlineListView(BRect(1,1,(Bounds().Width()/2)-B_V_SCROLL_BAR_WIDTH-5,Bounds().Height()),"overview",B_SINGLE_SELECTION_LIST,B_FOLLOW_ALL_SIDES);
	BMessage	*selectionMessage	= new BMessage(MESSAGE_SELECTED);
	configList->SetSelectionMessage(selectionMessage);
	AddChild(new BScrollView("scrollyRight",configList,B_FOLLOW_ALL_SIDES,0 ,false,true));
	showMessage	= new MessageView(BRect(1,1,(Bounds().Width()/2.1)-B_V_SCROLL_BAR_WIDTH,Bounds().Height()),configMessage);
	AddChild(new BScrollView("scrollyRight",showMessage,B_FOLLOW_ALL_SIDES,0 ,false,true));
}

void ConfigView::ChangeLanguage()
{
	TRACE();
}

void ConfigView::SetConfigMessage(BMessage *configureMessage)
{
	TRACE();
	configMessage=configureMessage;
	ValueChanged();
}

void ConfigView::ValueChanged(void)
{
	TRACE();
	configList->MakeEmpty();
	BListItem *general = new BStringItem("General");
	configList->AddItem(general);
	BuildConfigList(configMessage,general);
}


void ConfigView::BuildConfigList(BMessage *confMessage, BListItem *parentItem)
{
	char		*name; 
	uint32		type; 
	int32		count;
	BListItem	*item;
	#ifdef B_ZETA_VERSION_1_0_0
	for (int32 i = 0; confMessage->GetInfo(B_ANY_TYPE, i,(const char **) &name, &type, &count) == B_OK; i++)
	#else
	for (int32 i = 0; confMessage->GetInfo(B_ANY_TYPE, i,(char **) &name, &type, &count) == B_OK; i++)
	#endif
	{
		if (type == B_MESSAGE_TYPE)
		{
			BMessage	*tmpMessage		= new BMessage();
			if (confMessage->FindMessage(name,count-1,tmpMessage)==B_OK)
			{
				item = new MessageItem(name,tmpMessage);
				tmpMessage		= new BMessage();
				configList->AddUnder(item,parentItem);
				BuildConfigList(tmpMessage, item);
			}
		}
	}
	configList->SetTarget(this);
}
void ConfigView::MessageReceived(BMessage *msg) 
{
	if (msg->what == MESSAGE_SELECTED)
	{
		MessageItem	*selectedItem= dynamic_cast<MessageItem*> (configList->ItemAt(configList->CurrentSelection()));
		if (selectedItem != NULL)
			showMessage->SetConfigMessage(selectedItem->Message());
	}
}


