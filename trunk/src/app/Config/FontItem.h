#ifndef FONT_ITEM_H
#define FONT_ITEM_H


#include <interface/Rect.h>
#include <interface/TextControl.h>
#include <interface/View.h>

#include "AFont.h"

#include <stdlib.h>

class FontItem 
{
public:
					FontItem(char *newLabel,char* newVlaue, BMessage *message);
	virtual	AFont*	GetValue(void);
	virtual void	SetValue(AFont* newValue);
};
#endif
