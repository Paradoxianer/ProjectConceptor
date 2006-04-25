#ifndef TOOL_ITEM_H
#define TOOL_ITEM_H

#include <app/Invoker.h>
#include <app/Message.h>
#include <interface/Bitmap.h>
#include <interface/Button.h>

#include <support/Archivable.h>
#include <support/String.h>

#include "BaseItem.h"

const rgb_color shadowGray		= {130, 130, 130, 255 };
const rgb_color itemGray		= {230, 230, 230, 255 };
const rgb_color itemDarkGray	= {180, 180, 180, 255 };
const rgb_color triangleBlue	= {51, 151, 230, 255 };
const rgb_color black			= {0, 0, 0, 255 };


class ToolBar;

/**
 * @class ToolItem
 *
 * @brief  ToolItem add a BButton with icon to a ToolBar 
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2005/10/04
 * @Contact: mail@rundumvideo.de
 *
 * @see ToolBar
 * @see ToolMenu
 */
class ToolItem: public	BaseItem, 
				public	BButton
{

public:
							ToolItem(const char *name, BBitmap *bmp, BMessage *msg, uint32 behave = P_M_ONE_STATE_ITEM);
							ToolItem(BMessage *msg);
							~ToolItem(void);
	virtual	void			AttachedToToolBar(ToolBar *tb);
	virtual	void			DetachedFromToolBar(ToolBar *tb);

	virtual	status_t		Archive(BMessage *archive,bool deep=true) const;
	static	BArchivable*	Instantiate(BMessage *archive);


virtual		BString			*GetDescription(void){return description;};
virtual		void			SetDescription(BString *descript){description=descript;};
virtual		BString			*GetToolTip(void){return toolTip;};
virtual		void			SetToolTip(BString *toolT){toolTip=toolT;};
virtual	const	char		*GetName(void){return tName;};
			BBitmap			*GetBitmap(void){return toolItemBitmap;};
virtual		void			SetState(uint32 newState){state=newState;};
virtual		uint32			GetState(void){return state;};
virtual		void			SetBehavior(uint32 newBehavior){behavior=newBehavior;};
virtual		uint32			GetBehavior(void){return behavior;};
virtual		void			MouseDown(BPoint point);
virtual		void			MouseUp(BPoint point);
virtual		void			Draw(BRect updateRect);

virtual		BRect			Frame(void) {return BButton::Frame();};
virtual		void			MoveTo(float x,float y){BButton::MoveTo(x,y);};
virtual		void			ResizeTo(float width,float height){ResizeTo(width,height);};

protected:
			void			Init();
	const	char			*tName;
			BBitmap			*toolItemBitmap;
			BString			*description;
			BString			*toolTip;
			uint32			behavior;
			uint32			state;
			float			shadow_offset_by;

};
#endif
