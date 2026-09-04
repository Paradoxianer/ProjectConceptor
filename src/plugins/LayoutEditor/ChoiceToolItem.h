#ifndef CHOICE_TOOL_ITEM_H
#define CHOICE_TOOL_ITEM_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "ToolItem.h"

#include <support/List.h>

/**
 * @class ChoiceToolItem
 *
 * @brief ToolItem that opens a BPopUpMenu of labeled choices on click
 * instead of firing its own message directly - MouseDown() is fully
 * overridden (no BButton::MouseDown() call), so this bypasses ToolItem's
 * normal one-click-fires-Invoke() behavior entirely. Picking a choice
 * sends the item's own base message (from the ToolItem constructor) with
 * a "value" string appended, to its own target - same target/messenger
 * setup as any other ToolItem, just triggered from a popup instead of a
 * direct click. Also updates its own tooltip to the picked label, since
 * ToolItem's Draw() has no room to show the current selection otherwise.
 */
class ChoiceToolItem : public ToolItem
{
public:
							ChoiceToolItem(const char *name, BBitmap *bmp,
								BMessage *baseMessage);
	virtual					~ChoiceToolItem(void);

			void			AddChoice(const char *label, const char *value);

	virtual	void			MouseDown(BPoint point);

protected:
			BList			labels;		// BString*
			BList			values;		// BString*
};
#endif
