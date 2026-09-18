#ifndef IF_PLUGIN_H
#define IF_PLUGIN_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "BasePlugin.h"
#include "If.h"

class IfPlugin : public BasePlugin
{
public:

						IfPlugin(image_id id);

	//++++++++++++++++BasePlugin
	virtual uint32			GetType(){return P_C_COMMANDO_PLUGIN_TYPE;};
	virtual	char*			GetVersionsString(void){return "0.01preAlpha";};
	virtual char*			GetAutor(void){return "Paradoxon";};
	virtual char*			GetName(void){return "If";};
	virtual char*			GetDescription(void){return "Runs its subPCommand children conditionally (#135)";};
	virtual void*			GetNewObject(void *value){return new If();};
};
#endif
