#ifndef COLOR_ITEM_H
#define COLOR_ITEM_H

#include <interface/Button.h>
#include <interface/ColorControl.h>
#include <interface/Font.h>
#include <interface/Rect.h>
#include <interface/View.h>

#include <stdlib.h>

#include "ConfigItem.h"


class ColorItem : public ConfigItem, BView
{
public:
						ColorItem(char *newLabel,rgb_color newVlaue, BMessage *message);
	virtual	void*		GetValue(void);
	virtual void		SetValue(void* newValue);
	
	virtual void		ValueChange(void){};
	virtual	BView*		ViewForValue(void){return this;};
	virtual void		Draw(BRect updateRect);

protected:
	char			*label;
	BButton			*button;
	BColorControl	*colorChecker;	
	
};
#endif
