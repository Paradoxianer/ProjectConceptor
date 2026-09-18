#include "RememberPlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	RememberPlugin *basicCommand=new RememberPlugin( id );
  	return basicCommand;
}

RememberPlugin::RememberPlugin(image_id id):BasePlugin(id)
{
}
