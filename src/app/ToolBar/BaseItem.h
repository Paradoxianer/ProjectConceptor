#ifndef BASE_ITEM_H
#define BASE_ITEM_H

#include <app/Message.h>
#include <interface/Rect.h>
#include <interface/Button.h>
//#include <support/Archivable.h>
/*
 * @author Paradoxon powered by Jesus Christ
 */
const float ITEM_WIDTH		= 26.0;
const float ITEM_HEIGHT		= 26.0;
const float	TOOL_BAR_SEPERATOR_WIDTH = 6.0;


enum 
{ 
        P_M_ONE_STATE_ITEM, 
        P_M_TWO_STATE_ITEM 
}; 

enum
{
		P_M_ITEM_UP			= 1,
		P_M_ITEM_DOWN		= 2,
		P_M_SHOW_KONTEXT	= 4,
		P_M_SHOW_POPUP		= 8,	
};

class ToolBar;

class BaseItem 
{

public:
							BaseItem(const char *name);
							BaseItem(BMessage *archive);
	virtual	void			AttachedToToolBar(ToolBar *tb);
	virtual	void			DetachedFromToolBar(ToolBar *tb);

	virtual	status_t		Archive(BMessage *archive,bool deep=true) const;
	static	BArchivable*	Instantiate(BMessage *archive);
	
	
	virtual	BRect			Frame(void) = 0;
	virtual	void			Draw(BRect updateRect) = 0;
	virtual void			MoveTo(float x,float y) = 0;
	virtual void			ResizeTo(float width,float Height) = 0;

protected:
			ToolBar			*parentToolBar;

};
#endif

