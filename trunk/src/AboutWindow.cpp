#include <interface/Screen.h>
#include <interface/Rect.h>
#include <interface/StringView.h>
#include "AboutWindow.h"
#include "ProjectConceptorDefs.h"

AboutWindow::AboutWindow():BWindow(BRect(100,100,400,500),_T("About"),B_FLOATING_WINDOW,0)
{
	TRACE();
	BScreen *tmpScreen	= new BScreen();
	BRect windowRect	= tmpScreen->Frame();
	MoveTo((windowRect.Width()-Bounds().Width())/2,(windowRect.Height()-Bounds().Height())/2);
	AddChild(new BStringView(Bounds(),"VersionsString",REV_NUMBER));
}
AboutWindow::~AboutWindow()
{
	TRACE();

}
void AboutWindow::ChangeLanguage(void)
{
	TRACE();
}
