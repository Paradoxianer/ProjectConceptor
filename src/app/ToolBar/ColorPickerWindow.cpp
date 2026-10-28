#include <stdio.h>
#include <stdlib.h>

#include "ColorPickerWindow.h"
#include "AlphaSlider.h"
#include "ColorSwatchView.h"

#include <app/Looper.h>
#include <app/MessageFilter.h>
#include <interface/Button.h>
#include <interface/ColorControl.h>
#include <interface/TextControl.h>
#include <interface/View.h>

#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ColorPickerWindow"

// Local to this file only - ColorToolItem.h happens to declare its own
// (unrelated) COLOR_CHANGED constant, so these stay unexported to avoid
// any confusion between the two.
enum {
	PW_COLOR_CONTROL_CHANGED	= 'pwCC',
	PW_ALPHA_CHANGED			= 'pwAC',
	PW_ALPHA_TEXT_ENTERED		= 'pwAT',
	PW_PALETTE_CLICKED			= 'pwPC',
	PW_TARGET_CLICKED			= 'pwTB',
};

static const float kSwatchHeight		= 22.0;
static const float kMargin				= 1.0;
static const float kGap					= 4.0;
// matches BColorControl's own (private) kTextFieldsHSpacing and
// kBevelSpacing - see ~/repos/haiku/src/kits/interface/ColorControl.cpp
static const float kTextFieldsHSpacing	= 6.0;
static const float kBevelSpacing		= 2.0;

// calm, evenly bright tones that work as a node's color band as well as
// for a border or text
static const rgb_color kPalette[PW_PALETTE_SIZE] = {
	{ 74, 127, 214, 255 },	// blue
	{ 47, 164, 169, 255 },	// teal
	{ 62, 157, 110, 255 },	// green
	{ 139, 191, 63, 255 },	// lime
	{ 232, 197, 71, 255 },	// yellow
	{ 227, 163, 59, 255 },	// orange
	{ 217, 83, 79, 255 },	// red
	{ 224, 108, 159, 255 },	// pink
	{ 139, 111, 209, 255 },	// purple
	{ 107, 114, 128, 255 }	// slate
};

// the same hues as soft tints: a card background that dark text still
// reads on
static const rgb_color kLightPalette[PW_PALETTE_SIZE] = {
	{ 221, 233, 250, 255 },	// blue
	{ 214, 240, 240, 255 },	// teal
	{ 219, 240, 228, 255 },	// green
	{ 234, 244, 214, 255 },	// lime
	{ 252, 244, 214, 255 },	// yellow
	{ 252, 234, 212, 255 },	// orange
	{ 250, 222, 220, 255 },	// red
	{ 250, 226, 237, 255 },	// pink
	{ 233, 227, 248, 255 },	// purple
	{ 230, 232, 236, 255 }	// slate
};


bool
IsPaletteColor(rgb_color color)
{
	for (int32 i = 0; i < PW_PALETTE_SIZE; i++) {
		if ((kPalette[i] == color) || (kLightPalette[i] == color))
			return true;
	}
	return false;
}


// Closes the window on Escape - installed as a common filter so it sees
// the key before any child control's own KeyDown handling. Marks the
// close as cancelled first, so QuitRequested() tells the target to
// discard whatever was last previewed instead of committing it.
class ColorPickerEscapeFilter : public BMessageFilter {
public:
	ColorPickerEscapeFilter(ColorPickerWindow *window)
		: BMessageFilter(B_KEY_DOWN), fWindow(window) {}

	virtual filter_result Filter(BMessage *message, BHandler **target) {
		int8	byte	= 0;
		if ((message->FindInt8("byte",&byte) == B_OK) && (byte == B_ESCAPE)) {
			fWindow->Cancel();
			fWindow->PostMessage(B_QUIT_REQUESTED);
			return B_SKIP_MESSAGE;
		}
		return B_DISPATCH_MESSAGE;
	}

private:
	ColorPickerWindow	*fWindow;
};


