#ifndef GET_VALUE_PLUGIN_H
#define GET_VALUE_PLUGIN_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "BasePlugin.h"
#include "GetValue.h"

class GetValuePlugin : public BasePlugin
{
public:

						GetValuePlugin(image_id id);

	//++++++++++++++++BasePlugin
	virtual uint32			GetType(){return P_C_COMMANDO_PLUGIN_TYPE;};
	virtual	char*			GetVersionsString(void){return "0.01preAlpha";};
	virtual char*			GetAutor(void){return "Paradoxon";};
	virtual char*			GetName(void){return "GetValue";};
	virtual char*			GetDescription(void){return "Reads a node attribute into a macro variable";};
	virtual void*			GetNewObject(void *value){return new GetValue();};
};
#endif
