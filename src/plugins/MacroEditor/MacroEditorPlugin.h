#ifndef MACRO_EDITOR_PLUGIN_H
#define MACRO_EDITOR_PLUGIN_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "BasePlugin.h"
#include "MacroEditor.h"

class MacroEditorPlugin : public BasePlugin
{
public:
							MacroEditorPlugin(image_id id);

	//++++++++++++++++BasePlugin
	virtual uint32			GetType(){return P_C_EDITOR_PLUGIN_TYPE;};
	virtual	char*			GetVersionsString(void){return "0.01preAlpha";};
	virtual char*			GetAutor(void){return "Paradoxon";};
	virtual char*			GetName(void){return "MacroEditor";};
	virtual char*			GetDescription(void){return "Views and edits recorded macros as a guided text DSL";};
	virtual void*			GetNewObject(void *value){return new MacroEditor();};
	//----------------BasePlugin
};
#endif
