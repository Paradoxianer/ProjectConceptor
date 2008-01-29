#ifndef FLOAT_ITEM_H
#define FLOAT_ITEM_H

#include "ConfigItem.h"

#include <interface/Font.h>
#include <interface/Rect.h>
#include <interface/TextControl.h>
#include <interface/View.h>

#include <stdlib.h>

class FloatItem : public ConfigItem, BTextControl
{
public:
					FloatItem(char *newLabel,float newVlaue, BMessage *message);
	virtual	void*	GetValue(void);
	virtual void	SetValue(void* newValue);
		
	virtual	BView*	ViewForValue(void){return this;};

};
#endif
