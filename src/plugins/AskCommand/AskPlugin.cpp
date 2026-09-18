#include "AskPlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	AskPlugin *basicCommand=new AskPlugin( id );
  	return basicCommand;
}

AskPlugin::AskPlugin(image_id id):BasePlugin(id)
{
}
