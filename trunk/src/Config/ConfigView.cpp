#include "ConfigView.h"
#include "ProjectConceptorDefs.h"

ConfigView::ConfigView(BRect rect,const char *newName,BMessage *forMessage,uint32 resizingMode, uint32 flags):BBox(rect,NULL,resizingMode,flags)
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
