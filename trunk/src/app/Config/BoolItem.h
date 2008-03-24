#ifndef BOOL_ITEM_H
#define BOOL_ITEM_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "ConfigItem.h"

#include <interface/Font.h>
#include <interface/Rect.h>
#include <interface/CheckBox.h>
#include <interface/View.h>

#include <stdlib.h>

class BoolItem : public ConfigItem, BCheckBox
{
public:
					BoolItem(char *newLabel,bool newVlaue, BMessage *message);
	virtual	void*	GetValue(void);
	virtual void	SetValue(void* newValue);
	virtual	BView*	ViewForValue(void){return this;};

};
#endif
