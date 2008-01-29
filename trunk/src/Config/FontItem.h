#ifndef STRING_ITEM_H
#define STRING_ITEM_H

#include "ConfigItem.h"

#include <interface/Font.h>
#include <interface/Rect.h>
#include <interface/TextControl.h>
#include <interface/View.h>

#include <stdlib.h>

class FontItem : public ConfigItem, BTextControl
{
public:
					FontItem(char *newLabel,char* newVlaue, BMessage *message);
	virtual	void*	GetValue(void);
	virtual void	SetValue(void* newValue);
	virtual	BView*	ViewForValue(void){return this;};

};
#endif
