
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
	
	//BBitmap
	fBitmapType = new BBitmap(BRect(0,0,64-1,64-1),B_RGBA32);
	memcpy(fBitmapType->Bits(), kLogogross64Bits, fBitmapType->BitsLength());
	//
	
	//TitleTextView
	TitleView = new BTextView(BRect(75,10,kW,60), "titleview",
	BRect((kW+60)/2-(16*25/2),15,kW,45), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
	BFont TitleFont;
	TitleFont.SetSize(30.0);
	TitleView->SetFontAndColor(&TitleFont, B_FONT_ALL);
	TitleView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	TitleView->Insert("ProjectConceptor\n test");
	AddChild(TitleView);
	//SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	
	//test test
	
	//Debug
	//char* meinCharSTring;
	//sprintf(meinCharSTring,"%d",h);
	//(new BAlert("", meinCharSTring, "Exit"))->Go();
	
	//CutString
	fRevNumbRaw.SetTo(SVN_REV_STR);
	kStrLength = fRevNumbRaw.Length()-7;
	fRevNumbRaw.MoveInto(fRevNumb, 5, kStrLength);
	
	//String Insert Titel and __DATE__
	fRevNumb.Insert("revision: ",1);
	fRevNumb.Insert("    ",fRevNumb.Length());
	fRevNumb.Insert("build date: ",fRevNumb.Length());
	fRevNumb.Insert(__DATE__,fRevNumb.Length());
	
	//revision and build date stringview
	//AddChild(new BStringView(BRect(0,kH-20,kW,kH),"VersionsString",fRevNumb.String()));
	AddChild(new BStringView(BRect((kW+60)/2-(fRevNumb.Length()*5/2),65,kW,75),"VersionsString",fRevNumb.String()));
}

AboutView::~AboutView()
{
	//
}

void
AboutView::Draw(BRect updated)
{
	//Draw stuffs	
	SetDrawingMode(B_OP_ALPHA);
	DrawBitmap(fBitmapType,BPoint(10,10));	
	
}
