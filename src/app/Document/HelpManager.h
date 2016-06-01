#ifndef HELP_MANAGER_H
#define HELP_MANAGER_H
#include <interface/ListView.h>
#include <interface/OutlineListView.h>
#include <interface/Window.h>
#include <interface/TextControl.h>

/**
 * @class HelpManager
 *
 * @brief  The Helpmanger is the place where all helpstuff runnig together its the main Helppoint
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2005/10/04
 *
 *
 *@todo Helpmanager complete impelementation 
 *
 * @see ObjectHelp
 */
#include "ProjectConceptorDefs.h"
#include "ObjectHelp.h"

//using the ugly stl instead of the nice Zeta templates to make it Haiku ready
#include <map>
#include <iterator>
using namespace std;


typedef pair<char*,ObjectHelp*> *Id_Help;

class HelpManager : public BWindow
{
public:
								HelpManager(void);
virtual void					RegisterHelpObject(ObjectHelp *object);
virtual	void					MessageReceived(BMessage *message);
protected:
		void					Init(void);
		void					Search(void);
private:
		BListView				*index;
		BListView				*searchresult;
		BOutlineListView		*grouped;
		BTextControl			*inputString;
		map<char*,ObjectHelp*>	helpObjects;
		
};
#endif
