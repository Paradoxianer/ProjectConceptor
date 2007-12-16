#include "GraphEditorPlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	GraphEditorPlugin *graphEditorPlugin = new GraphEditorPlugin(id);
  	return graphEditorPlugin;
}


GraphEditorPlugin::GraphEditorPlugin(image_id id):BasePlugin(id)
{
}
