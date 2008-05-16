#ifndef FLOAT_TOOL_ITEM_H
#define FLOAT_TOOL_ITEM_H

#include "BaseItem.h"

#include <app/Invoker.h>
#include <app/Message.h>
#include <interface/Bitmap.h>
#include <interface/Button.h>
#include <interface/TextControl.h>
#include <interface/Window.h>

#include <support/Archivable.h>
#include <support/String.h>

class ToolBar;

/**
 * @class FloatToolItem
 *
 * @brief  FloatToolItem add a BButton with a visualisation of a FloatValue to a ToolBar
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2005/10/04
 * @Contact: mail@rundumvideo.de
 *
 * @see ToolBar
 * @see ToolMenu
 */
class FloatToolItem: public	BButton, public BaseItem
{

public:
							FloatToolItem(const char *name, float newValue, BMessage *msg);
							FloatToolItem(BMessage *msg);
							~FloatToolItem(void);
	virtual	void			AttachedToToolBar(ToolBar *tb);
	virtual	void			DetachedFromToolBar(ToolBar *tb);

	virtual	status_t		Archive(BMessage *archive,bool deep=true) const;
	static	BArchivable*	Instantiate(BMessage *archive);


virtual		BString			*GetDescription(void){return description;};
virtual		void			SetDescription(BString *descript){description=descript;};
virtual		BString			*GetToolTip(void){return toolTip;};
virtual		void			SetToolTip(BString *toolT){toolTip=toolT;};
virtual	const	char		*GetName(void){return tName;};
virtual		float			GetValue(void);
			void			SetValue(float);

virtual		void			Draw(BRect updateRect);


virtual		BRect			Frame(void) {return BButton::Frame();};
virtual		void			MoveTo(float x,float y){BButton::MoveTo(x,y);};
virtual		void			ResizeTo(float width,float height){ResizeTo(width,height);};
virtual 	void			MessageReceived(BMessage *message);
virtual		rgb_color		TextViewColor(void){return textValue->TextView()->ViewColor(rgb);};
virtual		void			SetTextViewColor(rgb_color rgb){textValue->TextView()->SetViewColor(rgb);};

protected:
			void			Init();
	const	char			*tName;
			float			value;
			BString			*description;
			BString			*toolTip;
			BTextControl	*textValue;
};
#endif
