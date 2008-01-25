#include <interface/Screen.h>
#include <interface/Rect.h>
#include <interface/StringView.h>

#include "AboutWindow.h"
#include "AboutView.h"
#include "ProjectConceptorDefs.h"
#include "SvnInfo.h"

AboutWindow::AboutWindow():BWindow(BRect(100,100,500,350),_T("About"),B_FLOATING_WINDOW,B_NOT_RESIZABLE | B_NOT_ZOOMABLE,0)
{
	TRACE();
	BScreen *tmpScreen	= new BScreen();
	BRect windowRect	= tmpScreen->Frame();
	MoveTo((windowRect.Width()-Bounds().Width())/2,(windowRect.Height()-Bounds().Height())/2);
	
	AddChild(new BStringView(Bounds(),"VersionsString",SVN_REV_STR));
	
	AboutView* av = new AboutView(windowRect);
	AddChild(av);
	
}
AboutWindow::~AboutWindow()
{
	TRACE();

}
void AboutWindow::ChangeLanguage(void)
{
	TRACE();
}
