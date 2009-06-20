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
 *
 * @see ToolItem
 * @see ToolMenu
 */
class ToolBar : public BControl
{
friend class ToolMenu;
public:
                       /**
                         *Creates a new Toolbar.
                         *@see SetLayout
                         */
							ToolBar(BRect rect,const char *name,menu_layout ori=B_ITEMS_IN_ROW);
							ToolBar(BMessage *msg);
							~ToolBar();
							
	//inherit from BArchivable
	virtual	status_t		Archive(BMessage *archive,bool deep=true) const;
	static	BArchivable*	Instantiate(BMessage *archive);
                        /**
                         *Adds a Item to the Toolbar you can easy design your own Item by deriving your Class from BaseItem.
                         *@see BaseItem
                         */

			void			AddItem(BaseItem *item);
                        /**
                         *Adds a seperator after the last insert Item
                         */

                        void			AddSeperator(void);
			/** 
			 *Removes the last inserted Seperator
			 */
			void			RemoveSeperator(void);
                        /**
                         *Removes the item you passed
                         */

			void			RemoveItem(BaseItem *item);
			void			ReorderItems(void);

	virtual	void			Draw(BRect updateRect);
	virtual	void			DrawAfterChildren(BRect updateRect);
                         /**
                         *SetLayout supports three modes
                         * B_ITEMS_IN_ROW ,B_ITEMS_IN_COLUMN ,B_ITEMS_IN_MATRIX
                         * it tells the Toolbar how to layout the added Items.
                         * so B_ITEMS_IN_ROW will place all items in a row, B_ITEMS_IN_COLUMN will place all items ... of course in a Column
                         * and B_ITEMS_IN_MATRIX will  try to place all imtems like on a check board so that all items fit best.
                         */

			void			SetLayout(menu_layout justification);
			menu_layout		GetLayout(void){return tool_bar_menu_layout;};
	virtual void			MouseDown(BPoint point);
	virtual void			MouseUp(BPoint point);
	virtual void			SetEventReciver(ToolItem *item){eventReciver=item;};
	virtual void			SetModificationMessage(BMessage *message){BInvoker::SetMessage(message);};
			BMessage		*ModificationMessage(){return BInvoker::Message();};
                        /**
                         *Trys to find the ToolMenu by the given signature (the name of the BaseItem)
                         *@see BaseItem
                         *@see ToolMenu
                         */
	virtual	ToolMenu*		GetToolMenu(const char *signature);
                        /**
                         *Try to find the ToolItem by the given signature (the name of the BaseItem)
                         *@see BaseItem
                         *@see ToolItem
                         */
        virtual ToolItem*		GetToolItem(const char *signature);
	
			void			SetButtonBorder(bool _buttonBorder){ fButtonBorder = _buttonBorder;};
			bool			HasButtonBorder(void){return fButtonBorder;};
	
	
	virtual void			GetPreferredSize(float *width, float *height){*width=rightIconBorder;*height=bottomIconBorder;};
protected:
			bool 			fButtonBorder;
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
