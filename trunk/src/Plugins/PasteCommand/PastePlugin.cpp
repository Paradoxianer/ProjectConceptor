#include "PastePlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	PastePlugin *basicCommand=new PastePlugin( id );
  	return basicCommand;
}

PastePlugin::PastePlugin(image_id id):BasePlugin(id)
{
}
