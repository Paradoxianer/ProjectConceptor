#ifndef MESSAGE_LISTVIEW_H
#define MESSAGE_LISTVIEW_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <app/Message.h>
#include <interface/OutlineListView.h>
#include <interface/Point.h>
#include <interface/Rect.h>
#include <support/String.h>

class PDocument;
class NavigatorEditor;
class ColorItem;
const uint32	M_L_VALUE_CHANGED		= 'mlVC';

class MessageListView : public BOutlineListView
{

public:
						MessageListView(PDocument *document,BRect rect, BMessage * forContainer, NavigatorEditor *forEditor = NULL);
	virtual	void		MouseDown(BPoint point);
	virtual void			AttachedToWindow(void);
	virtual	BMessage*	GetContainer(void){return container;};
	virtual	void		ValueChanged(void);
	/** shows only the rows whose field name or text contains filter,
	 * with the groups they are in; a matching group shows all of it */
			void		SetFilter(const char *newFilter);

	virtual	void		MessageReceived(BMessage *msg);

protected:
	/** showAll: the group matched the filter, show all of it */
	virtual void		AddMessage(BMessage *message, BListItem* superItem,
							bool showAll = false);
			/** removes a group header nothing under it is shown for */
			void		DropIfEmpty(BListItem *header, bool showAll);
			void		AddNodeUnder(BMessage *node, BListItem *header,
							bool showAll);
	BMessage			*container;
	BMessage			*editMessage;
	BMessage			*baseEditMessage;
	PDocument			*doc;
	NavigatorEditor		*editor;
	// PW_CLOSED (unlike COLOR_ITEM_REPORT, which we construct ourselves)
	// is ColorPickerWindow's own fixed notification and carries no way
	// to say which ColorItem's picker it was - this is the one open at
	// a time this list view actually has, tracked so PW_CLOSED knows
	// who to tell.
	ColorItem			*activeColorItem;
	BString				filter;
private:
};
#endif
