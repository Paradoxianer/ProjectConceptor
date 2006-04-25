#ifndef FIND_PLUGIN_H
#define FIND_PLUGIN_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "BasePlugin.h"
#include "Find.h"

class FindPlugin : public BasePlugin
{
public:

						FindPlugin(image_id id);
	
	//++++++++++++++++BasePlugin
	virtual uint32			GetType(){return P_C_COMMANDO_PLUGIN_TYPE;};
	virtual	char*			GetVersionsString(void){return "0.01preAlpha";};
	virtual char*			GetAutor(void){return "Paradoxon";};
	virtual char*			GetName(void){return "Find";};
	virtual char*			GetDescription(void){return "This is a simple Find Command for ProjectConceptor with basic funktionality";};
	virtual void*			GetNewObject(void *value){return new Find();};
};
#endif
