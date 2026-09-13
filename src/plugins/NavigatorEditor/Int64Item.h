#ifndef INT64_ITEM_H
#define INT64_ITEM_H
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

class Int64Item : public BaseListItem
{
public:
						Int64Item(char *newLabel,int64 newValue, uint32 level = 0, bool expanded = true);
	virtual	int64		GetValue(void){return (int64)atoll(value->Text());};

	virtual void		ValueChange(void){value->SetText(svalue);};
	virtual const char	*GetLabel(void){return label;};

	virtual	void		Select(void);
	virtual	void		Deselect(void);
	virtual void		SetExpanded(bool expande);
	virtual	void		Update(BView *owner, const BFont *font);
	virtual	void		DrawItem(BView *owner, BRect bounds, bool complete = false);
	virtual status_t	Invoke(BMessage *message = NULL);

protected:
	BTextControl		*value;
	char 				*svalue;
	char				*label;
	float				textLine;
	rgb_color			foreground;
	rgb_color			background;
	rgb_color			backgroundHi;
private:
};
#endif
