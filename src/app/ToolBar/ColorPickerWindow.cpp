#include "ColorPickerWindow.h"
#include "AlphaSlider.h"
#include "ColorSwatchView.h"

#include <app/Looper.h>
#include <app/MessageFilter.h>
#include <interface/ColorControl.h>

// Local to this file only - ColorToolItem.h happens to declare its own
// (unrelated) COLOR_CHANGED constant, so these stay unexported to avoid
// any confusion between the two.
enum {
	PW_COLOR_CONTROL_CHANGED	= 'pwCC',
	PW_ALPHA_CHANGED			= 'pwAC',
};


// Closes the window on Escape - installed as a common filter so it sees
// the key before any child control's own KeyDown handling.
class ColorPickerEscapeFilter : public BMessageFilter {
public:
	ColorPickerEscapeFilter(BWindow *window)
		: BMessageFilter(B_KEY_DOWN), fWindow(window) {}

	virtual filter_result Filter(BMessage *message, BHandler **target) {
		int8	byte	= 0;
		if ((message->FindInt8("byte",&byte) == B_OK) && (byte == B_ESCAPE)) {
			fWindow->PostMessage(B_QUIT_REQUESTED);
			return B_SKIP_MESSAGE;
		}
		return B_DISPATCH_MESSAGE;
	}

private:
	BWindow	*fWindow;
};


ColorPickerWindow::ColorPickerWindow(BRect frame, rgb_color color,
		BMessage *message, BHandler *target)
	: BWindow(frame, "Color", B_BORDERED_WINDOW_LOOK,
		B_FLOATING_APP_WINDOW_FEEL,
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS),
	fMessage(message),
	fTarget(target)
{
	fColorControl = new BColorControl(BPoint(1,1), B_CELLS_32x8, 1.0,
		"ColorPickerWindow::colorControl", new BMessage(PW_COLOR_CONTROL_CHANGED));
	fColorControl->SetValue(color);
	fColorControl->SetTarget(this);
	AddChild(fColorControl);

	float	width, height;
	fColorControl->GetPreferredSize(&width,&height);

	fAlphaSlider = new AlphaSlider(B_HORIZONTAL, new BMessage(PW_ALPHA_CHANGED));
	fAlphaSlider->MoveTo(1, height + 4);
	fAlphaSlider->ResizeTo(width - 2, 20);
	fAlphaSlider->SetTarget(this);
	fAlphaSlider->SetColor(color);
	fAlphaSlider->SetValue(color.alpha);
	AddChild(fAlphaSlider);

	ResizeTo(width, height + 4 + 20 + 4);

	AddCommonFilter(new ColorPickerEscapeFilter(this));
}


ColorPickerWindow::~ColorPickerWindow()
{
}


void
ColorPickerWindow::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case PW_COLOR_CONTROL_CHANGED: {
			rgb_color	newColor	= fColorControl->ValueAsColor();
			newColor.alpha			= fAlphaSlider->Value();
			fAlphaSlider->SetColor(newColor);
			_ReportColor();
			break;
		}
		case PW_ALPHA_CHANGED: {
			_ReportColor();
			break;
		}
		default:
			BWindow::MessageReceived(message);
			break;
	}
}


void
ColorPickerWindow::WindowActivated(bool active)
{
	BWindow::WindowActivated(active);
	if (!active)
		PostMessage(B_QUIT_REQUESTED);
}


bool
ColorPickerWindow::QuitRequested(void)
{
	if ((fMessage != NULL) && (fTarget != NULL)) {
		BLooper *looper = fTarget->Looper();
		if (looper != NULL) {
			BMessage	closed(PW_CLOSED);
			looper->PostMessage(&closed, fTarget);
		}
	}
	return BWindow::QuitRequested();
}


void
ColorPickerWindow::SetColor(rgb_color color)
{
	fColorControl->SetValue(color);
	fAlphaSlider->SetColor(color);
	fAlphaSlider->SetValue(color.alpha);
}


rgb_color
ColorPickerWindow::Color(void) const
{
	rgb_color	color	= fColorControl->ValueAsColor();
	color.alpha			= fAlphaSlider->Value();
	return color;
}


void
ColorPickerWindow::_ReportColor()
{
	if ((fMessage == NULL) || (fTarget == NULL))
		return;

	BLooper *looper = fTarget->Looper();
	if (looper == NULL)
		return;

	BMessage	report(*fMessage);
	AddColorToMessage(&report, Color());
	looper->PostMessage(&report, fTarget);
}
