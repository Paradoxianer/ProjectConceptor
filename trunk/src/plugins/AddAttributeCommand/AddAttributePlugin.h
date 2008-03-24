#ifndef ADD_ATTRIBUTE_PLUGIN_H
#define ADD_ATTRIBUTE_PLUGIN_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "BasePlugin.h"
#include "AddAttribute.h"

class AddAttributePlugin : public BasePlugin
{
public:

						AddAttributePlugin(image_id id);
	
	//++++++++++++++++BasePlugin
	virtual uint32			GetType(){return P_C_COMMANDO_PLUGIN_TYPE;};
	virtual	char*			GetVersionsString(void){return "0.01preAlpha";};
	virtual char*			GetAutor(void){return "Paradoxon";};
	virtual char*			GetName(void){return "AddAttribute";};
	virtual char*			GetDescription(void){return "This is a simple AddAttribute Command for ProjectConceptor with basic funktionality";};
	virtual void*			GetNewObject(void *value){return new AddAttribute();};
};
#endif
