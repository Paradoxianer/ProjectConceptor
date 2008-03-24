#ifndef INSERT_PLUGIN_H
#define INSERT_PLUGIN_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "BasePlugin.h"
#include "Copy.h"

class CopyPlugin : public BasePlugin
{
public:

						CopyPlugin(image_id id);
	
	//++++++++++++++++BasePlugin
	virtual uint32			GetType(){return P_C_COMMANDO_PLUGIN_TYPE;};
	virtual	char*			GetVersionsString(void){return "0.01preAlpha";};
	virtual char*			GetAutor(void){return "Paradoxon";};
	virtual char*			GetName(void){return "Copy";};
	virtual char*			GetDescription(void){return "This is a simple Copy Command for ProjectConceptor with basic funktionality";};
	virtual void*			GetNewObject(void *value){return new Copy();};
};
#endif
