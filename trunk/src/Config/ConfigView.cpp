#include "ConfigView.h"
#include "ProjectConceptorDefs.h"

ConfigView::ConfigView(BRect rect,const char *newName,BMessage *forMessage,uint32 resizingMode = B_FOLLOW_ALL_SIDES, uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP):BBox(rect,NULL,resizingMode,flags)
{
	TRACE();
	configMessage=forMessage;
	name=newName;
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
	
}
