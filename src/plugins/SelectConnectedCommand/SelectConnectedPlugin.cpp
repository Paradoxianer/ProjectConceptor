#include "SelectConnectedPlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	SelectConnectedPlugin *basicCommand=new SelectConnectedPlugin( id );
  	return basicCommand;
}

SelectConnectedPlugin::SelectConnectedPlugin(image_id id):BasePlugin(id)
{
}
