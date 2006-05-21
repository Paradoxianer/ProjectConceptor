#ifndef HELP_MANAGER_H
#define HELP_MANAGER_H
#include <interface/ListView.h>
#include <interface/OutlineListView.h>
#include <interface/Window.h>
#include <interface/TextControl.h>
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "ProjectConceptorDefs.h"
#include "ObjectHelp.h"

#ifdef B_ZETA_VERSION_1_0_0
	#include <locale/Locale.h>
	#include <locale/LanguageNotifier.h>
#else
	#define _T(a) a
#endif 

class HelpManager : public BWindow
{
public:
							HelpManager(void);
virtual void				RegisterHelpObject(ObjectHelp *object);
virtual	void				MessageReceived(BMessage *message);
protected:
		void				Init(void);
		void				Search(void);
private:
		BListView			*index;
		BListView			*searchresult;
		BOutlineListView	*grouped;
		BTextControl		*inputString;
};
#endif
