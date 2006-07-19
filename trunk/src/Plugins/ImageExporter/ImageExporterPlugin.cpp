#include "ImageExporterPlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	ImageExporterPlugin *basicCommand=new ImageExporterPlugin( id );
  	return basicCommand;
}

ImageExporterPlugin::ImageExporterPlugin(image_id id):BasePlugin(id)
{
}
