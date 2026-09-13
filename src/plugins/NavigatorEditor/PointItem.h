#ifndef POINT_ITEM_H
#define POINT_ITEM_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <interface/Font.h>
#include <interface/ListItem.h>
#include <interface/Point.h>
#include <interface/Rect.h>
#include <interface/TextControl.h>
#include <interface/View.h>

#include "BaseListItem.h"

class PointItem : public BaseListItem
{
public:
						PointItem(char *newLabel, BPoint newPoint, uint32 level = 0, bool expanded = true);
	virtual	BPoint		GetPoint(void);

	virtual	void		Select(void);
	virtual const char	*GetLabel(void){return label;};
	virtual	void		Deselect(void);
	virtual void		SetExpanded(bool expande);
	virtual	void		Update(BView *owner, const BFont *font);
	virtual	void		DrawItem(BView *owner, BRect bounds, bool complete = false);
	virtual status_t	Invoke(BMessage *message = NULL);

protected:
	BTextControl		*x,*y;
	char				*sx,*sy;
	char				*label;
	float				textLine;
	rgb_color			foreground;
	rgb_color			background;
	rgb_color			backgroundHi;
	float				textControlHeight;
private:
};
#endif
