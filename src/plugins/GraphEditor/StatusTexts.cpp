#include "StatusTexts.h"

#include <Catalog.h>
#include <StringFormat.h>

#include <math.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "GraphEditor"


namespace StatusTexts {

static BString Count(const char *pattern, int32 count)
{
	BString			text;
	BStringFormat	format(pattern);
	if ((format.InitCheck() != B_OK) || (format.Format(text, count) != B_OK))
		text.SetToFormat("%d", (int)count);
	return text;
}


BString Selection(int32 nodes, int32 connections)
{
	if ((nodes == 0) && (connections == 0))
		return BString(B_TRANSLATE("No selection"));
	BString	text;
	if (nodes > 0)
		text	= Count(B_TRANSLATE("{0, plural, one{# node} other{# nodes}}"), nodes);
	if (connections > 0) {
		if (text.Length() > 0)
			text	<< ", ";
		text	<< Count(B_TRANSLATE("{0, plural, one{# connection} other{# connections}}"),
			connections);
	}
	return text;
}


BString Geometry(BRect frame)
{
	BString	text;
	text.SetToFormat(B_TRANSLATE("Position %d, %d · Size %d × %d"),
		(int)floorf(frame.left + 0.5f), (int)floorf(frame.top + 0.5f),
		(int)floorf(frame.Width() + 0.5f), (int)floorf(frame.Height() + 0.5f));
	return text;
}


BString Pointer(BPoint where)
{
	BString	text;
	text.SetToFormat(B_TRANSLATE("Pointer %d, %d"),
		(int)floorf(where.x + 0.5f), (int)floorf(where.y + 0.5f));
	return text;
}

}
