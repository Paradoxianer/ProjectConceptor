/*
	FontPanel.cpp: A general-purpose font selection class
	Written by DarkWyrm <darkwyrm@earthlink.net>, Copyright 2007
	Released under the MIT license.
*/

#include "FontPanel.h"
#include "FontView.h"

#include <Application.h>
#include <Invoker.h>
#include <String.h>
#include <stdio.h>
#include <ScrollView.h>
#include <ScrollBar.h>


// TODO: Add Escape key as a shortcut for cancelling

enum
{
	M_OK = 'm_ok',
	M_CANCEL,
	M_SIZE_CHANGED,
	M_FAMILY_SELECTED,
	M_STYLE_SELECTED,
	M_HIDE_WINDOW
};

class FontPreview : public BView
{
public:
					FontPreview(const BRect &frame);
			
	void			SetPreviewText(const char *text);
	const char *	PreviewText(void) const;
			
	void			Draw(BRect r);
	
private:
	BString	fPreviewText;
};


FontPreview::FontPreview(const BRect &frame)
 :	BView(frame,"fontpreview",B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW),
 	fPreviewText("AaBbCcDdEeFfGg1234567890")
{
}


void
FontPreview::SetPreviewText(const char *text)
{
	fPreviewText = text ? text : "AaBbCcDdEeFfGg";
}


const char *
FontPreview::PreviewText(void) const
{
	return fPreviewText.String();
} 


void
FontPreview::Draw(BRect r)
{
	BFont font;
	GetFont(&font);
	
	int32 width = (int32)font.StringWidth(fPreviewText.String());
	
	BPoint drawpt;
	if (width < Bounds().IntegerWidth())
		drawpt.x = (Bounds().IntegerWidth() - width) / 2;
	else
		drawpt.x = 10;
	
	font_height fheight;
	font.GetHeight(&fheight);
	int32 size = (int32)(fheight.ascent + fheight.descent + fheight.leading);
	
	if (size < Bounds().IntegerHeight() - 10)
		drawpt.y = (Bounds().IntegerHeight() + fheight.ascent) / 2;
	else
		drawpt.y = Bounds().bottom - 10;
	DrawString(fPreviewText.String(), drawpt);
}






class FontWindow : public BWindow
{
public:
			FontWindow(const BRect &frame, float fontsize);
			~FontWindow(void);
	bool	QuitRequested(void);
	void	MessageReceived(BMessage *msg);
	
	void	ReallyQuit(void) { fReallyQuit = true; }
	
			FontView *fView;

private:
	bool	fReallyQuit;
};


FontWindow::FontWindow(const BRect &frame, float fontsize)
 : BWindow(frame,"Choose a Font", B_TITLED_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL,0)
{
	RemoveShortcut('w',B_COMMAND_KEY);
	AddShortcut('w',B_COMMAND_KEY,new BMessage(M_HIDE_WINDOW));
	SetSizeLimits(400,2400,300,2400);
	fReallyQuit = false;
	
	fView = new FontView();
	AddChild(fView);
}


FontWindow::~FontWindow(void)
{
}


bool
FontWindow::QuitRequested(void)
{
	if (!fReallyQuit)
		PostMessage(M_HIDE_WINDOW);
	
	return fReallyQuit;
}

 
void
FontWindow::MessageReceived(BMessage *msg)
{
	if (msg->what == M_HIDE_WINDOW)
		Hide();
	else
		BWindow::MessageReceived(msg);
}


FontPanel::FontPanel(BMessenger *target,BMessage *msg, float size, bool modal,
					bool hide_when_done)
{
	fWindow = new FontWindow(BRect(200,200,600,500),size);
	
	if (target)
		fWindow->fView->SetTarget(*target);
	
	if (msg)
		fWindow->fView->SetMessage(msg);
	
	if (modal)
		fWindow->SetFeel(B_MODAL_APP_WINDOW_FEEL);
	
	
	fWindow->fView->SetFontSize(size);
	//fWindow->fView->SetHideWhenDone(hide_when_done);
}


FontPanel::~FontPanel(void)
{
	fWindow->ReallyQuit();
	fWindow->PostMessage(B_QUIT_REQUESTED);
}


void
FontPanel::SelectFont(const BFont &font)
{
	//fWindow->fView->SelectFont(&font);
}


void
FontPanel::SelectFont(font_family family, font_style style, float size)
{
	fWindow->fView->SelectFont(family,style,size);
}


void
FontPanel::Show()
{
	fWindow->Show();
}


void
FontPanel::Hide()
{
	fWindow->Hide();
}


bool
FontPanel::IsShowing(void) const
{
	return !fWindow->IsHidden();
}


BWindow *
FontPanel::Window(void) const
{
	return fWindow;
}


void
FontPanel::SetTarget(BMessenger msgr)
{
	fWindow->fView->SetTarget(msgr);
}


void
FontPanel::SetMessage(BMessage *msg)
{
	fWindow->fView->SetMessage(msg);
}


void
FontPanel::SetHideWhenDone(bool value)
{
	//fWindow->fView->SetHideWhenDone(value);
}


bool
FontPanel::HideWhenDone(void) const
{
//	return fWindow->fView->HideWhenDone();
}


void
FontPanel::SetFontSize(uint16 size)
{
	fWindow->fView->SetFontSize(size);
}
