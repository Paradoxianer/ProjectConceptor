#include <app/Handler.h>
#include <app/Looper.h>

#include "ShortCutFilter.h"


ShortCutFilter::ShortCutFilter(BMessage* shortcutList):BMessageFilter(B_PROGRAMMED_DELIVERY,B_LOCAL_SOURCE)
{
	shortcutVector= new BKeyedVector<uint32, shortcut*>();
	AddShortCutList(shortcutList);
}


BMessage* ShortCutFilter::GetShortcutList(void)
{
	shortcut	*theShortcut	= NULL;
	BMessage	*returnMessage	= new BMessage();
	BLooper		*tmpLooper		= NULL;
	for (uint32 i=0;i<shortcutVector->CountItems();i++)
	{
		theShortcut = shortcutVector->ValueAt(i);
		returnMessage->AddInt32("key",theShortcut->key);
		returnMessage->AddInt32("modifiers",theShortcut->modifiers);
		returnMessage->AddMessage("message",(theShortcut->sendMessage));
		returnMessage->AddPointer("handler",(theShortcut->sentTo)->Target(&tmpLooper));
	}
	return returnMessage;
}
filter_result ShortCutFilter::Filter(BMessage *message, BHandler **target)
{
	uint32		key;
	shortcut	*theShortcut	= NULL;
	switch(message->what) 
	{
		case B_KEY_DOWN:
		{
			if (message->FindInt32("raw_char",(int32 *)&key) == B_OK)
			{
				BMessenger *messenger		= NULL;
				BMessage	*sendMessage	= NULL;
				theShortcut	= shortcutVector->ValueFor(key);
				if (theShortcut != NULL)
				{
					messenger	= theShortcut->sentTo;
					sendMessage	= theShortcut->sendMessage;
					messenger->SendMessage(sendMessage);
				}
			}
		}
	}
	return B_DISPATCH_MESSAGE;
}
void ShortCutFilter::SetShortCutList(BMessage *shortcutList)
{
	shortcutVector->MakeEmpty();
	AddShortCutList(shortcutList);
}

void ShortCutFilter::AddShortCutList(BMessage *shortcutList)
{
	uint32		key;
	uint32		modifiers;
	int32		i				= 0;
	shortcut	*theShortcut	= NULL;
	BHandler	*handler		= NULL;

	
	while (shortcutList->FindInt32("key",i,(int32 *)&key) == B_OK)
	{
		handler 	= NULL;
		modifiers	= 0;
		theShortcut	= new shortcut;
		theShortcut->sendMessage = new BMessage();
		theShortcut->key=key;
		shortcutList->FindInt32("modifiers",i,(int32 *)&(theShortcut->modifiers));
		shortcutList->FindMessage("message",i,(theShortcut->sendMessage));
		shortcutList->FindPointer("handler",i,(void **)&handler);
		theShortcut->sentTo= new BMessenger(handler);
		i++;
		shortcutVector->AddItem(key,theShortcut);
	}
}

void ShortCutFilter::RemoveShortCutList(BMessage *shortcutList)
{
	uint32		key;
	int32 i	= 0;
	while (shortcutList->FindInt32("key",i,(int32 *)&key) == B_OK)
	{
		shortcutVector->RemoveItemFor(key);
		i++;
	}
}
