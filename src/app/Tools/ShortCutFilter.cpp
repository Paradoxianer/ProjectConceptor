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
		// Filter() used to insert NULL entries for unhandled keys (see
		// there) - skip rather than dereference, so an old map that still
		// has some can still be read out
		if (theShortcut == NULL)
			continue;
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
			// find(), not operator[] - reading through operator[] inserts a
			// default (NULL) entry for every key that isn't a shortcut, so
			// the map grew with every ordinary keystroke and anything later
			// walking it (GetShortcutList()) hit those NULLs.
			map<uint32, shortcut*>::iterator	found	= shortcutMap.find(key);
			if (found != shortcutMap.end())
				theShortcut	= found->second;
			if ((theShortcut != NULL) && (theShortcut->sentTo != NULL))
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
		// reset per entry: on a miss FindPointer leaves this untouched, so
		// it used to keep the previous entry's messenger - or, on the very
		// first one, whatever was on the stack
		messenger	= NULL;
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
