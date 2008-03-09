#ifndef CONFIG_VIEW_H
#define CONFIG_VIEW_H

#include <app/Message.h>
#include <interface/Box.h>
#include <interface/OutlineListView.h>
#include <interface/Rect.h>

#include "MessageView.h"


/**
 * @class ConfigView
 * @brief Generates a ConfigView for a passed BMessage and return a configured BMessage
 *
 * @author Paradoxon powered by Jesus Christ
 * @todo implemt the hole class
 * @bug dont work :-)
 */

class ConfigView : public BBox
{

public:
							ConfigView (BRect rect,BMessage *forMessage, uint32 resizingMode = B_FOLLOW_ALL_SIDES, uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP);
//							~ConfigView();

	virtual	void			ChangeLanguage(void);
	virtual	void			SetConfigMessage(BMessage *configureMessage);
	virtual	BMessage*		GetConfigMessage(void){return configMessage;};
	virtual void 			ValueChanged(void);

protected:
	virtual	void			BuildConfigList(BMessage *confMessage, BListItem *parent);

private:
			void			Init();
	BMessage				*configMessage;
	float					seperator;
	BOutlineListView		*configList;
	MessageView				*showMessage;
	
};
#endif
