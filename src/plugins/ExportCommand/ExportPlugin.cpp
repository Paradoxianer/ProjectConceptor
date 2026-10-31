#include "ExportPlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	ExportPlugin *basicCommand=new ExportPlugin( id );
  	return basicCommand;
}

ExportPlugin::ExportPlugin(image_id id):BasePlugin(id)
{
}
