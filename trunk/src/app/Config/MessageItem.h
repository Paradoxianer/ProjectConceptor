#ifndef MESSAGE_ITEM_H
#define MESSAGE_ITEM_H

#include <interface/Font.h>
#include <interface/ListItem.h>
#include <interface/Rect.h>
#include <interface/TextControl.h>
#include <interface/View.h>

#include <stdlib.h>

class MessageItem : public BStringItem
{
public:
				MessageItem(char *text, BMessage *message, uint32 level = 0, bool expanded = true);
	BMessage*	Message(void){return container;}
protected:
	BMessage	*container;
	
};
#endif
