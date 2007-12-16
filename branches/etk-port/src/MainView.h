#ifndef MAIN_VIEW_H
#define MAIN_VIEW_H

#include <app/Clipboard.h>
#include <app/Message.h>
#include <interface/Button.h>
#include <interface/TabView.h>
#include <support/List.h>

class PDocument;
/*
 * @author Paradoxon powered by Jesus Christ
 */
class MainView : public BTabView
{

public:
						MainView(PDocument *newDoc,BRect rect,const char *name,  button_width width = B_WIDTH_AS_USUAL, uint32 resizingMode = B_FOLLOW_ALL, uint32 flags = B_FULL_UPDATE_ON_RESIZE | B_WILL_DRAW | B_NAVIGABLE_JUMP | B_FRAME_EVENTS | B_NAVIGABLE);
	virtual				~MainView();
	virtual void		AttachedToWindow(void); 
	virtual void		Select(int32 tab); 
	virtual void		Select(const BTab *tab);
	virtual void		LanguageChanged(void){};
protected:
			BButton		*closeButton;
			PDocument	*doc;
};
#endif
