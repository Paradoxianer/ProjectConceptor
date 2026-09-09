#ifndef CHOICE_TOOL_ITEM_H
#define CHOICE_TOOL_ITEM_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "BaseItem.h"

#include <interface/MenuField.h>

class BBitmap;
class BPopUpMenu;

/**
 * @class ChoiceToolItem
 *
 * @brief BMenuField-based ToolBar item for picking one of a small set of
 * labeled choices - always shows the current pick as visible text (radio
 * mode + label-from-marked, both BPopUpMenu defaults), unlike a plain
 * ToolItem button which has no room to show which of several choices is
 * active. Each choice may carry a small icon (IconMenuItem), drawn in the
 * popup only - the field itself just shows the current label, matching
 * BMenuField's own standard behavior.
 *
 * Picking a choice sends the base message from the constructor, with a
 * "value" string appended, to this item's own target - same target setup
 * as any other ToolItem.
 */
class ChoiceToolItem : public BMenuField, public BaseItem
{
public:
	/** width should fit this field's own longest choice label - BMenuField
	 * doesn't grow/shrink itself when the selection changes, and ToolBar
	 * positions every item once at AddItem() time, so a too-narrow fixed
	 * width truncates text rather than reflowing anything later.
	 */
							ChoiceToolItem(const char *name, BMessage *baseMessage,
								float width = ITEM_WIDTH*3);
	virtual					~ChoiceToolItem(void);

	/** icon may be NULL for a text-only entry. Ownership of baseMessage's
	 * clone stays with this item; icon is not copied/owned (matches
	 * ToolItem's own bitmap parameter convention).
	 */
			void			AddChoice(const char *label, const char *value,
								BBitmap *icon = NULL);
	/** Marks the first choice matching value as current, without firing a
	 * message - for setting an initial default after AddChoice() calls.
	 */
			void			SetValue(const char *value);

	/** Shows the marked choice as its icon instead of its label, turning
	 * the field into an icon field - the popup keeps icon *and* label, so
	 * the text is still there while choosing. Only worth setting when
	 * every choice actually carries an icon; one without still falls back
	 * to drawing its label. Set it before adding choices and give the
	 * item an icon-sized width.
	 */
			void			SetIconOnly(bool iconOnly);

	virtual	void			AttachedToToolBar(ToolBar *tb);
	virtual	void			DetachedFromToolBar(ToolBar *tb);

	virtual	status_t		Archive(BMessage *archive,bool deep=true) const;
	static	BArchivable*	Instantiate(BMessage *archive);

	virtual	BRect			Frame(void) {return BMenuField::Frame();};
	virtual	void			Draw(BRect updateRect);
	virtual	void			DrawAfterChildren(BRect updateRect);
	virtual	void			MoveTo(float x,float y) {BMenuField::MoveTo(x,y);};
	virtual	void			ResizeTo(float width,float height)
								{BMenuField::ResizeTo(width,height);};

	/** Forwards to menu->SetTargetForItems() - the individual BMenuItems
	 * are what actually invoke, not the BMenuField itself.
	 */
			status_t		SetTarget(BMessenger messenger);

protected:
			BPopUpMenu		*menu;
			BMessage		*baseMessage;
			bool			iconOnly;
};
#endif
