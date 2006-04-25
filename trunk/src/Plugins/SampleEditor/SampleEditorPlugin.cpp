#include "SampleEditorPlugin.h"

extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	SampleEditorPlugin *graphEditorPlugin = new SampleEditorPlugin(id);
  	return graphEditorPlugin;
}


SampleEditorPlugin::SampleEditorPlugin(image_id id):BasePlugin(id)
{
}
