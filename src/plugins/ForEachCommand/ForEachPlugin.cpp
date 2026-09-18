#include "ForEachPlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	ForEachPlugin *basicCommand=new ForEachPlugin( id );
  	return basicCommand;
}

ForEachPlugin::ForEachPlugin(image_id id):BasePlugin(id)
{
}