ColorPickerWindow::ColorPickerWindow(BRect frame,
		BMessage *message, BHandler *target,
		const ColorPickerTarget *targets, int32 targetCount,
		int32 currentTarget, const rgb_color *history, int32 historyCount)
	: BWindow(frame, "Color", B_BORDERED_WINDOW_LOOK,
		B_FLOATING_APP_WINDOW_FEEL,
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS),
	fTargetCount(targetCount < PW_MAX_TARGETS ? targetCount : PW_MAX_TARGETS),
	fCurrentTarget(0),
	fHistoryCount(0),
	fMessage(message),
	fTarget(target),
	fCancelled(false)
{
	for (int32 i = 0; i < fTargetCount; i++)
		fTargets[i]	= targets[i];
	if ((currentTarget >= 0) && (currentTarget < fTargetCount))
		fCurrentTarget	= currentTarget;
	rgb_color	color	= fTargets[fCurrentTarget].color;

	fColorControl = new BColorControl(BPoint(1,1), B_CELLS_32x8, 1.0,
		"ColorPickerWindow::colorControl", new BMessage(PW_COLOR_CONTROL_CHANGED));
	fColorControl->SetValue(color);
	float	width, height;
	fColorControl->GetPreferredSize(&width,&height);

	// BColorControl's own R/G/B fields (public FindView()), so the alpha
	// row lines up with them exactly
	BTextControl	*redText	= dynamic_cast<BTextControl *>(
		fColorControl->FindView("_red"));
	BTextControl	*greenText	= dynamic_cast<BTextControl *>(
		fColorControl->FindView("_green"));
	BTextControl	*blueText	= dynamic_cast<BTextControl *>(
		fColorControl->FindView("_blue"));
	float	textFieldLeft	= 1 + redText->Frame().left;
	float	textFieldWidth	= redText->Frame().Width();
	float	textFieldHeight	= redText->Frame().Height();
	float	rowHeight		= greenText->Frame().top - redText->Frame().top;
	// one ramp of BColorControl: its 8 cell rows shared by 4 ramps
	float	rampHeight		= 2 * fColorControl->CellSize();

	BView	*background	= new BView(BRect(0,0,width,100),
		"ColorPickerWindow::background", B_FOLLOW_NONE, 0);
	background->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(background);

	float	y	= kMargin;

	// one tab per target, if there is a choice
	for (int32 i = 0; i < PW_MAX_TARGETS; i++)
		fTargetButton[i]	= NULL;
	if (fTargetCount > 1) {
		float	buttonWidth	= (width - 2*kMargin) / fTargetCount;
		float	buttonHeight	= 0;
		for (int32 i = 0; i < fTargetCount; i++) {
			BMessage	*clicked	= new BMessage(PW_TARGET_CLICKED);
			clicked->AddInt32("index",i);
			fTargetButton[i]	= new BButton(BRect(0,0,buttonWidth-1,20),
				"ColorPickerWindow::target",fTargets[i].label.String(),clicked);
			fTargetButton[i]->SetBehavior(BButton::B_TOGGLE_BEHAVIOR);
			fTargetButton[i]->SetValue(i == fCurrentTarget ? B_CONTROL_ON : B_CONTROL_OFF);
			float	w, h;
			fTargetButton[i]->GetPreferredSize(&w,&h);
			fTargetButton[i]->ResizeTo(buttonWidth-1,h);
			fTargetButton[i]->MoveTo(kMargin + i*buttonWidth,y);
			fTargetButton[i]->SetTarget(this);
			background->AddChild(fTargetButton[i]);
			buttonHeight	= h;
		}
		y	+= buttonHeight + kGap;
	}

	// "Standard", then the palette
	float	swatchWidth	= (width - 2*kMargin) / (PW_PALETTE_SIZE + 1);
	fStandardSwatch	= new ColorSwatchView("ColorPickerWindow::standard",
		new BMessage(PW_PALETTE_CLICKED), this, fTargets[fCurrentTarget].standard,
		swatchWidth - 1, kSwatchHeight);
	fStandardSwatch->SetToolTip(B_TRANSLATE("Standard"));
	fStandardSwatch->MoveTo(kMargin,y);
	background->AddChild(fStandardSwatch);
	for (int32 i = 0; i < PW_PALETTE_SIZE; i++) {
		fPaletteSwatch[i]	= new ColorSwatchView("ColorPickerWindow::palette",
			new BMessage(PW_PALETTE_CLICKED), this, kPalette[i],
			swatchWidth - 1, kSwatchHeight);
		fPaletteSwatch[i]->MoveTo(kMargin + (i+1)*swatchWidth,y);
		background->AddChild(fPaletteSwatch[i]);
	}
	_ShowPalette();
	y	+= kSwatchHeight + kMargin;

	// recently used colors; the row is always there, unused slots empty
	int32	shown	= (historyCount < PW_HISTORY_SIZE) ? historyCount : PW_HISTORY_SIZE;
	fHistoryCount	= shown;
	for (int32 i = 0; i < shown; i++) {
		fHistorySwatch[i]	= new ColorSwatchView("ColorPickerWindow::history",
			new BMessage(PW_PALETTE_CLICKED), this, history[i],
			swatchWidth - 1, kSwatchHeight);
		fHistorySwatch[i]->MoveTo(kMargin + i*swatchWidth,y);
		background->AddChild(fHistorySwatch[i]);
	}
	rgb_color	emptySlot	= tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),
		B_DARKEN_1_TINT);
	for (int32 i = shown; i < PW_HISTORY_SIZE; i++) {
		BView	*slot	= new BView(BRect(0,0,swatchWidth - 1,kSwatchHeight),
			"ColorPickerWindow::emptyHistory", B_FOLLOW_NONE, 0);
		slot->SetViewColor(emptySlot);
		slot->MoveTo(kMargin + i*swatchWidth,y);
		background->AddChild(slot);
	}
	y	+= kSwatchHeight + kGap;

	fColorControl->MoveTo(1,y);
	fColorControl->SetTarget(this);
	background->AddChild(fColorControl);

	// alpha: a fifth ramp below BColorControl, its field in the R/G/B
	// column. Not inside the control's frame: there the control gets the
	// clicks and the slider can't be dragged.
	float	alphaBarRight	= textFieldLeft - kTextFieldsHSpacing;
	float	sliderHeight	= rampHeight + 2*kBevelSpacing;
	float	sliderTop		= y + height + kGap;
	float	alphaTop		= sliderTop + (sliderHeight - textFieldHeight)/2;
	fAlphaSlider = new AlphaSlider(B_HORIZONTAL, new BMessage(PW_ALPHA_CHANGED));
	fAlphaSlider->ResizeTo(alphaBarRight - 1, sliderHeight);
	fAlphaSlider->MoveTo(1, sliderTop);
	fAlphaSlider->SetTarget(this);
	fAlphaSlider->SetColor(color);
	fAlphaSlider->SetValue(color.alpha);
	background->AddChild(fAlphaSlider);

	BRect	alphaTextFrame(textFieldLeft, alphaTop,
		textFieldLeft + textFieldWidth, alphaTop + textFieldHeight);
	fAlphaText = new BTextControl(alphaTextFrame, "_alpha",
		B_TRANSLATE("Alpha:"), "255", new BMessage(PW_ALPHA_TEXT_ENTERED),
		B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
	fAlphaText->SetDivider(redText->Divider());
	for (int32 i = 0; i < 256; i++)
		fAlphaText->TextView()->DisallowChar(i);
	for (int32 i = '0'; i <= '9'; i++)
		fAlphaText->TextView()->AllowChar(i);
	fAlphaText->TextView()->SetMaxBytes(3);
	fAlphaText->SetAlignment(B_ALIGN_LEFT, B_ALIGN_RIGHT);
	fAlphaText->SetTarget(this);
	char	string[4];
	sprintf(string, "%d", color.alpha);
	fAlphaText->SetText(string);
	background->AddChild(fAlphaText);
	y	= max_c(alphaTop + textFieldHeight, sliderTop + sliderHeight) + kGap;
	_MarkSwatch(color);

	background->ResizeTo(width,y);
	ResizeTo(width,y);

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
			// the slider keeps its field in step, like BColorControl's
			// ramps keep "_red" etc.
			char	string[4];
			sprintf(string, "%" B_PRId32, fAlphaSlider->Value());
			fAlphaText->SetText(string);
			_ReportColor();
			break;
		}
		case PW_ALPHA_TEXT_ENTERED: {
			int32	value	= strtol(fAlphaText->Text(), NULL, 10);
			value			= max_c(0, min_c(255, value));
			fAlphaSlider->SetValue(value);
			// SetValue() is a no-op for the slider's current value, so
			// normalize what was typed here too
			char	string[4];
			sprintf(string, "%" B_PRId32, value);
			fAlphaText->SetText(string);
			_ReportColor();
			break;
		}
		case PW_PALETTE_CLICKED: {
			rgb_color	newColor;
			if (ColorFromMessage(message,newColor)) {
				_ShowColor(newColor);
				_ReportColor();
			}
			break;
		}
		case PW_TARGET_CLICKED: {
			int32	index;
			if (message->FindInt32("index",&index) == B_OK)
				_SelectTarget(index);
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
	BMessage	closed(PW_CLOSED);
	closed.AddBool("cancel", fCancelled);
	_Post(&closed);
	return BWindow::QuitRequested();
}


