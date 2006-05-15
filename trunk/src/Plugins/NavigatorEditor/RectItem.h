#ifndef RECT_ITEM_H
#define RECT_ITEM_H

#include <interface/Font.h>
#include <interface/ListItem.h>
#include <interface/Rect.h>
#include <interface/TextControl.h>
#include <interface/View.h>


#include "BaseListItem.h"

class RectItem : public BaseListItem
{
public:
					RectItem(char *newLabel,BRect newRect, uint32 level = 0, bool expanded = true);
	virtual	BRect	GetRect(void);

	virtual	void	Select(void);
	virtual	void	Deselect(void);
	virtual void	SetExpanded(bool expande);
	virtual	void	Update(BView *owner, const BFont *font);
	virtual	void	DrawItem(BView *owner, BRect bounds, bool complete = false);


protected:
	BTextControl	*left,*top,*right,*bottom;
	char 			*sleft,*stop,*sright,*sbottom;
	char			*label;
	BRect			rect;
	BView			*owner;
	float			textLine;
	rgb_color		foreground;
	rgb_color		background;
	rgb_color		backgroundHi;
	float			separated;
	float			textControlHeight;
private:
};
#endif
