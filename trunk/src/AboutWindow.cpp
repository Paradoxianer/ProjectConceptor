#include <interface/Screen.h>
#include <interface/Rect.h>
#include <interface/StringView.h>

#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>

#include "AboutWindow.h"
#include "AboutView.h"
#include "ProjectConceptorDefs.h"

AboutWindow::AboutWindow():BWindow(BRect(100,100,500,350),_T("About"),B_FLOATING_WINDOW,0)
{
	TRACE();
	BScreen *tmpScreen	= new BScreen();
	BRect windowRect	= tmpScreen->Frame();
	MoveTo((windowRect.Width()-Bounds().Width())/2,(windowRect.Height()-Bounds().Height())/2);
	
	#ifdef REV_NUMBER
	AddChild(new BStringView(Bounds(),"VersionsString",REV_NUMBER));
	#else
	AddChild(new BStringView(Bounds(),"VersionsString","TEST"));
	#endif
	
	AboutView* av = new AboutView(windowRect);
	AddChild(av);
	//std::cout << "hallo" << std::endl;
	
}
AboutWindow::~AboutWindow()
{
	TRACE();

}
void AboutWindow::ChangeLanguage(void)
{
	TRACE();
}
