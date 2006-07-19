#ifndef TRANSLATOR_EXPORTER_PLUGIN_H
#define TRANSLATOR_EXPORTER_PLUGIN_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "BasePlugin.h"
#include "ImageExporter.h"

class ImageExporterPlugin : public BasePlugin
{
public:

						ImageExporterPlugin(image_id id);
	
	//++++++++++++++++BasePlugin
	virtual uint32			GetType(){return P_C_ITEM_INPORT_EXPORT_TYPE;};
	virtual	char*			GetVersionsString(void){return "0.01preAlpha";};
	virtual char*			GetAutor(void){return "Paradoxon";};
	virtual char*			GetName(void){return "ImageExporterPlugin";};
	virtual char*			GetDescription(void){return "This is a ImageExporterPlugin for ProjectConceptor wich can create Image of the current active Editor.";};
	virtual void*			GetNewObject(void *value){return new ImageExporter();};
};
#endif
