#ifndef SLEEP_PLUGIN_H
#define SLEEP_PLUGIN_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "BasePlugin.h"
#include "Sleep.h"

class SleepPlugin : public BasePlugin
{
public:

						SleepPlugin(image_id id);

	//++++++++++++++++BasePlugin
	virtual uint32			GetType(){return P_C_COMMANDO_PLUGIN_TYPE;};
	virtual	char*			GetVersionsString(void){return "0.01preAlpha";};
	virtual char*			GetAutor(void){return "Paradoxon";};
	virtual char*			GetName(void){return "Sleep";};
	virtual char*			GetDescription(void){return "Pauses macro playback for a given duration";};
	virtual void*			GetNewObject(void *value){return new Sleep();};
};
#endif
