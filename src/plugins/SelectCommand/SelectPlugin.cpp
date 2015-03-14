#include "SelectPlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	SelectPlugin *basicCommand=new SelectPlugin( id );
  	return basicCommand;
}

SelectPlugin::SelectPlugin(image_id id):BasePlugin(id)
{
}
