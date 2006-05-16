#ifndef STRING_ITEM_H
#define STRING_ITEM_H

#include <interface/Font.h>
#include <interface/ListItem.h>
#include <interface/Rect.h>
#include <interface/TextControl.h>
#include <interface/View.h>

#include "BaseListItem.h"


class StringItem : public BaseListItem
{
public:
						StringItem(char *newLabel,char *newString, uint32 level = 0, bool expanded = true);
	virtual	const char*	GetString(void){return textControl->Text();};

	virtual	void		Select(void);
	virtual	void		Deselect(void);
	virtual void		SetExpanded(bool expande);
	virtual	void		Update(BView *owner, const BFont *font);
	virtual	void		DrawItem(BView *owner, BRect bounds, bool complete = false);
	virtual status_t	Invoke(BMessage *message = NULL);
protected:
	BTextControl		*textControl;
	char				*label;
	char				*string;
	float				textLine;
	rgb_color			foreground;
	rgb_color			background;
	rgb_color			backgroundHi;
private:
};
#endif
