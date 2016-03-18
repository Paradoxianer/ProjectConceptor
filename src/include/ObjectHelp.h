#ifndef OBJECT_HELP_H
#define OBJECT_HELP_H

#include <storage/Entry.h>
/**
 * @class ObjectHelp
 *
 * @brief  ObjectHelp is the interface for every object wicht provide a programmewide help
 *
 * @author Paradoxon
 * @version 0.01
 * @date 2005/10/04
 *
 * @todo change every function to virtual and abstract this cause changes to many classes
 */

class ObjectHelp
{
public:

	char*		GetID(void){return NULL;};
	char*		GetToolTip(void){return NULL;};
	char*		GetShortDescription(void){return NULL;};
	entry_ref*	GetHelpFile(void){return NULL;};
};
#endif
