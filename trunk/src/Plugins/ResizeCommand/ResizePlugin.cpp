#include "ResizePlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	ResizePlugin *basicCommand=new ResizePlugin( id );
  	return basicCommand;
}

ResizePlugin::ResizePlugin(image_id id):BasePlugin(id)
{
}
