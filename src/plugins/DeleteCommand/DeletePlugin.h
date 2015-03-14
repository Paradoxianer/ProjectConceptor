#ifndef DELETE_PLUGIN_H
#define DELETE_PLUGIN_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "BasePlugin.h"
#include "Delete.h"

class DeletePlugin : public BasePlugin
{
public:

						DeletePlugin(image_id id);
	
	//++++++++++++++++BasePlugin
	virtual uint32			GetType(){return P_C_COMMANDO_PLUGIN_TYPE;};
	virtual	char*			GetVersionsString(void){return "0.01preAlpha";};
	virtual char*			GetAutor(void){return "Paradoxon";};
	virtual char*			GetName(void){return "Delete";};
	virtual char*			GetDescription(void){return "This is a simple Delete Command for ProjectConceptor with basic funktionality";};
	virtual void*			GetNewObject(void *value){return new Delete();};
};
#endif
