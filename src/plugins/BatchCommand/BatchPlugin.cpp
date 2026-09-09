#include "BatchPlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	BatchPlugin *basicCommand=new BatchPlugin( id );
	return basicCommand;
}

BatchPlugin::BatchPlugin(image_id id):BasePlugin(id)
{
}
