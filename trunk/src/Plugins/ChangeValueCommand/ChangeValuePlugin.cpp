#include "ChangeValuePlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	ChangeValuePlugin *basicCommand=new ChangeValuePlugin( id );
  	return basicCommand;
}

ChangeValuePlugin::ChangeValuePlugin(image_id id):BasePlugin(id)
{
}
