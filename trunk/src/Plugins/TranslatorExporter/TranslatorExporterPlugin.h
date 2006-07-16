#ifndef TRANSLATOR_EXPORTER_PLUGIN_H
#define TRANSLATOR_EXPORTER_PLUGIN_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "BasePlugin.h"
#include "TranslatorExporter.h"

class TranslatorExporterPlugin : public BasePlugin
{
public:

						TranslatorExporterPlugin(image_id id);
	
	//++++++++++++++++BasePlugin
	virtual uint32			GetType(){return P_C_ITEM_INPORT_EXPORT_TYPE;};
	virtual	char*			GetVersionsString(void){return "0.01preAlpha";};
	virtual char*			GetAutor(void){return "Paradoxon";};
	virtual char*			GetName(void){return "TranslatorExporterPlugin";};
	virtual char*			GetDescription(void){return "This is a TranslatorExporterPlugin for ProjectConceptor wich deals with all Translators wich can read/write ProjectConceptor Data";};
	virtual void*			GetNewObject(void *value){return new TranslatorExporter();};
};
#endif
