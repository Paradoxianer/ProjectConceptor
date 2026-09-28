#ifndef CALCULATE_PLUGIN_H
#define CALCULATE_PLUGIN_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "BasePlugin.h"
#include "Calculate.h"

class CalculatePlugin : public BasePlugin
{
public:

						CalculatePlugin(image_id id);

	//++++++++++++++++BasePlugin
	virtual uint32			GetType(){return P_C_COMMANDO_PLUGIN_TYPE;};
	virtual	char*			GetVersionsString(void){return "0.01preAlpha";};
	virtual char*			GetAutor(void){return "Paradoxon";};
	virtual char*			GetName(void){return "Calculate";};
	virtual char*			GetDescription(void){return "Arithmetic on macro variables (#135 follow-up)";};
	virtual void*			GetNewObject(void *value){return new Calculate();};
};
#endif
