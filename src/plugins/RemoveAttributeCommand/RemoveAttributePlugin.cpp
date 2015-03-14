#include "RemoveAttributePlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	RemoveAttributePlugin *basicCommand=new RemoveAttributePlugin( id );
  	return basicCommand;
}

RemoveAttributePlugin::RemoveAttributePlugin(image_id id):BasePlugin(id)
{
}
