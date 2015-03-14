#include <app/Handler.h>
#include <app/Looper.h>
#include <support/Debug.h>

#include "ShortCutFilter.h"


ShortCutFilter::ShortCutFilter(BMessage* shortcutList):BMessageFilter(B_ANY_DELIVERY,B_ANY_SOURCE)
{
	shortcutMap= map<uint32, shortcut*>();
	AddShortCutList(shortcutList);
}


BMessage* ShortCutFilter::GetShortcutList(void)
{
	map<uint32, shortcut*>::iterator iter;
	shortcut	*theShortcut	= NULL;
	BMessage	*returnMessage	= new BMessage();
	for (iter =shortcutMap.begin();iter != shortcutMap.end();iter++)
	{
		theShortcut = iter->second;
		returnMessage->AddInt32("key",theShortcut->key);
		returnMessage->AddInt32("modifiers",theShortcut->modifiers);
		returnMessage->AddMessage("message",theShortcut->sendMessage);
		returnMessage->AddPointer("handler",theShortcut->sentTo);
	}
	return returnMessage;
}
filter_result ShortCutFilter::Filter(BMessage *message, BHandler **target)
{
	uint32		key;
	uint32		modifiers;
	shortcut	*theShortcut	= NULL;
	target = target;
	if (message->what == B_KEY_DOWN)
	{
		if (message->FindInt32("raw_char",(int32 *)&key) == B_OK)
		{
			message->FindInt32("modifiers",(int32 *)&modifiers);
			BMessage	*sendMessage	= NULL;
			theShortcut	= shortcutMap[key];
			if (theShortcut != NULL)
			{
				if ( (theShortcut->modifiers == 0) || ((theShortcut->modifiers & modifiers) != 0 ) )
				{
					sendMessage	= theShortcut->sendMessage;
					theShortcut->sentTo->SendMessage(sendMessage);
				}
			}
		}
	}
	return B_DISPATCH_MESSAGE;
}
void ShortCutFilter::SetShortCutList(BMessage *shortcutList)
{
	shortcutMap.clear();
	AddShortCutList(shortcutList);
}

void ShortCutFilter::AddShortCutList(BMessage *shortcutList)
{
	uint32		key;
	uint32		modifiers;
	int32		i				= 0;
	shortcut	*theShortcut	= NULL;
	BMessenger	*messenger		= NULL;
	while (shortcutList->FindInt32("key",i,(int32 *)&key) == B_OK)
	{
//		handler 	= NULL;
		modifiers	= 0;
		theShortcut	= new shortcut;
		theShortcut->sendMessage = new BMessage();
		theShortcut->key=key;
		shortcutList->FindInt32("modifiers",i,(int32 *)&modifiers);
		shortcutList->FindMessage("message",i,(theShortcut->sendMessage));
		shortcutList->FindPointer("handler",i,(void **)&messenger);
		theShortcut->sentTo		= messenger;
		theShortcut->modifiers	= modifiers;
		i++;
		shortcutMap[key]=theShortcut;
	}
}

void ShortCutFilter::RemoveShortCutList(BMessage *shortcutList)
{
	uint32		key;
	int32 i	= 0;
	while (shortcutList->FindInt32("key",i,(int32 *)&key) == B_OK)
	{
		shortcutMap.erase(key);
		i++;
	}
}
