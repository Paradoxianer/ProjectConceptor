#ifndef COLOR_TOOL_ITEM_H
#define COLOR_TOOL_ITEM_H

#include "BaseItem.h"

#include <app/Invoker.h>
#include <app/Message.h>
#include <interface/Bitmap.h>
#include <interface/Button.h>
#include <interface/Window.h>

#include <support/Archivable.h>
#include <support/String.h>

class ToolBar;
class ColorSwatchView;
class ColorPickerWindow;

// Number of algorithmically-generated palette swatches shown next to the
// current-color swatch - a hue sweep, not a hand-curated list, so this
// widget doesn't need per-project color curation to be reusable.
const int32 CTI_PALETTE_SIZE = 8;

/**
 * @class ColorToolItem
 *
 * @brief A row of quick-pick palette swatches plus a current-color swatch
 * that opens a full BColorControl+alpha picker on click - adds this to a
 * ToolBar.
 *
 * Replaces the previous single-swatch-button-with-popup design, whose
 * popup closed again on the same click that was supposed to open it
 * (MouseUp hid it whenever the release point was still over the button -
 * i.e. any ordinary click). Picking a palette swatch applies that color
 * immediately; picking one from the full picker does the same. Either
 * way, the exact same external contract this item always had is
 * preserved: GetColor() returns the current value, and picking a color
 * explicitly invokes this item's own message/target (see
 * GraphEditor::MessageReceived's G_E_COLOR_CHANGED case, which just
 * polls GetColor() on receiving it - unchanged by this redesign).
 *
 * @see ToolBar
 * @see ColorSwatchView
 * @see ColorPickerWindow
 */
class ColorToolItem: public	BaseItem,
				public	BButton
{

public:
							ColorToolItem(const char *name, rgb_color newValue, BMessage *msg);
							ColorToolItem(BMessage *msg);
							~ColorToolItem(void);
	virtual	void			AttachedToToolBar(ToolBar *tb);
	virtual	void			DetachedFromToolBar(ToolBar *tb);

	virtual	status_t		Archive(BMessage *archive,bool deep=true) const;
	static	BArchivable*	Instantiate(BMessage *archive);


virtual		BString			*GetDescription(void){return description;};
virtual		void			SetDescription(BString *descript){description=descript;};
virtual		BString			*GetToolTip(void){return toolTip;};
virtual		void			SetToolTip(BString *toolT){toolTip=toolT;};
virtual		const char		*GetName(void){return tName;};
			rgb_color		GetColor(void){return value;};
virtual		void			SetState(uint32 newState){state=newState;};
virtual		uint32			GetState(void){return state;};
virtual		void			SetBehavior(uint32 newBehavior){behavior=newBehavior;};
virtual		uint32			GetBehavior(void){return behavior;};
virtual		void			MouseDown(BPoint point);
virtual		void			MouseUp(BPoint point);
virtual		void			Draw(BRect updateRect);

virtual		BRect			Frame(void) {return BButton::Frame();};
virtual		void			MoveTo(float x,float y){BButton::MoveTo(x,y);};
virtual		void			ResizeTo(float width,float height){BButton::ResizeTo(width,height);};
virtual 	void			MessageReceived(BMessage *message);

protected:
			void			Init();
			void			BuildChildren(rgb_color initialColor);
			void			SetColor(rgb_color newColor);

	const	char			*tName;
			rgb_color		value;
			BString			*description;
			BString			*toolTip;
			uint32			behavior;
			uint32			state;

			ColorSwatchView	*currentColorSwatch;
			ColorSwatchView	*paletteSwatch[CTI_PALETTE_SIZE];
			ColorPickerWindow	*pickerWindow;
};
#endif
