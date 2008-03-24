#ifndef TOOL_MENU_H
#define TOOL_MENU_H

#include "ToolBar.h"
#include "ToolItem.h"
#include "BaseItem.h"
#include "ProjectConceptorDefs.h"

#include <app/Message.h>
#include <interface/Window.h>
#include <interface/Menu.h>

/**
 * @class ToolMenu
 *
 * @brief ToolMenu create a ToolItem wich controls a popupWindow ..containing more ToolItems or ToolMenu´s
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2005/10/04
 * @Contact: mail@rundumvideo.de
 *
 * @see ToolItem
 * @see ToolBar
 */
class ToolMenu: public ToolItem
{

public:
						ToolMenu(const char *name,BBitmap *icon,menu_layout layout = B_ITEMS_IN_MATRIX);
						ToolMenu(BMessage *archive);
						~ToolMenu(void);
						
virtual	status_t		Archive(BMessage *archive,bool deep = true) const;
static	BArchivable*	Instantiate(BMessage *archive);
		bool			AddItem(BaseItem *item);
		void			RemoveItem(BaseItem *item){menuPainter->RemoveItem(item);};
		
virtual	void			Draw(BRect updateRect);
virtual	void			MouseDown(BPoint point);

virtual	void			MouseUp(BPoint point);
		void			SetLabelFromMarked(bool flag) ;
		bool			IsLabelFromMarked(void){return labelFromMarked;};
virtual void            MessageReceived(BMessage *message);
virtual	ToolMenu*		GetToolMenu(const char *signature){return menuPainter->GetToolMenu(signature);};
virtual ToolItem*		GetToolItem(const char *signature){return menuPainter->GetToolItem(signature);};

protected:
		ToolBar			*menuPainter;
		BWindow			*menuWindow;
		ToolItem		*selectedItem;
		menu_layout		popUpLayout;
		//**maximum Widht or height... to layout the Icons... if both defined.. and there are to many Items. AddItem will refuse the item
		float			max_width;
		float			max_height;	
		//aktual width or height...
		float			width;
		float			height;
		bool 			labelFromMarked;
		bool			mousepressed;
};
#endif
