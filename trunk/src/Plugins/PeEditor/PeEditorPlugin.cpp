#include "PeEditorPlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	PeEditorPlugin *peEditorPlugin = new PeEditorPlugin(id);
  	return peEditorPlugin;
}


PeEditorPlugin::PeEditorPlugin(image_id id):BasePlugin(id)
{
}
