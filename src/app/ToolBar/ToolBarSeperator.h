#ifndef TOOLBAR_SEPERATOR_H
#define TOOLBAR_SEPERATOR_H

#include "BaseItem.h"

#include <app/Message.h>
#include <interface/InterfaceDefs.h>
#include <interface/Point.h>
#include <interface/Menu.h>
#include <interface/Rect.h>
#include <support/Archivable.h>

/*
 * @author Paradoxon powered by Jesus Christ
 */
class ToolBar;

class ToolBarSeperator : public BaseItem,BArchivable
{
public:
							ToolBarSeperator(menu_layout ori);

							ToolBarSeperator(BMessage *archive);

							~ToolBarSeperator(void);
	virtual	status_t		Archive(BMessage *archive,bool deep = true) const;
	static	BArchivable*	Instantiate(BMessage *archive);
			void			Draw(BRect updateRect);
			void			SetLayout(menu_layout justification);
			menu_layout		GetLayout(void){return seperatormenu_layout;};
	virtual	BRect			Frame(void){return frame;};
	virtual void			MoveTo(float x,float y){frame.OffsetTo(x,y);};
	virtual void			ResizeTo(float width,float height){frame.SetRightBottom(BPoint(frame.left+width,frame.top+height));};

			
protected:
			//**later maby we coud change the look
			BRect			frame;
			uint32			kind;
			menu_layout		seperatormenu_layout;
private:
};
#endif
