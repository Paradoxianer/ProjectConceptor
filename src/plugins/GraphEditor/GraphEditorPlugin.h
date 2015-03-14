#ifndef BASIC_EDITOR_PLUGIN_H
#define BASIC_EDITOR_PLUGIN_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <app/Message.h>
#include <interface/View.h>
#include <support/List.h>

#include "GraphEditor.h"
#include "BasePlugin.h"


class GraphEditorPlugin : public BasePlugin
{
public:
							GraphEditorPlugin(image_id id);
//							GraphEditor(BMessage* archive);

/*	virtual	status_t		Archive(BMessage *archive, bool deep = true) const;
	static	BArchivable*	Instantiate(BMessage *archive);*/

	//++++++++++++++++BasePlugin
	virtual uint32			GetType(){return P_C_EDITOR_PLUGIN_TYPE;};
	virtual	char*			GetVersionsString(void){return "0.01preAlpha";};
	virtual char*			GetAutor(void){return "Paradoxon";};
	virtual char*			GetName(void){return "GraphEditor";};
        virtual char*			GetDescription(void){return "GraphEidtor is a  Gaphical interface wich allow you to create, edit nodes. It also supports Attributes for Nodes";};
	virtual void*			GetNewObject(void *value){return new GraphEditor(GetPluginID());};
	//----------------BasePlugin
};
#endif
