#include "LayoutEditorPlugin.h"
/*
 * @author Paradoxon powered by Jesus Christ
 */
extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	LayoutEditorPlugin *layoutEditorPlugin = new LayoutEditorPlugin(id);
	return layoutEditorPlugin;
}


LayoutEditorPlugin::LayoutEditorPlugin(image_id id):BasePlugin(id)
{
}
