#ifndef NODE_ITEM_H
#define NODE_ITEM_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <interface/Font.h>
#include <interface/ListItem.h>
#include <interface/Rect.h>

#include <interface/View.h>


#include "BaseListItem.h"

class NodeItem : public BaseListItem
{
public:
						NodeItem(BMessage *newNode, uint32 level = 0, bool expanded = true);
	virtual	BMessage*	GetNode(void){return node;};
	virtual void		SetNode(BMessage *newNode){node=newNode;};

	virtual void		ValueChanged(void);

	virtual	void		Select(void);
	virtual	void		Deselect(void);
	virtual void		SetExpanded(bool expande);
	virtual void		Update(BView *owner, const BFont *font);
	virtual	void		DrawItem(BView *owner, BRect bounds, bool complete = false);

protected:
	char				*label;
	BMessage			*node;
	rgb_color			foreground;
	rgb_color			background;
	rgb_color			backgroundHi;

private:
};
#endif
