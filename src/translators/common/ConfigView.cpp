#include "ConfigView.h"
#include <interface/Font.h>
#include <interface/StringView.h>

ConfigView::ConfigView(const char *title): BView(BRect(0,0,100,100),"Test",B_FOLLOW_ALL_SIDES,0)
{
	BStringView *writer	= new BStringView(BRect(15,10,50,20),"Header",title);
	// a copy: be_bold_font itself is shared by the whole host app
	BFont	myFont(be_bold_font);
	myFont.SetSize(be_bold_font->Size()+1);
	writer->SetFont(&myFont);
	AddChild(writer);
	writer				= new BStringView(BRect(15,writer->Frame().bottom+10,50,writer->Frame().bottom+40),"Header","Version 0.1a     July 16.2006, Paradoxon powerd by Jesus");
	AddChild(writer);
}
