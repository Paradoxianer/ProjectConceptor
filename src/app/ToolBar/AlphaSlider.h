/*
 * Adapted for ProjectConceptor from Haiku's own Icon-O-Matic app
 * (~/repos/haiku/src/apps/icon-o-matic/generic/gui/panel/color_picker/
 * AlphaSlider.{h,cpp}).
 *
 * Copyright 2006-2012 Stephan Assmus <superstippi@gmx.de>
 * Distributed under the terms of the MIT License.
 *
 * ProjectConceptor addition: a small checkerboard-aware swatch "end cap"
 * at the end of the gradient bar (see _EndCapRect()/_DrawEndCap() in the
 * .cpp), showing the resulting color at the current alpha value instead
 * of just a position marker - drawn with the same checkerboard style
 * ColorSwatchView uses for the palette/history rows elsewhere in
 * ColorPickerWindow, so this reads as the same "color swatch" visual
 * language rather than a foreign widget bolted on next to it.
 */

#ifndef ALPHA_SLIDER_H
#define ALPHA_SLIDER_H

#include <interface/Control.h>

class BBitmap;

class AlphaSlider : public BControl {
public:
								AlphaSlider(orientation dir = B_HORIZONTAL,
									BMessage *message = NULL,
									border_style border = B_FANCY_BORDER);
	virtual						~AlphaSlider();

	// BControl interface
	virtual	void				WindowActivated(bool active);
	virtual	void				MakeFocus(bool focus);

	virtual	BSize				MinSize();
	virtual	BSize				PreferredSize();
	virtual	BSize				MaxSize();

	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseUp(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32 transit,
										const BMessage *dragMessage);

	virtual	void				KeyDown(const char *bytes, int32 numBytes);

	virtual	void				Draw(BRect updateRect);
	virtual	void				FrameResized(float width, float height);

	virtual	void				SetValue(int32 value);
	virtual	void				SetEnabled(bool enabled);

	// AlphaSlider
			void				SetColor(const rgb_color &color);

			bool				IsTracking() const { return fDragging; }

private:
			void				_UpdateColors();
			void				_AllocBitmap(int32 width, int32 height);
			BRect				_BitmapRect() const;
			BRect				_EndCapRect() const;
			void				_DrawEndCap();
			int32				_ValueFor(BPoint where) const;

private:
			BBitmap				*fBitmap;
			rgb_color			fColor;
			bool				fDragging;
			orientation			fOrientation;
			border_style		fBorderStyle;
};

#endif // ALPHA_SLIDER_H
