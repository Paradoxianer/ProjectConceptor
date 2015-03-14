#ifndef CONFIG_VIEW_H
#define CONFIG_VIEW_H

#include <app/Message.h>
#include <interface/Box.h>
#include <interface/OutlineListView.h>
#include <interface/Rect.h>
#include <GridView.h>
#include <Box.h>



/**
 * @class ConfigView
 * @brief Generates the main ConfigView and passe back the changed ConfigMessage
 *
 * @author Paradoxon powered by Jesus Christ
 * @todo implemt the hole class
 * @bug dont work :-)
 */

const uint32 MESSAGE_SELECTED	= 'MEsl';
const uint32 AUTOSAVE_CHANGED	= 'AScg';
const uint32 AUTOSAVE_TOGGLED	= 'AStg';


class ConfigView : public BGridView
{

public:
                                        ConfigView (BMessage *_configMessage);
//					~ConfigView();

	virtual	void			ChangeLanguage(void);
	virtual	void			SetConfigMessage(BMessage *configureMessage);
	virtual	BMessage*		GetConfigMessage(void){return configMessage;};
	virtual void 			ValueChanged(void);
	virtual void			MessageReceived(BMessage *msg);

protected:
	virtual	void			BuildConfigList(BMessage *confMessage, BListItem *parent);

private:
            void			Init();
        BMessage			*configMessage;
};
#endif
