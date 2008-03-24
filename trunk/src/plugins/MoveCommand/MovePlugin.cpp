#include "MovePlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	MovePlugin *basicCommand=new MovePlugin( id );
  	return basicCommand;
}

MovePlugin::MovePlugin(image_id id):BasePlugin(id)
{
}
