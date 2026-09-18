#include "RepeatPlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	RepeatPlugin *basicCommand=new RepeatPlugin( id );
  	return basicCommand;
}

RepeatPlugin::RepeatPlugin(image_id id):BasePlugin(id)
{
}
