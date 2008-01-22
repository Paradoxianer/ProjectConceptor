#include "ConfigWindow.h"


ConfigWindow::ConfigWindow(BMessage *_configMessage):BWindow(BRect(50,50,300,300),_T("Settings"),B_TITLED_WINDOW,0)
{
	TRACE();
	configMessage=_configMessage;

}
void ConfigWindow::ChangeLanguage()
{
	TRACE();

}
void ConfigWindow::Quit()
{
	TRACE();
	BWindow::Quit();
}
