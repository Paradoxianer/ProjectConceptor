#include <stdio.h>

#include <interface/Screen.h>
#include <interface/Rect.h>
#include <interface/StringView.h>
#include <Alert.h>

#include "AboutView.h"
#include "SvnInfo.h"

AboutView::AboutView(BRect frame):BView(frame, "", B_FOLLOW_ALL, B_WILL_DRAW)
{
	//Constucter
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	
	kW = frame.Width();
	kH = frame.Height();
	
	//Debug
	//char* meinCharSTring;
	//sprintf(meinCharSTring,"%d",h);
	//(new BAlert("", meinCharSTring, "Exit"))->Go();
	
	//CutString
	//BString fRevNumbRaw, fRevNumb;
	fRevNumbRaw.SetTo(SVN_REV_STR);
	kStrLength = fRevNumbRaw.Length()-7;
	fRevNumbRaw.MoveInto(fRevNumb, 5, kStrLength);
	
	AddChild(new BStringView(BRect(0,kH-20,kW,kH),"VersionsString",fRevNumb.String()));
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
