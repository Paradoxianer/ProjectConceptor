#ifndef MOVER_H
#define MOVER_H

#include <interface/InterfaceDefs.h>
#include <interface/Point.h>
#include <interface/Menu.h>
#include <interface/Rect.h>
#include <interface/View.h>
#include <support/Archivable.h>
/*
 * @author Paradoxon powered by Jesus Christ
 */
//#include "Toolbar.h"

enum direction
{
	P_M_VORWARD		= -1,
	P_M_BACKWARD	= 1
};

const float	MOVER_WIDTH			=	5.0;

const rgb_color moverGray = {130, 130, 130, 255 };
//const rgb_color black = {0, 0, 0, 255 };


class ToolBar;

class Mover: public BArchivable
{
public:
						Mover();
						Mover(BRect rect,menu_layout ori,direction direct);
						Mover(BMessage *msg);

virtual	status_t		Archive(BMessage *archive,bool deep=true) const;
static	BArchivable*	Instantiate(BMessage *archive);

		BRect			Frame(){return frame;};
		void			SetFrame(BRect rect){frame=rect;};
		float			GetSteps(){return steps;};
		void			SetSteps(float step);
		direction		GetDirection(void){return mover_direction;};
		menu_layout		GetLayout(void){return mover_menu_layout;};
		void			SetDirection(direction direct);
		void			SetLayout(menu_layout justification);
		void			Draw(ToolBar *tb,BRect updateRect);
		void			SetHighlight(bool state){highlighted=state;};
		bool			GetHighlight(void){return highlighted;};
		void			SetEnabled(bool state){enabled=state;};
		bool			GetEndabled(void){return enabled;};
virtual void			MouseMoved(BView *tb,BPoint point);
		void			Scroller();
protected:
		void			Init(BRect rect,menu_layout ori,direction direct);
		bool			highlighted;
		bool			enabled; 
		BRect			frame;
		menu_layout		mover_menu_layout;
		direction		mover_direction;
		float			steps;
		bool			scroll;
private:
};
#endif
