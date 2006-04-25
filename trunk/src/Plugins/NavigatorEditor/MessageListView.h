#ifndef MESSAGE_LISTVIEW_H
#define MESSAGE_LISTVIEW_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <app/Message.h>
#include <interface/OutlineListView.h>
#include <interface/Point.h>
#include <interface/Rect.h>

class MessageListView : public BOutlineListView
{

public:
						MessageListView(BRect rect, BMessage * forContainer);
	virtual	void		MouseDown(BPoint point);
	virtual	BMessage*	GetContainer(void){return container;};
	virtual	void		ValueChanged(void);

protected:
	virtual void		AddMessage(BMessage *message, BListItem* superItem);
	
	BMessage		*container;
private:
};
#endif
