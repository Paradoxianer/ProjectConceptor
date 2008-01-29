#ifndef TOOLBAR_H
#define TOOLBAR_H

#include "ProjectConceptorDefs.h"

//#include <interface/InterfaceDefs.h>
#include <interface/Menu.h>
#include <interface/Rect.h>
#include <interface/Control.h>
#include <support/List.h>

//#include "Mover.h"
//#include "ToolItem.h"
// For gcc4 we need to put this one into the cpp to not
// mess up the stl includes
#if __GNUC__ < 3
#define max(a,b) ((a)>(b))?a:b
#define min(a,b) ((a)<(b))?a:b
#endif


class Mover;
class ToolItem;
class ToolMenu;
class BaseItem;
/**
 * @class ToolBar
 *
 * @brief  ToolBar provide an ToolBar Class
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2005/10/04
 * @Contact: mail@rundumvideo.de
 *
 * @see ToolItem
 * @see ToolMenu
 * @todo Implement a Method RemoveSeperator
 */
class ToolBar : public BControl
{
friend class ToolMenu;
public:
							ToolBar(BRect rect,const char *name,menu_layout ori=B_ITEMS_IN_ROW);
							ToolBar(BMessage *msg);
							~ToolBar();
							
	//inherit from BArchivable
	virtual	status_t		Archive(BMessage *archive,bool deep=true) const;
	static	BArchivable*	Instantiate(BMessage *archive);
	
			void			AddItem(BaseItem *item);
			void			AddSeperator(void);
			/** 
			 *Removes the last inserted Seperator
			 */
			void			RemoveSeperator(void);
			void			RemoveItem(BaseItem *item);
			void			ReorderItems(void);

	virtual	void			Draw(BRect updateRect);
	virtual	void			DrawAfterChildren(BRect updateRect);
			void			SetLayout(menu_layout justification);
			menu_layout		GetLayout(void){return tool_bar_menu_layout;};
	virtual void			MouseDown(BPoint point);
	virtual void			MouseUp(BPoint point);
	virtual void			SetEventReciver(ToolItem *item){eventReciver=item;};
	virtual void			SetModificationMessage(BMessage *message){BInvoker::SetMessage(message);};
			BMessage		*ModificationMessage(){return BInvoker::Message();};
	virtual	ToolMenu*		GetToolMenu(const char *signature);
	virtual ToolItem*		GetToolItem(const char *signature);
	
	virtual void			GetPreferredSize(float *width, float *height){*width=rightIconBorder;*height=bottomIconBorder;};
protected:
			Mover			*vorward_mover;
			Mover			*backward_mover;
			BList			*toolitems;
			menu_layout		tool_bar_menu_layout;
			float			left_margin,right_margin,top_margin,bottom_margin;
			bool			mouseTrace;
			ToolItem		*klickedItem;
			ToolItem		*eventReciver;
			border_style	border;	
			//used in MatrixLayout
			uint32 			countx;
			uint32 			county;		
			float			rightIconBorder;
			float			bottomIconBorder;
			BMessage		*modificationMessage();
};
#endif
