#ifndef LAYOUT_PLUGIN_H
#define LAYOUT_PLUGIN_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "BasePlugin.h"
#include "Layout.h"

class LayoutPlugin : public BasePlugin
{
public:

						LayoutPlugin(image_id id);

	//++++++++++++++++BasePlugin
	virtual uint32			GetType(){return P_C_COMMANDO_PLUGIN_TYPE;};
	virtual	char*			GetVersionsString(void){return "0.01preAlpha";};
	virtual char*			GetAutor(void){return "Paradoxon";};
	virtual char*			GetName(void){return "Layout";};
	virtual char*			GetDescription(void){return "Runs automatic layout on the graph - reuses LayoutEditor's algorithm as a scriptable, macro-recordable command";};
	virtual void*			GetNewObject(void *value){return new Layout();};
};
#endif
