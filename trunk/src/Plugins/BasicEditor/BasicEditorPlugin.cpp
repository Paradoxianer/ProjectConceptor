#include "BasicEditorPlugin.h"
/*
 * @author Paradoxon powered by Jesus Christ
 */
extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	BasicEditorPlugin *graphEditorPlugin = new BasicEditorPlugin(id);
  	return graphEditorPlugin;
}


BasicEditorPlugin::BasicEditorPlugin(image_id id):BasePlugin(id)
{
}
