#ifndef CONFIG_WINDOW_H
#define CONFIG_WINDOW_H

#include "ProjectConceptorDefs.h"
#include "ConfigView.h"

#include <app/Message.h>
#include <interface/Window.h>
#include <interface/OutlineListView.h>

#ifdef B_ZETA_VERSION_1_0_0
	#include <locale/Locale.h>
	#include <locale/LanguageNotifier.h>
#else
	#define _T(a) a
#endif

/**
 * @class ConfigWindow
 * @brief Generates a ConfigWindow fo a passed BMessage and return the "new" configured BMessage
 *
 * @author Paradoxon powered by Jesus Christ
 *
 * the ConfigWindow simply pass the BMessage to a ConfigView
 * @see CionfigView
 *
 * @todo implemt the hole class
 * @todo define the complete interface ;-(
 *
 * @bug dont work :-)
 */

class ConfigWindow : public BWindow
{

public:
						ConfigWindow(BMessage *forMessage);
		void			SetConfigMessage(BMessage *_configMessage);
virtual	void			Quit(void);
protected:
		void			ChangeLanguage(void);

private:
        void            CreateViews();
		BMessage		*configMessage;
		ConfigView		*configView;
};
#endif
