#include "InsertPlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	InsertPlugin *basicCommand=new InsertPlugin( id );
  	return basicCommand;
}

InsertPlugin::InsertPlugin(image_id id):BasePlugin(id)
{
}
