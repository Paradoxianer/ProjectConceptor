#include "FindPlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	FindPlugin *basicCommand=new FindPlugin( id );
  	return basicCommand;
}

FindPlugin::FindPlugin(image_id id):BasePlugin(id)
{
}
