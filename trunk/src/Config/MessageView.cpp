#include "MessageView.h"
#include "ProjectConceptorDefs.h"

MessageView::MessageView(BRect rect,const char *newName,BMessage *forMessage,uint32 resizingMode, uint32 flags):BBox(rect,NULL,resizingMode,flags)
{
	TRACE();
	configMessage=forMessage;
	name=newName;
	Init();
}

void MessageView::Init()
{
	seperator = Bounds().Width()/3;
}


void MessageView::ChangeLanguage()
{
	TRACE();
}

void MessageView::SetConfigMessage(BMessage *configureMessage)
{
	TRACE();
	configMessage=configureMessage;
	ValueChanged();
}
void MessageView::ValueChanged(void)
{
	TRACE();
	
}
