#ifndef REMEMBER_PLUGIN_H
#define REMEMBER_PLUGIN_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "BasePlugin.h"
#include "Remember.h"

class RememberPlugin : public BasePlugin
{
public:

						RememberPlugin(image_id id);

	//++++++++++++++++BasePlugin
	virtual uint32			GetType(){return P_C_COMMANDO_PLUGIN_TYPE;};
	virtual	char*			GetVersionsString(void){return "0.01preAlpha";};
	virtual char*			GetAutor(void){return "Paradoxon";};
	virtual char*			GetName(void){return "Remember";};
	virtual char*			GetDescription(void){return "Saves the current selection as a macro variable (#135)";};
	virtual void*			GetNewObject(void *value){return new Remember();};
};
#endif
