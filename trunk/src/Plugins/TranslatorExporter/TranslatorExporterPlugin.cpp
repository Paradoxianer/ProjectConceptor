#include "TranslatorExporterPlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	TranslatorExporterPlugin *basicCommand=new TranslatorExporterPlugin( id );
  	return basicCommand;
}

TranslatorExporterPlugin::TranslatorExporterPlugin(image_id id):BasePlugin(id)
{
}
