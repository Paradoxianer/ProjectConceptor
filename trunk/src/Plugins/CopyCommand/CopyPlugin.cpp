#include "CopyPlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	CopyPlugin *basicCommand=new CopyPlugin( id );
  	return basicCommand;
}

CopyPlugin::CopyPlugin(image_id id):BasePlugin(id)
{
}
