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
class ColorPickerWindow;

/**
 * @class ColorToolItem
 *
 * @brief A compact split button: the main area shows/reapplies the
 * current fill color, a narrow arrow strip on the right opens a full
 * picker (palette + BColorControl + alpha) for anything else.
 *
 * Replaces a previous, wider design (an always-visible row of palette
 * swatches next to a current-color swatch) that turned out too wide and
 * visually busy for a horizontal toolbar row - that layout works for
 * Icon-O-Matic's own SwatchGroup because it lives in a roomy sidebar,
 * not a toolbar. The palette now lives inside ColorPickerWindow instead,
 * only visible while actually picking a color.
 *
 * Clicking the main area needs no custom handling at all - it's a plain
 * BButton, and its native click-Invoke() already sends this item's own
 * message (e.g. G_E_COLOR_CHANGED) to its target, exactly what
 * GraphEditor::MessageReceived already expects (it just polls
 * GetColor() fresh on receiving it - unaffected by this redesign).
 * Clicking the arrow opens ColorPickerWindow; picking a color there also
 * invokes the same message.
 *
 * @see ToolBar
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

			ColorPickerWindow	*pickerWindow;
};
#endif