rgb_color
ColorPickerWindow::Color(void) const
{
	rgb_color	color	= fColorControl->ValueAsColor();
	color.alpha			= fAlphaSlider->Value();
	return color;
}


void
ColorPickerWindow::_ShowColor(rgb_color color)
{
	fColorControl->SetValue(color);
	fAlphaSlider->SetColor(color);
	fAlphaSlider->SetValue(color.alpha);
	char	string[4];
	sprintf(string, "%d", color.alpha);
	fAlphaText->SetText(string);
	_MarkSwatch(color);
}


void
ColorPickerWindow::_ShowPalette(void)
{
	const rgb_color	*palette	= fTargets[fCurrentTarget].light
		? kLightPalette : kPalette;
	for (int32 i = 0; i < PW_PALETTE_SIZE; i++)
		fPaletteSwatch[i]->SetColor(palette[i]);
}


// the ring sits on the swatch of the color shown, if there is one
void
ColorPickerWindow::_MarkSwatch(rgb_color color)
{
	bool	found	= false;
	ColorSwatchView	*swatches[1 + PW_PALETTE_SIZE + PW_HISTORY_SIZE];
	int32	count	= 0;
	swatches[count++]	= fStandardSwatch;
	for (int32 i = 0; i < PW_PALETTE_SIZE; i++)
		swatches[count++]	= fPaletteSwatch[i];
	for (int32 i = 0; i < fHistoryCount; i++)
		swatches[count++]	= fHistorySwatch[i];
	for (int32 i = 0; i < count; i++) {
		bool	match	= !found && (swatches[i]->Color() == color);
		swatches[i]->SetMarked(match);
		found	= found || match;
	}
}


