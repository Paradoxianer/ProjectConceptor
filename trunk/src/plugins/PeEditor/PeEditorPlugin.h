#ifndef PE_EDITOR_PLUGIN_H
#define PE_EDITOR_PLUGIN_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <app/Message.h>
#include <interface/View.h>
#include <support/List.h>

#include "PeEditor.h"
#include "BasePlugin.h"


class GraphEditorPlugin : public BasePlugin
{
public:
							PeEditorPlugin(image_id id);
//							PeEditor(BMessage* archive);

/*	virtual	status_t		Archive(BMessage *archive, bool deep = true) const;
	static	BArchivable*	Instantiate(BMessage *archive);*/

	//++++++++++++++++BasePlugin
	virtual uint32			GetType(){return P_C_EDITOR_PLUGIN_TYPE;};
	virtual	char*			GetVersionsString(void){return "0.01preAlpha";};
	virtual char*			GetAutor(void){return "Paradoxon";};
	virtual char*			GetName(void){return "PeEditor";};
	virtual char*			GetDescription(void){return "The well know Pe Editor as a ProjectConceptor Plugin";};
	virtual void*			GetNewObject(void *value){return new PeEditor(GetPluginID());};
	//----------------BasePlugin
};
#endif
