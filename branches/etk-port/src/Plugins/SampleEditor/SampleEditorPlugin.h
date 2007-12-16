#ifndef SAMPLE_EDITOR_PLUGIN_H
#define SAMPLE_EDITOR_PLUGIN_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <app/Message.h>
#include <interface/View.h>
#include <support/List.h>

#include "SampleEditor.h"
#include "BasePlugin.h"


class SampleEditorPlugin : public BasePlugin
{
public:
							SampleEditorPlugin(image_id id);
//							SampleEditor(BMessage* archive);

/*	virtual	status_t		Archive(BMessage *archive, bool deep = true) const;
	static	BArchivable*	Instantiate(BMessage *archive);*/

	//++++++++++++++++BasePlugin
	virtual uint32			GetType(){return P_C_EDITOR_PLUGIN_TYPE;};
	virtual	char*			GetVersionsString(void){return "0.01preAlpha";};
	virtual char*			GetAutor(void){return "Paradoxon";};
	virtual char*			GetName(void){return "SampleEditor";};
	virtual char*			GetDescription(void){return "This is a simple Editor for ProjectConceptor with basic funktionality";};
	virtual void*			GetNewObject(void *value){return new SampleEditor();};
	//----------------BasePlugin
};
#endif
