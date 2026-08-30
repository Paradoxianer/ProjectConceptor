#ifndef COLOR_SWATCH_VIEW_H
#define COLOR_SWATCH_VIEW_H

#include <interface/View.h>
#include <app/Message.h>
#include <app/Handler.h>

// Packs/unpacks an rgb_color into a BMessage as raw B_RGB_COLOR_TYPE data
// under "color" - the shared wire format ColorSwatchView and
// ColorPickerWindow both use to report a picked color.
void	AddColorToMessage(BMessage *message, rgb_color color);
bool	ColorFromMessage(const BMessage *message, rgb_color &color);

/**
 * @class ColorSwatchView
 *
 * A single small, clickable, paintable color patch - renders flat when
 * opaque, checkerboard-blended when the color's alpha is below 255.
 * Adapted from Haiku's own Icon-O-Matic app
 * (~/repos/haiku/src/apps/icon-o-matic/generic/gui/SwatchView.{h,cpp},
 * Copyright 2006-2012 Stephan Assmus, MIT licensed) - drag-and-drop
 * support intentionally dropped, not needed here.
 *
 * Generic on purpose: knows nothing about ColorToolItem, nodes, or
 * connections - just a color patch that invokes a message on its target
 * when clicked, so it can be reused in other toolbar items or projects.
 */
class ColorSwatchView : public BView {
public:
							ColorSwatchView(const char *name,
								BMessage *message, BHandler *target,
								rgb_color color,
								float width = 24.0, float height = 24.0);
	virtual					~ColorSwatchView();

	virtual	void			Draw(BRect updateRect);
	virtual	void			MouseDown(BPoint where);
	virtual	void			MouseUp(BPoint where);

			void			SetColor(rgb_color color);
	inline	rgb_color		Color() const { return fColor; }

			void			SetTarget(BHandler *target) { fTarget = target; }

private:
			void			_Invoke();

			rgb_color		fColor;
			BPoint			fTrackingStart;
			BMessage		*fClickMessage;
			BHandler		*fTarget;
};

#endif // COLOR_SWATCH_VIEW_H
