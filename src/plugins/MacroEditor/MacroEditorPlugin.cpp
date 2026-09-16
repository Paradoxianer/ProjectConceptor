#include "MacroEditorPlugin.h"
/*
 * @author Paradoxon powered by Jesus Christ
 */
extern "C" _EXPORT BasePlugin *NewProjektConceptorPlugin(image_id);

BasePlugin* NewProjektConceptorPlugin( image_id id )
{
	MacroEditorPlugin *macroEditorPlugin = new MacroEditorPlugin(id);
  	return macroEditorPlugin;
}


MacroEditorPlugin::MacroEditorPlugin(image_id id):BasePlugin(id)
{
}
