#ifndef STRING_ITEM_H
#define STRING_ITEM_H
/*
 * @author Paradoxon powered by Jesus Christ
 */

#include <interface/Font.h>
#include <interface/Rect.h>
#include <interface/TextControl.h>
#include <interface/View.h>

#include <stdlib.h>

class StringItem : public BTextControl
{
public:
					StringItem(char *newLabel,char* newVlaue, BMessage *message);
	virtual	void*	GetValue(void);
	virtual void	SetValue(void* newValue);
	virtual	BView*	ViewForValue(void){return this;};

};
#endif
