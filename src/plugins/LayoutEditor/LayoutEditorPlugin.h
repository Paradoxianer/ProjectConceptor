#ifndef LAYOUT_EDITOR_PLUGIN_H
#define LAYOUT_EDITOR_PLUGIN_H

#include <app/Message.h>
#include <support/List.h>

#include "BasePlugin.h"
#include "LayoutEditor.h"


class LayoutEditorPlugin : public BasePlugin
{
public:
							LayoutEditorPlugin(image_id id);

	//++++++++++++++++BasePlugin
	virtual uint32			GetType(){return P_C_EDITOR_PLUGIN_TYPE;};
	virtual	char*			GetVersionsString(void){return "0.01preAlpha";};
	virtual char*			GetAutor(void){return "Paradoxon";};
	virtual char*			GetName(void){return "LayoutEditor";};
	virtual char*			GetDescription(void){return "Automatic graph layout (issue #54)";};
	virtual void*			GetNewObject(void *value){return new LayoutEditor();};
	//----------------BasePlugin
};
#endif
