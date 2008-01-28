#include <stdio.h>
#include <interface/Screen.h>
#include <interface/Rect.h>
#include <interface/StringView.h>
#include <Alert.h>

#include "AboutView.h"
#include "SvnInfo.h"
#include "AboutURLView.h"

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
	
	//String and Length
	BString fDeveloperTitleString;
	fDeveloperTitleString.SetTo("developer");
	int kDL = fDeveloperTitleString.Length();
	
	BString fContributorTitleString;
	fContributorTitleString.SetTo("contributor");
	int kCL = fContributorTitleString.Length();
	
	BString fTranslatorTitleString;
	fTranslatorTitleString.SetTo("translator");
	int kTL = fTranslatorTitleString.Length();
	
	BString fWebsiteTitleString;
	fWebsiteTitleString.SetTo("website");
	int kWL = fWebsiteTitleString.Length();
	
	//DeveloperTitleView
	BTextView* DeveloperTitleView = new BTextView(BRect(10+30,100,kDL*(12/1.5)+30,115), "developertitleview",
	BRect(0,0,kW-15,15), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
	BFont DeveloperTitleFont;
	DeveloperTitleFont.SetSize(12.0);
	DeveloperTitleView->SetFontAndColor(&DeveloperTitleFont, B_FONT_ALL);
	DeveloperTitleView->Insert(fDeveloperTitleString.String());
	DeveloperTitleView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(DeveloperTitleView);
	
	//DeveloperNameView
	BString fDeveloperNameString;
	fDeveloperNameString.SetTo("- paradoxon\n- beathlon\n- stargater");
	BTextView* DeveloperNameView = new BTextView(BRect(10+30,120,kDL*(12/1.5)+30,kH-70), "developernameview",
	BRect(0,0,kW-15,15), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
	BFont DeveloperNameFont;
	DeveloperNameFont.SetSize(10.0);
	DeveloperNameView->SetFontAndColor(&DeveloperNameFont, B_FONT_ALL);
	DeveloperNameView->Insert(fDeveloperNameString.String());
	DeveloperNameView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(DeveloperNameView);
	
	//ContributorTitleView
	BTextView* ContributorTitleView = new BTextView(BRect(20+30+(kDL*(12/1.5)),100,kCL*(12/1.5)+(kDL*(12/1.5)-9+40),115), "contributortitleview",
	BRect(0,0,kW-15,15), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
	BFont ContributorTitleFont;
	ContributorTitleFont.SetSize(12.0);
	ContributorTitleView->SetFontAndColor(&ContributorTitleFont, B_FONT_ALL);
	ContributorTitleView->Insert(fContributorTitleString.String());
	ContributorTitleView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(ContributorTitleView);
	
	//ContributorNameView
	BString fContributorNameString;
	fContributorNameString.SetTo("- tombhadAC\n- jan__64\n- MauriceK");
	BTextView* ContributorNameView = new BTextView(BRect(20+30+(kDL*(12/1.5)),120,kCL*(12/1.5)+(kDL*(12/1.5)-9+40),kH-70), "contributornameview",
	BRect(0,0,kW-15,15), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
	BFont ContributorNameFont;
	ContributorNameFont.SetSize(10.0);
	ContributorNameView->SetFontAndColor(&ContributorNameFont, B_FONT_ALL);
	ContributorNameView->Insert(fContributorNameString.String());
	ContributorNameView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(ContributorNameView);
	
	//TranslatorTitleView
	BTextView* TranslatorTitleView = new BTextView(BRect(20+30+(kDL*(12/1.5)+kCL*(12/1.5)),100,kTL*(12/1.5)+(kDL*(12/1.5)+kCL*(12/1.5)-9+40),115), "translatortitleview",
	BRect(0,0,kW-15,15), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
	BFont TranslatorTitleFont;
	TranslatorTitleFont.SetSize(12.0);
	TranslatorTitleView->SetFontAndColor(&TranslatorTitleFont, B_FONT_ALL);
	TranslatorTitleView->Insert(fTranslatorTitleString.String());
	TranslatorTitleView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(TranslatorTitleView);
	
	//TranslatorNameView
	BString fTranslatorNameString;
	fTranslatorNameString.SetTo("- thaflo");
	BTextView* TranslatorNameView = new BTextView(BRect(20+30+(kDL*(12/1.5)+kCL*(12/1.5)),120,kTL*(12/1.5)+(kDL*(12/1.5)+kCL*(12/1.5)-9+40),kH-70), "translatornameview",
	BRect(0,0,kW-15,15), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
	BFont TranslatorNameFont;
	TranslatorNameFont.SetSize(10.0);
	TranslatorNameView->SetFontAndColor(&TranslatorNameFont, B_FONT_ALL);
	TranslatorNameView->Insert(fTranslatorNameString.String());
	TranslatorNameView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(TranslatorNameView);
	
	//WebsiteTitleView
	BTextView* WebsiteTitleView = new BTextView(BRect(20+30+(kDL*(12/1.5)+kCL*(12/1.5)+kTL*(12/1.5)),100,kWL*(12/1.5)+(kDL*(12/1.5)+kCL*(12/1.5)+kTL*(12/1.5)+40),115), "websitetitleview",
	BRect(0,0,kW-15,15), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
	BFont WebsiteTitleFont;
	WebsiteTitleFont.SetSize(12.0);
	WebsiteTitleView->SetFontAndColor(&WebsiteTitleFont, B_FONT_ALL);
	WebsiteTitleView->Insert(fWebsiteTitleString.String());
	WebsiteTitleView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(WebsiteTitleView);
	
	//WebsiteNameView
	BString fWebsiteNameString;
	fWebsiteNameString.SetTo("- neilch");
	BTextView* WebsiteNameView = new BTextView(BRect(20+30+(kDL*(12/1.5)+kCL*(12/1.5)+kTL*(12/1.5)),120,kWL*(12/1.5)+(kDL*(12/1.5)+kCL*(12/1.5)+kTL*(12/1.5)+40),kH-70), "websitenameview",
	BRect(0,0,kW-15,15), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
	BFont WebsiteNameFont;
	WebsiteNameFont.SetSize(10.0);
	WebsiteNameView->SetFontAndColor(&WebsiteNameFont, B_FONT_ALL);
	WebsiteNameView->Insert(fWebsiteNameString.String());
	WebsiteNameView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(WebsiteNameView);
	
	
	//ContactTitleView
	BString fContactTitleString;
	fContactTitleString.SetTo("contact");
	BTextView* ContactTitleView = new BTextView(BRect(40,kH-60,40+fContactTitleString.Length()*(12/1.5),kH-45), "contacttetitleview",
	BRect(0,0,kW-15,15), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
	BFont ContactTitleFont;
	ContactTitleFont.SetSize(12.0);
	ContactTitleView->SetFontAndColor(&ContactTitleFont, B_FONT_ALL);
	ContactTitleView->Insert(fContactTitleString.String());
	ContactTitleView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(ContactTitleView);
	
	
	//ContactURLView
	//This is a link for a web URL.
	URLView* web = new URLView(BRect(fContactTitleString.Length()+90, kH-60+1, fContactTitleString.Length()*(12/1.5)+90+80, kH-45+1), "Web", "ProjectConceptor.de", "http://www.projectconceptor.de");
	web->SetFontSize(12.0);
	web->AddAttribute("META:keyw", "ProjectConceptor.de");
	web->SetUnderlineThickness(2);
	AddChild(web);
	
	//This is a link for an e-mail address.
	URLView *email = new URLView( BRect(fContactTitleString.Length()+90+150, kH-60+2, kW-5, kH-45+2), "E-mail", "send me a E-mail", "mail@projectconceptor.de");
	email->SetHoverEnabled(false);
	email->SetFontSize(12.0);
	email->AddAttribute("META:name", "ProjectConceptor");
	//email->AddAttribute("META:nickname", "xxx");
	//email->AddAttribute("META:company", "xxx");
	email->AddAttribute("META:url", "http://www.projectconceptor.de");
	email->AddAttribute("META:state", "DE");
	email->AddAttribute("META:country", "Germany");
	AddChild(email );
	
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
