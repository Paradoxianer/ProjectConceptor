#include <math.h>

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
	PW_PALETTE_CLICKED			= 'pwPC',
};

static const float kPaletteSwatchHeight	= 22.0;
static const float kMargin					= 1.0;


// Standard HSV->RGB conversion (hueDegrees in [0,360), saturation/value in
// [0,1]) - written fresh rather than reusing any third-party
// implementation (this codebase deliberately avoids the non-standard-
// licensed rgb_hsv.h that ships alongside Icon-O-Matic's gradient
// picker), since this is what generates the algorithmic palette.
static rgb_color
HueToColor(float hueDegrees, float saturation, float value)
{
	float	c			= value * saturation;
	float	hPrime		= hueDegrees / 60.0;
	float	x			= c * (1.0 - fabs(fmod(hPrime, 2.0) - 1.0));
	float	r1 = 0, g1 = 0, b1 = 0;

	if (hPrime < 1)			{ r1 = c; g1 = x; b1 = 0; }
	else if (hPrime < 2)	{ r1 = x; g1 = c; b1 = 0; }
	else if (hPrime < 3)	{ r1 = 0; g1 = c; b1 = x; }
	else if (hPrime < 4)	{ r1 = 0; g1 = x; b1 = c; }
	else if (hPrime < 5)	{ r1 = x; g1 = 0; b1 = c; }
	else					{ r1 = c; g1 = 0; b1 = x; }

	float	m = value - c;
	rgb_color	color;
	color.red	= (uint8)((r1 + m) * 255.0 + 0.5);
	color.green	= (uint8)((g1 + m) * 255.0 + 0.5);
	color.blue	= (uint8)((b1 + m) * 255.0 + 0.5);
	color.alpha	= 255;
	return color;
}


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

	float	width, height;
	fColorControl->GetPreferredSize(&width,&height);

	// palette row above the color control - algorithmically generated
	// (hue sweep), not hand-curated, so this widget doesn't need
	// per-project color curation to be reusable
	float	swatchWidth	= (width - 2*kMargin) / PW_PALETTE_SIZE;
	for (int32 i = 0; i < PW_PALETTE_SIZE; i++) {
		rgb_color	swatchColor	= HueToColor(
			i * (360.0 / PW_PALETTE_SIZE), 0.85, 0.95);
		fPaletteSwatch[i] = new ColorSwatchView("ColorPickerWindow::palette",
			new BMessage(PW_PALETTE_CLICKED), this, swatchColor,
			swatchWidth, kPaletteSwatchHeight);
		fPaletteSwatch[i]->MoveTo(kMargin + i*swatchWidth, kMargin);
		AddChild(fPaletteSwatch[i]);
	}

	float	colorControlTop	= kMargin + kPaletteSwatchHeight + kMargin;
	fColorControl->MoveTo(1, colorControlTop);
	fColorControl->SetTarget(this);
	AddChild(fColorControl);

	fAlphaSlider = new AlphaSlider(B_HORIZONTAL, new BMessage(PW_ALPHA_CHANGED));
	fAlphaSlider->MoveTo(1, colorControlTop + height + 4);
	fAlphaSlider->ResizeTo(width - 2, 20);
	fAlphaSlider->SetTarget(this);
	fAlphaSlider->SetColor(color);
	fAlphaSlider->SetValue(color.alpha);
	AddChild(fAlphaSlider);

	ResizeTo(width, colorControlTop + height + 4 + 20 + 4);

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
		case PW_PALETTE_CLICKED: {
			rgb_color	newColor;
			if (ColorFromMessage(message,newColor))
				_ApplyColor(newColor);
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
ColorPickerWindow::_ApplyColor(rgb_color color)
{
	fColorControl->SetValue(color);
	fAlphaSlider->SetColor(color);
	fAlphaSlider->SetValue(color.alpha);
	_ReportColor();
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
