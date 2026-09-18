#ifndef ASK_PLUGIN_H
#define ASK_PLUGIN_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "BasePlugin.h"
#include "Ask.h"

class AskPlugin : public BasePlugin
{
public:

						AskPlugin(image_id id);

	//++++++++++++++++BasePlugin
	virtual uint32			GetType(){return P_C_COMMANDO_PLUGIN_TYPE;};
	virtual	char*			GetVersionsString(void){return "0.01preAlpha";};
	virtual char*			GetAutor(void){return "Paradoxon";};
	virtual char*			GetName(void){return "Ask";};
	virtual char*			GetDescription(void){return "Shows a text-input dialog and stores the answer as a macro variable (#135)";};
	virtual void*			GetNewObject(void *value){return new Ask();};
};
#endif
