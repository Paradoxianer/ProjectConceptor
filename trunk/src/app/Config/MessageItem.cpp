#include "MessageItem.h"
#include <stdio.h>


MessageItem::MessageItem(char *text, BMessage *message, uint32 level = 0, bool expanded = true): BStringItem(text, level, expanded)
{
	container=message;
}