void
ColorPickerWindow::_SelectTarget(int32 index)
{
	if ((index < 0) || (index >= fTargetCount))
		return;
	for (int32 i = 0; i < fTargetCount; i++)
		fTargetButton[i]->SetValue(i == index ? B_CONTROL_ON : B_CONTROL_OFF);
	if (index == fCurrentTarget)
		return;
	// what was picked for the old target is applied by the owner on
	// PW_TARGET_CHANGED; remember it for coming back
	fTargets[fCurrentTarget].color	= Color();
	fCurrentTarget	= index;
	BMessage	changed(PW_TARGET_CHANGED);
	changed.AddInt32("target",index);
	_Post(&changed);

	rgb_color	color	= fTargets[index].color;
	fStandardSwatch->SetColor(fTargets[index].standard);
	_ShowPalette();
	_ShowColor(color);
}


void
ColorPickerWindow::_ReportColor()
{
	rgb_color	color	= Color();
	_MarkSwatch(color);
	if (fMessage == NULL)
		return;
	BMessage	report(*fMessage);
	AddColorToMessage(&report, color);
	report.AddInt32("target", fCurrentTarget);
	_Post(&report);
}


void
ColorPickerWindow::_Post(BMessage *message)
{
	if (fTarget == NULL)
		return;
	BLooper	*looper	= fTarget->Looper();
	if (looper != NULL)
		looper->PostMessage(message, fTarget);
}
