#ifndef RECT_ITEM_H
#define RECT_ITEM_H

#include <interface/Font.h>
#include <interface/Rect.h>
#include <interface/Box.h>
#include <interface/TextControl.h>
#include <interface/View.h>

#include <stdlib.h>

/**
 * @class RectItem
 * @brief Generates a ConfigItem for a BRect object
 *
 *@todo live lokalisation
 */

class RectItem : public BControl
{
public:
					RectItem(BRect newFrame,char *newLabel,BRect newVlaue, BMessage *message = NULL);
	virtual	BRect	Rect(void);
	virtual void	SetRect(BRect newValue);

protected:
	BTextControl	*left, *top,*bottom, *right;
};
#endif
