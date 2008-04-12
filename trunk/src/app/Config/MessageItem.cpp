#include "MessageItem.h"
#include <stdio.h>


MessageItem::MessageItem(char *text, BMessage *message, uint32 level, bool expanded): BStringItem(text, level, expanded)
{
	container=message;
}
