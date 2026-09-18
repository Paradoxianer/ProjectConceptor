#ifndef FOREACH_PLUGIN_H
#define FOREACH_PLUGIN_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "BasePlugin.h"
#include "ForEach.h"

class ForEachPlugin : public BasePlugin
{
public:

						ForEachPlugin(image_id id);

	//++++++++++++++++BasePlugin
	virtual uint32			GetType(){return P_C_COMMANDO_PLUGIN_TYPE;};
	virtual	char*			GetVersionsString(void){return "0.01preAlpha";};
	virtual char*			GetAutor(void){return "Paradoxon";};
	virtual char*			GetName(void){return "ForEach";};
	virtual char*			GetDescription(void){return "Runs its subPCommand children once per selected node (#135)";};
	virtual void*			GetNewObject(void *value){return new ForEach();};
};
#endif
