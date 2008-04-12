#ifndef PATTERN_ITEM_H
#define PATTERN_ITEM_H


#include <interface/Font.h>
#include <interface/Rect.h>
#include <interface/MenuField.h>
#include <interface/View.h>

#include <stdlib.h>

class PatternItem : BView
{
public:
					PatternItem(char *newLabel,pattern newVlaue, BMessage *message);
	virtual	void*	GetValue(void);
	virtual void	SetValue(void* newValue);
	virtual	BView*	ViewForValue(void){return this;};

};
#endif
