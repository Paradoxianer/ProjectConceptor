#include "DeletePlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	DeletePlugin *basicCommand=new DeletePlugin( id );
  	return basicCommand;
}

DeletePlugin::DeletePlugin(image_id id):BasePlugin(id)
{
}
