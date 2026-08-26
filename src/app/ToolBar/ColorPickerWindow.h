#ifndef COLOR_PICKER_WINDOW_H
#define COLOR_PICKER_WINDOW_H

#include <interface/Window.h>
#include <app/Handler.h>
#include <app/Message.h>

class BColorControl;
class AlphaSlider;

// Sent to the window's target right before it quits (dismissed by
// clicking outside, Escape, or being closed some other way), so the
// owner can drop its cached pointer to this (now-invalid) window
// instead of needing to poll IsHidden()/Lock() to find out.
const uint32 PW_CLOSED = 'pwCL';

/**
 * @class ColorPickerWindow
 *
 * Small popup window pairing a stock BColorControl with an AlphaSlider.
 * Replaces ColorToolItem's old ad hoc colorWindow / custom MouseDown-
 * MouseUp popup-hide logic, which closed the popup on a normal click
 * before a color could even be picked. This window opens on a normal
 * click and stays open until explicitly dismissed - clicking outside it
 * (detected via WindowActivated()) or pressing Escape - instead of being
 * tied to the mouse button being held down.
 *
 * Reports the picked rgb_color (RGB from the color control, alpha from
 * the slider) live: an explicit message is sent to the target each time
 * either control changes, rather than relying on a separate OK/commit
 * step or on BButton's native click-Invoke() semantics.
 */
class ColorPickerWindow : public BWindow {
public:
							ColorPickerWindow(BRect frame, rgb_color color,
								BMessage *message, BHandler *target);
	virtual					~ColorPickerWindow();

	virtual	void			MessageReceived(BMessage *message);
	virtual	void			WindowActivated(bool active);
	virtual	bool			QuitRequested(void);

			void			SetColor(rgb_color color);
			rgb_color		Color(void) const;

private:
			void			_ReportColor();

			BColorControl	*fColorControl;
			AlphaSlider		*fAlphaSlider;
			BMessage		*fMessage;
			BHandler		*fTarget;
};

#endif // COLOR_PICKER_WINDOW_H
