#include "IfPlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	IfPlugin *basicCommand=new IfPlugin( id );
  	return basicCommand;
}

IfPlugin::IfPlugin(image_id id):BasePlugin(id)
{
}
