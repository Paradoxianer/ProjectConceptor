#include "AddAttributePlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	AddAttributePlugin *basicCommand=new AddAttributePlugin( id );
  	return basicCommand;
}

AddAttributePlugin::AddAttributePlugin(image_id id):BasePlugin(id)
{
}
