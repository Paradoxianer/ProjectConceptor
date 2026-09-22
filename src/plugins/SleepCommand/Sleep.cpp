#include <OS.h>

#include "Sleep.h"


Sleep::Sleep():PCommand()
{
}

static const property_info kSleepProperties[] = {
	{ "Sleep", { B_EXECUTE_PROPERTY, 0 }, { B_DIRECT_SPECIFIER, 0 },
		"Pauses macro playback for \"milliseconds\" - releases the document "
		"lock while waiting, so whatever the previous step just changed "
		"actually gets a chance to redraw during the pause instead of only "
		"appearing once the whole macro finishes.", 0, {0},
		{ { { {"milliseconds", B_INT32_TYPE} } } } },
};

const property_info* Sleep::PropertyInfo(int32 *count)
{
	*count	= 1;
	return kSleepProperties;
}

BMessage* Sleep::Do(PDocument *doc, BMessage *settings)
{
	int32	milliseconds	= 0;
	if (settings->FindInt32("milliseconds",&milliseconds) != B_OK)
		milliseconds	= 0;	// no field at all - nothing to wait for, not an error

	if (milliseconds > 0) {
		// see the class comment - unlocked for the actual wait, not held
		doc->Unlock();
		snooze(((bigtime_t)milliseconds)*1000);
		doc->Lock();
	}

	settings	= PCommand::Do(doc,settings);
	return settings;
}

void Sleep::Undo(PDocument *doc,BMessage *undo)
{
	// nothing to do :) - a pause has no document state to reverse
}



void Sleep::AttachedToManager(void)
{
}

void Sleep::DetachedFromManager(void)
{
}
