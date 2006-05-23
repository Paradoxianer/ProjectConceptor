#ifndef ADD_ATTRIBUTE_H
#define ADD_ATTRIBUTE_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "PCommand.h"
#include "PDocument.h"

class AddAttribute : public PCommand
{

public:
							AddAttribute();
	
	//++++++++++++++++PCommand
	virtual	BMessage*		Do(PDocument *doc, BMessage *settings);
	virtual	void			Undo(PDocument *doc,BMessage *undo);
	virtual	char*			Name(void){return "AddAttribute";};
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);

protected:
			void			DoAddAttribute(PDocument *doc,BRect *rect,BMessage *settings);
	//----------------PCommand
};
#endif
