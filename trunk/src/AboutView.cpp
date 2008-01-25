
#include "AboutView.h"

AboutView::AboutView(BRect frame):BView(frame, "", B_FOLLOW_ALL, B_WILL_DRAW)
{
	//Constucter
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	
}

AboutView::~AboutView()
{
	//
}

void
AboutView::Draw(BRect updated)
{
	//Draw stuffs	
	
}
