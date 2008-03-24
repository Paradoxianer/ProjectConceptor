#ifndef MESSAGE_LISTVIEW_H
#define MESSAGE_LISTVIEW_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <app/Message.h>
#include <interface/OutlineListView.h>
#include <interface/Point.h>
#include <interface/Rect.h>

class PDocument;
const uint32	M_L_VALUE_CHANGED		= 'mlVC';

class MessageListView : public BOutlineListView
{

public:
						MessageListView(PDocument *document,BRect rect, BMessage * forContainer);
	virtual	void		MouseDown(BPoint point);
	virtual void		AttachedToWindow(void);
	virtual	BMessage*	GetContainer(void){return container;};
	virtual	void		ValueChanged(void);
	virtual void		SetContainer(BMessage *message);
	virtual void		SetDoc(PDocument *document){doc=document;};
	
	
	virtual	void		MessageReceived(BMessage *msg);

protected:
	virtual void		AddMessage(BMessage *message, BListItem* superItem);
	BMessage			*container;
	BMessage			*editMessage;
	BMessage			*baseEditMessage;
	PDocument			*doc;
private:
};
#endif
