#ifndef BASIC_RENDERER_H
#define BASIC_RENDERER_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "BasePlugin.h"
#include "ProjectConceptorDefs.h"
#include "BasicEditor.h"

class BasicRenderer : public BasePlugin
{

public:
							BasicRenderer(image_id id);
//							BasicEditor(BMessage* archive);

/*	virtual	status_t		Archive(BMessage *archive, bool deep = true) const;
	static	BArchivable*	Instantiate(BMessage *archive);*/

	//++++++++++++++++BasePlugin
	virtual uint32			GetType(){return B_E_RENDERER;};
	virtual	char*			GetVersionsString(void){return "0.01preAlpha";};
	virtual char*			GetAutor(void){return "Paradoxon";};
	virtual char*			GetName(void){return "BasicRenderer";};
	virtual char*			GetDescription(void){return "This is a simple OBJECT TYPE Renderr for BasicEditor";};
	virtual void*			GetNewObject(void *value);

protected:

private:
};
#endif
