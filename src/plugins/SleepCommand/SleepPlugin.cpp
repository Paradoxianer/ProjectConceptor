#include "SleepPlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	SleepPlugin *basicCommand=new SleepPlugin( id );
  	return basicCommand;
}

SleepPlugin::SleepPlugin(image_id id):BasePlugin(id)
{
}
