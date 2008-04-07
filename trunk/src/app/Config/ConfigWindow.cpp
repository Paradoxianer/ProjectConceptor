#include "ConfigWindow.h"


ConfigWindow::ConfigWindow(BMessage *_configMessage):BWindow(BRect(50,50,600,400),_T("Settings"),B_TITLED_WINDOW,0)
{
	TRACE();
	configMessage=_configMessage;
	configView	= new ConfigView(Bounds(),configMessage);
	AddChild(configView);

}


void ConfigWindow::ChangeLanguage()
{
	TRACE();

}

void ConfigWindow::SetConfigMessage(BMessage *_configMessage)
{
	configMessage=_configMessage;
	configView->SetConfigMessage(configMessage);
}

void ConfigWindow::Quit()
{
	TRACE();
	BWindow::Quit();
}
