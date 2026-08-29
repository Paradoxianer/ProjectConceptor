#ifndef COLOR_PICKER_WINDOW_H
#define COLOR_PICKER_WINDOW_H

#include <interface/Window.h>
#include <app/Handler.h>
#include <app/Message.h>

class BColorControl;
class BTextControl;
class AlphaSlider;
class ColorSwatchView;

// Sent to the window's target right before it quits (dismissed by
// clicking outside, Escape, or being closed some other way), so the
// owner can drop its cached pointer to this (now-invalid) window
// instead of needing to poll IsHidden()/Lock() to find out.
const uint32 PW_CLOSED = 'pwCL';

// Number of algorithmically-generated palette swatches shown above the
// custom color control - a hue sweep, not a hand-curated list, so this
// widget doesn't need per-project color curation to be reusable.
const int32 PW_PALETTE_SIZE = 8;

// Number of "recently used" custom-color swatches shown in a second row
// below the palette, most-recently-used first. The caller (ColorToolItem)
// owns the actual history; this window only displays up to this many of
// what it's given - kept equal to PW_PALETTE_SIZE just so both rows are
// the same width, not because the two need to match.
const int32 PW_HISTORY_SIZE = 8;

/**
 * @class ColorPickerWindow
 *
 * Popup window offering a row of quick-pick palette swatches plus a
 * stock BColorControl and an AlphaSlider for anything not in the
 * palette. Replaces ColorToolItem's old ad hoc colorWindow / custom
 * MouseDown-MouseUp popup-hide logic, which closed the popup on a normal
 * click before a color could even be picked. This window opens on a
 * normal click and stays open until explicitly dismissed - clicking
 * outside it (detected via WindowActivated()) or pressing Escape -
 * instead of being tied to the mouse button being held down.
 *
 * Reports the picked rgb_color (RGB from the color control, alpha from
 * the slider) live: an explicit message is sent to the target each time
 * a palette swatch is clicked or either control changes, rather than
 * relying on a separate OK/commit step or on BButton's native
 * click-Invoke() semantics.
 *
 * The alpha row (AlphaSlider + fAlphaText) is laid out to match
 * BColorControl's own R/G/B rows exactly - fAlphaText's frame/divider are
 * copied from BColorControl's own "_red" BTextControl (found via the
 * public BView::FindView(), not private API) rather than recomputing the
 * label/field geometry independently, so the two can't drift apart.
 *
 * All real content lives inside a single full-window background BView
 * (added first, in the constructor) rather than as direct siblings of
 * this BWindow - nested parent/child views reliably route mouse events
 * to whichever child is actually under the pointer; a set of overlapping
 * *sibling* views added directly to a BWindow do not (the first-added
 * one - originally an earlier, sibling-based version of this background
 * fix - swallowed every click in the window, an actual regression fixed
 * before it shipped).
 */
class ColorPickerWindow : public BWindow {
public:
							ColorPickerWindow(BRect frame, rgb_color color,
								BMessage *message, BHandler *target,
								const rgb_color *history = NULL,
								int32 historyCount = 0);
	virtual					~ColorPickerWindow();

	virtual	void			MessageReceived(BMessage *message);
	virtual	void			WindowActivated(bool active);
	virtual	bool			QuitRequested(void);

			void			SetColor(rgb_color color);
			rgb_color		Color(void) const;

	// Marks this close as a cancel rather than a commit - called by the
	// Escape key filter before requesting the quit, so QuitRequested()
	// can tell the target not to apply whatever was last previewed.
			void			Cancel(void) { fCancelled = true; }

private:
			void			_ReportColor();
			void			_ApplyColor(rgb_color color);

			BColorControl	*fColorControl;
			AlphaSlider		*fAlphaSlider;
			BTextControl	*fAlphaText;
			ColorSwatchView	*fPaletteSwatch[PW_PALETTE_SIZE];
			ColorSwatchView	*fHistorySwatch[PW_HISTORY_SIZE];
			BMessage		*fMessage;
			BHandler		*fTarget;
			bool			fCancelled;
};

#endif // COLOR_PICKER_WINDOW_H
