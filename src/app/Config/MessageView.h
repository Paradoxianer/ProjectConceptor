#ifndef MESSAGE_VIEW_H
#define MESSAGE_VIEW_H

#include <app/Message.h>
#include <interface/Box.h>
#include <interface/OutlineListView.h>
#include <interface/Rect.h>


/**
 * @class MessageView
 * @brief Generates a MessageView for a passed BMessage and displays a View to set the properties of this message
 *
 * @author Paradoxon powered by Jesus Christ
 * @todo implemt the hole class
 * @bug dont work :-)
 */
 
const float		MARGIN_SPACE		= 10.0;

class MessageView : public BBox
{

public:
							MessageView (BRect rect,BMessage *forMessage,uint32 resizingMode = B_FOLLOW_ALL_SIDES, uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP);
//							~MessageView();

	virtual	void			ChangeLanguage(void);
	virtual	void			SetConfigMessage(BMessage *configureMessage);
	virtual	BMessage*		GetConfigMessage(void){return configMessage;};
	virtual void 			ValueChanged(void);

protected:

private:
			void			Init();
			float			ItemTop();
	BMessage				*configMessage;
	float					seperator;
	
};
#endif
