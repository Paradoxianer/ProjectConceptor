#ifndef RECT_ITEM_H
#define RECT_ITEM_H

#include <interface/Font.h>
#include <interface/Rect.h>
#include <interface/Box.h>
#include <interface/TextControl.h>
#include <interface/View.h>

#include <stdlib.h>

#include "ConfigItem.h"
/**
 * @class RectItem
 * @brief Generates a ConfigItem for a BRect object
 *
 *@todo live lokalisation
 */

class RectItem : public ConfigItem, BBox
{
public:
					RectItem(char *newLabel,BRect newVlaue, BMessage *message);
	virtual	void*	GetValue(void);
	virtual void	SetValue(void* newValue);
	virtual	BView*	ViewForValue(void){return this;};

protected:
	BTextControl	*left, *top,*bottom, *right;
	char*			label;
};
#endif
