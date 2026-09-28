#include "CalculatePlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	CalculatePlugin *basicCommand=new CalculatePlugin( id );
  	return basicCommand;
}

CalculatePlugin::CalculatePlugin(image_id id):BasePlugin(id)
{
}
