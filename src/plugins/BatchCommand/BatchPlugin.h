#ifndef BATCH_PLUGIN_H
#define BATCH_PLUGIN_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "BasePlugin.h"
#include "Batch.h"

class BatchPlugin : public BasePlugin
{
public:

						BatchPlugin(image_id id);

	//++++++++++++++++BasePlugin
	virtual uint32			GetType(){return P_C_COMMANDO_PLUGIN_TYPE;};
	virtual	char*			GetVersionsString(void){return "0.01preAlpha";};
	virtual char*			GetAutor(void){return "Paradoxon";};
	virtual char*			GetName(void){return "Batch";};
	virtual char*			GetDescription(void){return "Runs its subPCommands as a single undo step, nothing else";};
	virtual void*			GetNewObject(void *value){return new Batch();};
};
#endif
