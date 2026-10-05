#include "GetValuePlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	GetValuePlugin *basicCommand=new GetValuePlugin( id );
  	return basicCommand;
}

GetValuePlugin::GetValuePlugin(image_id id):BasePlugin(id)
{
}
