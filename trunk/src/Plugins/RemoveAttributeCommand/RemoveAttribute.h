#ifndef ADD_ATTRIBUTE_H
#define ADD_ATTRIBUTE_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "PCommand.h"
#include "PDocument.h"

class RemoveAttribute : public PCommand
{

public:
							RemoveAttribute();
	
	//++++++++++++++++PCommand
	virtual	BMessage*		Do(PDocument *doc, BMessage *settings);
	virtual	void			Undo(PDocument *doc,BMessage *undo);
	virtual	char*			Name(void){return "RemoveAttribute";};
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);

	//----------------PCommand
};
#endif
