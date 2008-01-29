#include <interface/Screen.h>
#include <interface/Rect.h>
#include <interface/StringView.h>

#include "AboutWindow.h"
#include "ProjectConceptorDefs.h"


AboutWindow::AboutWindow():BWindow(BRect(100,100,500,350),_T("About"),B_FLOATING_WINDOW,B_MODAL_APP_WINDOW_FEEL | B_NOT_RESIZABLE | B_NOT_ZOOMABLE,0)
{
	TRACE();
	BScreen *tmpScreen	= new BScreen();
	BRect windowRect	= tmpScreen->Frame();
	MoveTo((windowRect.Width()-Bounds().Width())/2,(windowRect.Height()-Bounds().Height())/2);
	
	fAboutView = new AboutView(BRect(0,0,400,250));
	AddChild(fAboutView);
	
}
AboutWindow::~AboutWindow()
{
	TRACE();

}
void AboutWindow::ChangeLanguage(void)
{
	TRACE();
}
