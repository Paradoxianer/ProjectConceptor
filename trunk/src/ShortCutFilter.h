#ifndef SHORTCUT_FILTER_H
#define SHORTCUT_FILTER_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <app/Message.h>
#include <app/MessageFilter.h>
#include <app/Messenger.h>

#include <support/KeyedVector.h>
struct shortcut
{
			
	uint32		key;
	uint32		modifiers;
	BMessage	*sendMessage;
	BHandler	*sentTo;
};

class ShortCutFilter : public BMessageFilter
{

public:
							ShortCutFilter(BMessage* shortcutList);
	virtual void			AddShortCutList(BMessage *shortcutList);
	virtual	void			SetShortCutList(BMessage *shortcutList);
	virtual void			RemoveShortCutList(BMessage *shortcutList);
	virtual	BMessage*		GetShortcutList(void);
	virtual	filter_result	Filter(BMessage *message, BHandler **target);
protected:
	BKeyedVector<uint32, shortcut*>	*shortcutVector;
private:
};
#endif
