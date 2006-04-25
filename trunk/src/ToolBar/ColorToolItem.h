#ifndef COLOR_TOOL_ITEM_H
#define COLOR_TOOL_ITEM_H

#include <app/Invoker.h>
#include <app/Message.h>
#include <interface/Bitmap.h>
#include <interface/Button.h>
#include <interface/ColorControl.h>
#include <interface/Window.h>

#include <support/Archivable.h>
#include <support/String.h>

#include "BaseItem.h"
const uint32 COLOR_CHANGED	='ctCC';

class ToolBar;

/**
 * @class ColorToolItem
 *
 * @brief  ColorToolItem add a BButton with a visualisation of a pattern to a ToolBar 
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2005/10/04
 * @Contact: mail@rundumvideo.de
 *
 * @see ToolBar
 * @see ToolMenu
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

virtual 	status_t		Invoke(BMessage *message = NULL);
virtual		BRect			Frame(void) {return BButton::Frame();};
virtual		void			MoveTo(float x,float y){BButton::MoveTo(x,y);};
virtual		void			ResizeTo(float width,float height){ResizeTo(width,height);};
virtual 	void			MessageReceived(BMessage *message);

protected:
			void			Init();
	const	char			*tName;
			rgb_color		value;
			BString			*description;
			BString			*toolTip;
			uint32			behavior;
			uint32			state;
			BWindow			*colorWindow;
			BColorControl	*colorPicker;
};
#endif
