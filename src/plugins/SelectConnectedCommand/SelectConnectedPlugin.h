#ifndef SELECT_CONNECTED_PLUGIN_H
#define SELECT_CONNECTED_PLUGIN_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "BasePlugin.h"
#include "SelectConnected.h"

class SelectConnectedPlugin : public BasePlugin
{
public:

						SelectConnectedPlugin(image_id id);

	//++++++++++++++++BasePlugin
	virtual uint32			GetType(){return P_C_COMMANDO_PLUGIN_TYPE;};
	virtual	char*			GetVersionsString(void){return "0.01preAlpha";};
	virtual char*			GetAutor(void){return "Paradoxon";};
	virtual char*			GetName(void){return "SelectConnected";};
	virtual char*			GetDescription(void){return "Adds everything reachable over connections to the selection";};
	virtual void*			GetNewObject(void *value){return new SelectConnected();};
};
#endif
