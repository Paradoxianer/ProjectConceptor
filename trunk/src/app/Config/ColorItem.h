#ifndef COLOR_ITEM_H
#define COLOR_ITEM_H

#include <interface/Button.h>
#include <interface/Control.h>
#include <interface/ColorControl.h>
#include <interface/Font.h>
#include <interface/Rect.h>
#include <interface/View.h>

#include <stdlib.h>

class ColorItem : 	BControl
{
public:
						ColorItem(BRect frame, char *newLabel, rgb_color newValue, BMessage *message);
	virtual	rgb_color	Color(void);
	virtual void		SetColor(rgb_color newValue);

	virtual void		Draw(BRect updateRect);
protected:
	char			*label;
	BButton			*button;
	BColorControl	*colorChecker;	
	
};
#endif
