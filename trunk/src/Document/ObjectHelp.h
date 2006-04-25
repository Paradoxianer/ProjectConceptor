#ifndef OBJECT_HELP_H
#define OBJECT_HELP_H

#include <storage/Entry.h>
/**
 * @class ObjectHelp
 *
 * @brief  ObjectHelp is the interface for every object wicht provide a programmewide help
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2005/10/04
 *
 */

class ObjectHelp
{
public:
	char*		GetToolTip(void){return NULL;};
	char*		GetShortDescription(void){return NULL;};
	entry_ref*	GetHelpFile(void){return NULL;};
};
#endif
