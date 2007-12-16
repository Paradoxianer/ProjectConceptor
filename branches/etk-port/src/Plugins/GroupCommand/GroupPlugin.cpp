#include "GroupPlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	GroupPlugin *basicCommand=new GroupPlugin( id );
  	return basicCommand;
}

GroupPlugin::GroupPlugin(image_id id):BasePlugin(id)
{
}
