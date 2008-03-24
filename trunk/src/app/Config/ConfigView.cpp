#include "ConfigView.h"
#include "ProjectConceptorDefs.h"


ConfigView::ConfigView(BRect rect,BMessage *forMessage,uint32 resizingMode, uint32 flags):BBox(rect,NULL,resizingMode,flags)
{
	TRACE();
	configMessage=forMessage;
	configList	= NULL;
	showMessage	= NULL;
	Init();
}

void ConfigView::Init()
{
	seperator = Bounds().Width()/3;
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
	if (configList == NULL)
	{
		configList = new BOutlineListView(BRect(0,0,Bounds().Width()/3,Bounds().Height()),"overview");
		AddChild(configList);
	}
	else
	{
		configList->MakeEmpty();
	}
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
				configList->AddUnder(item = new BStringItem(name),parentItem);
				BuildConfigList(tmpMessage, item);
			}
		}
	}

}


