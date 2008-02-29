#ifndef BOOL_ITEM_H
#define BOOL_ITEM_H

#include <interface/Font.h>
#include <interface/Rect.h>
#include <interface/CheckBox.h>
#include <interface/View.h>

#include <stdlib.h>

#include "BaseListItem.h"


class BoolItem : public BaseListItem
{
public:
					BoolItem(char *newLabel,bool newVlaue, uint32 level = 0, bool expanded = true);
	virtual	bool	GetValue(void){return value->Value();};
	virtual void	SetValue(bool newValue){value->SetValue(newValue);};
	
	virtual void	ValueChange(void);

	virtual	void	Select(void);
	virtual	void	Deselect(void);
	virtual void	SetExpanded(bool expande);
	virtual	void	Update(BView *owner, const BFont *font);
	virtual	void	DrawItem(BView *owner, BRect bounds, bool complete = false);

	virtual status_t	Invoke(BMessage *message = NULL);

protected:
	BCheckBox		*value;
	char 			*svalue;
	char			*label;
	float			textLine;
	rgb_color		foreground;
	rgb_color		background;
	rgb_color		backgroundHi;
private:
};
#endif
