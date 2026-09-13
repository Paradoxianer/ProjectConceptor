#ifndef COLOR_ITEM_H
#define COLOR_ITEM_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <interface/Button.h>
#include <interface/Font.h>
#include <interface/GraphicsDefs.h>
#include <interface/ListItem.h>
#include <interface/Rect.h>
#include <interface/View.h>

#include <stdio.h>

#include "BaseListItem.h"

// A field's own click just opens the picker - the actual, undo-worthy
// value change only happens once the picker reports its final color
// (see ColorItem::ClosePicker(), mirroring how ColorToolItem commits on
// ColorPickerWindow's PW_CLOSED rather than on every live drag update).
const uint32	COLOR_ITEM_OPEN		= 'ciOP';
const uint32	COLOR_ITEM_REPORT	= 'ciRP';

class ColorPickerWindow;

// Displays/edits one of this app's packed-int32 color fields (FillColor,
// BorderColor, HighColor, LowColor, Font::Color, ...) as an actual color
// swatch instead of a meaningless signed integer - MessageListView picks
// this over Int32Item by field name (see NavIsColorField() usage there).
// Storage stays B_INT32_TYPE throughout; only the display/edit side is
// color-aware, matching ClassRenderer's own FindInt32()-based reads.
class ColorItem : public BaseListItem
{
public:
						ColorItem(char *newLabel, rgb_color newValue,
							uint32 level = 0, bool expanded = true);
	virtual				~ColorItem();
	virtual	rgb_color	GetColor(void){return colorValue;};
	virtual	void		SetColor(rgb_color newColor);
	virtual const char	*GetLabel(void){return label;};

	virtual void		ValueChange(void){};

	virtual	void		Select(void);
	virtual	void		Deselect(void);
	virtual void		SetExpanded(bool expande);
	virtual	void		Update(BView *owner, const BFont *font);
	virtual	void		DrawItem(BView *owner, BRect bounds, bool complete = false);
	virtual status_t	Invoke(BMessage *message = NULL);

	// Called by MessageListView, which is what ColorPickerWindow (a
	// generic, ColorItem-agnostic widget) actually targets.
			void		OpenPicker(BHandler *target);
			void		ApplyReport(BMessage *message);
			void		ClosePicker(BMessage *message);

protected:
	BButton				*value;
	rgb_color			colorValue;
	char				*svalue;
	char				*label;
	float				textLine;
	rgb_color			foreground;
	rgb_color			background;
	rgb_color			backgroundHi;
	rgb_color			originalValue;
	ColorPickerWindow	*picker;
private:
};
#endif
