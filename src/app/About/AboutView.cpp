/*
 * Copyright 2008 ProjectConceptor All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ralf Schülke, teammaui@web.de
 */

#include <stdio.h>

#include <Screen.h>
#include <Rect.h>
#include <StringView.h>
#include <Alert.h>

#include "AboutView.h"
#include "AboutGitInfo.h"

AboutView::AboutView(BRect frame):BView(frame, "", B_FOLLOW_ALL, B_WILL_DRAW)
{
	//Constucter
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	frameWidth	= frame.Width();
	frameHeight = frame.Height();


	//BBitmap
	fBitmap = new BBitmap(BRect(0,0,64-1,64-1),B_RGBA32);
	memcpy(fBitmap->Bits(), kLogogross_64Bits, fBitmap->BitsLength());
	//

	//TitleTextView
	fTitleView = new BTextView(BRect(80,10,frameWidth-5,55), "titleview",
	BRect((frameWidth+60)/2-(16*25/2),10,frameWidth,40), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
	fTitleView->MakeSelectable(false);
	fTitleView->MakeEditable(false);
	fTitleFont.SetSize(30.0);
	fTitleView->SetFontAndColor(&fTitleFont, B_FONT_ALL);
	fTitleView->Insert("ProjectConceptor");
	fTitleView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(fTitleView);

	//CeckString
	fRevNumb.SetTo(GIT_REV_STR);

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
	fSvnInfoView = new BTextView(BRect((frameWidth+60)/2-(fRevNumb.Length()*5/2),60,frameWidth-5,75), "svninfoview",
	BRect(0,0,frameWidth,15), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
	fSvnInfoView->MakeSelectable(false);
	fSvnInfoView->MakeEditable(false);
	fSvnInfoFont.SetSize(10.0);
	fSvnInfoView->SetFontAndColor(&fSvnInfoFont, B_FONT_ALL);
	fSvnInfoView->Insert(fRevNumb.String());
	fSvnInfoView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(fSvnInfoView);

	//String and Length
	fDeveloperTitleString.SetTo("developer");
	kDL = fDeveloperTitleString.Length();

	fContributorTitleString.SetTo("contributor");
	kCL = fContributorTitleString.Length();

	fTranslatorTitleString.SetTo("translator");
	kTL = fTranslatorTitleString.Length();

	fWebsiteTitleString.SetTo("website");
	kWL = fWebsiteTitleString.Length();

	//DeveloperTitleView
	fDeveloperTitleView = new BTextView(BRect(10+30,100,kDL*(12/1.5)+30,115), "developertitleview",
	BRect(0,0,frameWidth-15,15), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
	fDeveloperTitleView->MakeSelectable(false);
	fDeveloperTitleView->MakeEditable(false);
	fDeveloperTitleFont.SetSize(12.0);
	fDeveloperTitleView->SetFontAndColor(&fDeveloperTitleFont, B_FONT_ALL);
	fDeveloperTitleView->Insert(fDeveloperTitleString.String());
	fDeveloperTitleView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(fDeveloperTitleView);

	//DeveloperNameView
	fDeveloperNameString.SetTo("- paradoxon\n- beathlon\n- stargater");
	fDeveloperNameView = new BTextView(BRect(10+30,120,kDL*(12/1.5)+30,frameHeight-70), "developernameview",
	BRect(0,0,frameWidth-15,15), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
	fDeveloperNameView->MakeSelectable(false);
	fDeveloperNameView->MakeEditable(false);
	fDeveloperNameFont.SetSize(10.0);
	fDeveloperNameView->SetFontAndColor(&fDeveloperNameFont, B_FONT_ALL);
	fDeveloperNameView->Insert(fDeveloperNameString.String());
	fDeveloperNameView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(fDeveloperNameView);

	//ContributorTitleView
	fContributorTitleView = new BTextView(BRect(20+30+(kDL*(12/1.5)),100,kCL*(12/1.5)+(kDL*(12/1.5)-9+40),115), "contributortitleview",
	BRect(0,0,frameWidth-15,15), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
	fContributorTitleView->MakeSelectable(false);
	fContributorTitleView->MakeEditable(false);
	fContributorTitleFont.SetSize(12.0);
	fContributorTitleView->SetFontAndColor(&fContributorTitleFont, B_FONT_ALL);
	fContributorTitleView->Insert(fContributorTitleString.String());
	fContributorTitleView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(fContributorTitleView);

	//ContributorNameView
	fContributorNameString.SetTo("- tombhadAC\n- jan__64\n- MauriceK");
	fContributorNameView = new BTextView(BRect(20+30+(kDL*(12/1.5)),120,kCL*(12/1.5)+(kDL*(12/1.5)-9+40),frameHeight-70), "contributornameview",
	BRect(0,0,frameWidth-15,15), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
	fContributorNameView->MakeSelectable(false);
	fContributorNameView->MakeEditable(false);
	fContributorNameFont.SetSize(10.0);
	fContributorNameView->SetFontAndColor(&fContributorNameFont, B_FONT_ALL);
	fContributorNameView->Insert(fContributorNameString.String());
	fContributorNameView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(fContributorNameView);

	//TranslatorTitleView
	fTranslatorTitleView = new BTextView(BRect(20+30+(kDL*(12/1.5)+kCL*(12/1.5)),100,kTL*(12/1.5)+(kDL*(12/1.5)+kCL*(12/1.5)-9+40),115), "translatortitleview",
	BRect(0,0,frameWidth-15,15), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
	fTranslatorTitleView->MakeSelectable(false);
	fTranslatorTitleView->MakeEditable(false);
	fTranslatorTitleFont.SetSize(12.0);
	fTranslatorTitleView->SetFontAndColor(&fTranslatorTitleFont, B_FONT_ALL);
	fTranslatorTitleView->Insert(fTranslatorTitleString.String());
	fTranslatorTitleView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(fTranslatorTitleView);

	//TranslatorNameView
	fTranslatorNameString.SetTo("- thaflo");
	fTranslatorNameView = new BTextView(BRect(20+30+(kDL*(12/1.5)+kCL*(12/1.5)),120,kTL*(12/1.5)+(kDL*(12/1.5)+kCL*(12/1.5)-9+40),frameHeight-70), "translatornameview",
	BRect(0,0,frameWidth-15,15), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
	fTranslatorNameView->MakeSelectable(false);
	fTranslatorNameView->MakeEditable(false);
	fTranslatorNameFont.SetSize(10.0);
	fTranslatorNameView->SetFontAndColor(&fTranslatorNameFont, B_FONT_ALL);
	fTranslatorNameView->Insert(fTranslatorNameString.String());
	fTranslatorNameView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(fTranslatorNameView);

	//WebsiteTitleView
	fWebsiteTitleView = new BTextView(BRect(20+30+(kDL*(12/1.5)+kCL*(12/1.5)+kTL*(12/1.5)),100,kWL*(12/1.5)+(kDL*(12/1.5)+kCL*(12/1.5)+kTL*(12/1.5)+40),115), "websitetitleview",
	BRect(0,0,frameWidth-15,15), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
	fWebsiteTitleView->MakeSelectable(false);
	fWebsiteTitleView->MakeEditable(false);
	fWebsiteTitleFont.SetSize(12.0);
	fWebsiteTitleView->SetFontAndColor(&fWebsiteTitleFont, B_FONT_ALL);
	fWebsiteTitleView->Insert(fWebsiteTitleString.String());
	fWebsiteTitleView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(fWebsiteTitleView);

	//WebsiteNameView
	fWebsiteNameString.SetTo("- neilch");
	fWebsiteNameView = new BTextView(BRect(20+30+(kDL*(12/1.5)+kCL*(12/1.5)+kTL*(12/1.5)),120,kWL*(12/1.5)+(kDL*(12/1.5)+kCL*(12/1.5)+kTL*(12/1.5)+40),frameHeight-70), "websitenameview",
	BRect(0,0,frameWidth-15,15), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
	fWebsiteNameView->MakeSelectable(false);
	fWebsiteNameView->MakeEditable(false);
	fWebsiteNameFont.SetSize(12.0);
	fWebsiteNameView->SetFontAndColor(&fWebsiteNameFont, B_FONT_ALL);
	fWebsiteNameView->Insert(fWebsiteNameString.String());
	fWebsiteNameView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(fWebsiteNameView);


	//ContactTitleView
	fContactTitleString.SetTo("contact");
	fContactTitleView = new BTextView(BRect(10,frameHeight-23,10+fContactTitleString.Length()*(12/1.5),frameHeight-10), "contacttetitleview",
	BRect(0,0,frameWidth-15,15), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
	fContactTitleView->MakeSelectable(false);
	fContactTitleView->MakeEditable(false);
	fContactTitleFont.SetSize(10.0);
	fContactTitleView->SetFontAndColor(&fContactTitleFont, B_FONT_ALL);
	fContactTitleView->Insert(fContactTitleString.String());
	fContactTitleView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(fContactTitleView);


	//ContactURLView
	//This is a link for a web URL.
	fWebView = new URLView(BRect(fContactTitleString.Length()+90, frameHeight-23, fContactTitleString.Length()*(12/1.5)+180, frameHeight-10), "Web", "www.ProjectConceptor.de", "http://www.projectconceptor.de");
	fWebView->SetFontSize(10.0);
	fWebView->AddAttribute("META:keyw", "www.ProjectConceptor.de");
	AddChild(fWebView);

	//This is a link for an e-mail address.
	fMailView = new URLView(BRect(fContactTitleString.Length()+90+150, frameHeight-23, frameWidth-5, frameHeight-10), "E-mail", "send an Email", "mail@projectconceptor.de");
	fMailView->SetHoverEnabled(false);
	fMailView->SetFontSize(10.0);
	fMailView->AddAttribute("META:name", "ProjectConceptor");
	//fMailView->AddAttribute("META:nickname", "xxx");
	//fMailView->AddAttribute("META:company", "xxx");
	fMailView->AddAttribute("META:url", "http://www.projectconceptor.de");
	fMailView->AddAttribute("META:state", "DE");
	fMailView->AddAttribute("META:country", "Germany");
	AddChild(fMailView );

}

AboutView::~AboutView()
{
	//
}
 
void
AboutView::Draw(BRect)
{
	//Draw stuffs
	SetDrawingMode(B_OP_ALPHA);
	DrawBitmap(fBitmap,BPoint(10,10));

}
