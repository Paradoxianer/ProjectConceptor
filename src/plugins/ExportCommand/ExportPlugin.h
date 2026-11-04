#ifndef EXPORT_PLUGIN_H
#define EXPORT_PLUGIN_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "BasePlugin.h"
#include "Export.h"

class ExportPlugin : public BasePlugin
{
public:

						ExportPlugin(image_id id);

	//++++++++++++++++BasePlugin
	virtual uint32			GetType(){return P_C_COMMANDO_PLUGIN_TYPE;};
	virtual	char*			GetVersionsString(void){return "0.01preAlpha";};
	virtual char*			GetAutor(void){return "Paradoxon";};
	virtual char*			GetName(void){return "Export";};
	virtual char*			GetDescription(void){return "Writes a copy of the document in another format";};
	virtual void*			GetNewObject(void *value){return new Export();};
};
#endif
