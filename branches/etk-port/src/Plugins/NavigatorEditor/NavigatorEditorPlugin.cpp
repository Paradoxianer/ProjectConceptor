#include "NavigatorEditorPlugin.h"
/*
 * @author Paradoxon powered by Jesus Christ
 */
extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	NavigatorEditorPlugin *navigatorEditorPlugin = new NavigatorEditorPlugin(id);
  	return navigatorEditorPlugin;
}


NavigatorEditorPlugin::NavigatorEditorPlugin(image_id id):BasePlugin(id)
{
}
