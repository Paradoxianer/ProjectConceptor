#ifndef PATTERN_TOOL_ITEM_H
#define PATTERN_TOOL_ITEM_H

#include <app/Invoker.h>
#include <app/Message.h>
#include <interface/Bitmap.h>
#include <interface/Button.h>

#include <support/Archivable.h>
#include <support/String.h>

#include "BaseItem.h"

class ToolBar;

/**
 * @class PatternToolItem
 *
 * @brief  PatternToolItem add a BButton with a visualisation of a pattern to a ToolBar 
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2005/10/04
 * @Contact: mail@rundumvideo.de
 *
 * @see ToolBar
 * @see ToolMenu
 */
class PatternToolItem: public	BaseItem, 
				public	BButton
{

public:
							PatternToolItem(const char *name, pattern newValue, BMessage *msg, uint32 behave = P_M_ONE_STATE_ITEM);
							PatternToolItem(BMessage *msg);
							~PatternToolItem(void);
	virtual	void			AttachedToToolBar(ToolBar *tb);
	virtual	void			DetachedFromToolBar(ToolBar *tb);

	virtual	status_t		Archive(BMessage *archive,bool deep=true) const;
	static	BArchivable*	Instantiate(BMessage *archive);


virtual		BString			*GetDescription(void){return description;};
virtual		void			SetDescription(BString *descript){description=descript;};
virtual		BString			*GetToolTip(void){return toolTip;};
virtual		void			SetToolTip(BString *toolT){toolTip=toolT;};
virtual	const	char		*GetName(void){return tName;};

			void			SetPattern(pattern newValue){	value = newValue;Invalidate();};
			pattern			GetPattern(void){return value;};
			
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
			pattern			value;
			BString			*description;
			BString			*toolTip;
			uint32			behavior;
			uint32			state;
			float			shadow_offset_by;

};
#endif
