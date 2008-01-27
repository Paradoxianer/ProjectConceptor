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
	//char* fCharString;
	//sprintf(fCharString,"%d",kH);
	//(new BAlert("", fCharString, "Exit"))->Go();
	
	//BBitmap
	fBitmapType = new BBitmap(BRect(0,0,64-1,64-1),B_RGBA32);
	memcpy(fBitmapType->Bits(), kLogogross_64Bits, fBitmapType->BitsLength());
	//
	
	//TitleTextView
	TitleView = new BTextView(BRect(80,10,kW-5,55), "titleview",
	BRect((kW+60)/2-(16*25/2),10,kW,40), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
	TitleFont.SetSize(30.0);
	TitleView->SetFontAndColor(&TitleFont, B_FONT_ALL);
	TitleView->Insert("ProjectConceptor");
	TitleView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(TitleView);
	
	//CeckString
	fRevNumb.SetTo(SVN_REV_STR);
	
	if(fRevNumb.Length() == 0) 
	{
		fRevNumb.SetTo("---"); 
	}
	
	//String Insert Titel and __DATE__
	fRevNumb.Insert("revision: ",0);
	fRevNumb.Insert("    ",fRevNumb.Length());
	fRevNumb.Insert("build date: ",fRevNumb.Length());
	fRevNumb.Insert(__DATE__,fRevNumb.Length());
	
	//SvnInfoView
	SvnInfoView = new BTextView(BRect((kW+60)/2-(fRevNumb.Length()*5/2),60,kW-5,75), "svninfoview",
	BRect(0,0,kW,15), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
	SvnInfoFont.SetSize(10.0);
	SvnInfoView->SetFontAndColor(&SvnInfoFont, B_FONT_ALL);
	SvnInfoView->Insert(fRevNumb.String());
	SvnInfoView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(SvnInfoView);
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
