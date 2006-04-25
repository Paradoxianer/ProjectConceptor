#ifndef STRING_ITEM_H
#define STRING_ITEM_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <interface/Font.h>
#include <interface/ListItem.h>
#include <interface/Rect.h>
#include <interface/TextControl.h>
#include <interface/View.h>

#include "BaseListItem.h"


class PointerItem : public BaseListItem
{
public:
					PointerItem(char *newLabel,void *newValue, uint32 level = 0, bool expanded = true);
	virtual	void*	GetPointer(void){return value;};

	virtual	void	Select(void);
	virtual	void	Deselect(void);
	virtual void	SetExpanded(bool expande);
	virtual	void	Update(BView *owner, const BFont *font);
	virtual	void	DrawItem(BView *owner, BRect bounds, bool complete = false);

protected:
	char			*label;
	void			*value;
	BView			*owner;
	float			textLine;
	rgb_color		foreground;
	rgb_color		background;
	rgb_color		backgroundHi;
	float			separated;
private:
};
#endif
