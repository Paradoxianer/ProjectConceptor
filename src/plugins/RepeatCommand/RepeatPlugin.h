#ifndef REPEAT_PLUGIN_H
#define REPEAT_PLUGIN_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "BasePlugin.h"
#include "Repeat.h"

class RepeatPlugin : public BasePlugin
{
public:

						RepeatPlugin(image_id id);

	//++++++++++++++++BasePlugin
	virtual uint32			GetType(){return P_C_COMMANDO_PLUGIN_TYPE;};
	virtual	char*			GetVersionsString(void){return "0.01preAlpha";};
	virtual char*			GetAutor(void){return "Paradoxon";};
	virtual char*			GetName(void){return "Repeat";};
	virtual char*			GetDescription(void){return "Runs its subPCommand children a fixed number of times (#135)";};
	virtual void*			GetNewObject(void *value){return new Repeat();};
};
#endif
