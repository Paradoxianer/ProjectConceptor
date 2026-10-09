#ifndef COLOR_PICKER_WINDOW_H
#define COLOR_PICKER_WINDOW_H

#include <interface/Window.h>
#include <app/Handler.h>
#include <app/Message.h>
#include <support/String.h>

class BButton;
class BColorControl;
class BTextControl;
class AlphaSlider;
class ColorSwatchView;

// Sent to the window's target right before it quits (dismissed by
// clicking outside, Escape, or being closed some other way), so the
// owner can drop its cached pointer to this (now-invalid) window
// instead of needing to poll IsHidden()/Lock() to find out.
const uint32 PW_CLOSED = 'pwCL';
// Sent to the target when another target tab is chosen; "target" is its
// index. The color shown from then on is that target's.
const uint32 PW_TARGET_CHANGED = 'pwTC';

// curated palette, after the "Standard" swatch
const int32 PW_PALETTE_SIZE = 10;

// Number of "recently used" custom-color swatches shown in a row below
// the palette, most-recently-used first. The caller (ColorToolItem) owns
// the actual history; this window only displays up to this many.
const int32 PW_HISTORY_SIZE = 11;

// what a color is picked for: a node's fill, border or text, a
// connection's line
const int32 PW_MAX_TARGETS = 3;

struct ColorPickerTarget {
	BString		label;
	rgb_color	color;
	// what "Standard" resets to
	rgb_color	standard;
};

/**
 * @class ColorPickerWindow
 *
 * Popup window: a tab per target (fill, border, text) when there is more
 * than one, a row of curated swatches led by the target's standard color,
 * the recently used colors, a stock BColorControl and an AlphaSlider
 * drawn like its ramps. The picked color shows live on the selection.
 *
 * It opens on a normal click and stays open until dismissed by clicking
 * outside it (WindowActivated()) or Escape. Every change is reported
 * live to the target (a copy of message with the color and "target"
 * index); committing what was last reported is the target's business
 * once PW_CLOSED arrives.
 *
 * All content lives inside one full-window background view rather than as
 * direct siblings of this BWindow: overlapping sibling views added
 * straight to a BWindow don't route mouse events to the right one.
 */
class ColorPickerWindow : public BWindow {
public:
							ColorPickerWindow(BRect frame,
								BMessage *message, BHandler *target,
								const ColorPickerTarget *targets,
								int32 targetCount, int32 currentTarget,
								const rgb_color *history = NULL,
								int32 historyCount = 0);
	virtual					~ColorPickerWindow();

	virtual	void			MessageReceived(BMessage *message);
	virtual	void			WindowActivated(bool active);
	virtual	bool			QuitRequested(void);

			rgb_color		Color(void) const;

	// Marks this close as a cancel rather than a commit - called by the
	// Escape key filter before requesting the quit, so QuitRequested()
	// can tell the target not to apply whatever was last previewed.
			void			Cancel(void) { fCancelled = true; }

private:
			void			_ReportColor();
			void			_ShowColor(rgb_color color);
			void			_SelectTarget(int32 index);
			void			_Post(BMessage *message);

			BColorControl	*fColorControl;
			AlphaSlider		*fAlphaSlider;
			BTextControl	*fAlphaText;
			BButton			*fTargetButton[PW_MAX_TARGETS];
			ColorSwatchView	*fStandardSwatch;
			ColorPickerTarget	fTargets[PW_MAX_TARGETS];
			int32			fTargetCount;
			int32			fCurrentTarget;
			BMessage		*fMessage;
			BHandler		*fTarget;
			bool			fCancelled;
};

#endif // COLOR_PICKER_WINDOW_H
