#ifndef FLOAT_ITEM_H
#define FLOAT_ITEM_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <interface/Font.h>
#include <interface/ListItem.h>
#include <interface/Rect.h>
#include <interface/TextControl.h>
#include <interface/View.h>

#include <stdlib.h>
#include <stdio.h>

#include "BaseListItem.h"

class FloatItem : public BaseListItem
{
public:
					FloatItem(char *newLabel,float newVlaue, uint32 level = 0, bool expanded = true);
	virtual	float	GetFloat(void){return atof(svalue);};
	virtual	void	SetFloat(float *newFloat){sprintf(svalue,"%.2f",newFloat);};

	virtual void	ValueChange(void){value->SetText(svalue);};

	virtual	void	Select(void);
	virtual	void	Deselect(void);
	virtual void	SetExpanded(bool expande);
	virtual	void	Update(BView *owner, const BFont *font);
	virtual	void	DrawItem(BView *owner, BRect bounds, bool complete = false);

protected:
	BTextControl	*value;
	char 			*svalue;
	char			*label;
	float			textLine;
	rgb_color		foreground;
	rgb_color		background;
	rgb_color		backgroundHi;
	float			separated;
private:
};
#endif
