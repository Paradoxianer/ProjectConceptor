#include "KeyCaptureWindow.h"

#include <interface/InterfaceDefs.h>
#include <interface/Screen.h>
#include <interface/StringView.h>
#include <interface/View.h>
#include <kernel/OS.h>


class KeyCatcherView : public BView
{
public:
	KeyCatcherView(BRect frame,KeyCaptureWindow *owner)
		:BView(frame,"catcher",B_FOLLOW_ALL,B_WILL_DRAW|B_NAVIGABLE),fOwner(owner)
	{
		SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	}

	virtual void AttachedToWindow(void)
	{
		MakeFocus(true);
	}

	virtual void KeyDown(const char *bytes,int32 numBytes)
	{
		if ((numBytes > 0) && (bytes[0] == B_ESCAPE)) {
			fOwner->Cancel();
			return;
		}
		BMessage	*original	= Window()->CurrentMessage();
		int32		key			= 0;
		int32		modifiers	= 0;
		if (original) {
			original->FindInt32("raw_char",&key);
			original->FindInt32("modifiers",&modifiers);
		}
		fOwner->Capture(key,modifiers);
	}

private:
	KeyCaptureWindow	*fOwner;
};


KeyCaptureWindow::KeyCaptureWindow(const char *prompt)
	:BWindow(BRect(0,0,320,80),"Shortcut",B_MODAL_WINDOW_LOOK,B_MODAL_APP_WINDOW_FEEL,
			 B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{
	BScreen	screen;
	MoveTo(screen.Frame().Width()/2 - 160,screen.Frame().Height()/2 - 40);

	KeyCatcherView	*view	= new KeyCatcherView(Bounds(),this);
	BStringView		*label	= new BStringView(BRect(10,10,310,70),"prompt",prompt);
	view->AddChild(label);
	AddChild(view);

	fKey		= 0;
	fModifiers	= 0;
	fDone		= false;
	fCancelled	= false;
}

bool KeyCaptureWindow::Go(int32 *key,int32 *modifiers)
{
	Show();
	// same crude poll used by InputRequest::Go() - Capture()/Cancel() run
	// on this window's own thread, this loop runs on the caller's
	while (!fDone)
		snooze(50000);
	bool	success	= !fCancelled;
	if (success) {
		*key		= fKey;
		*modifiers	= fModifiers;
	}
	if (Lock())
		Quit();
	return success;
}

void KeyCaptureWindow::Capture(int32 key,int32 modifiers)
{
	fKey		= key;
	fModifiers	= modifiers;
	fDone		= true;
}

void KeyCaptureWindow::Cancel(void)
{
	fCancelled	= true;
	fDone		= true;
}
