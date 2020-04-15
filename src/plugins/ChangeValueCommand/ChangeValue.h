#ifndef CHANGE_VALUE_H
#define CHANGE_VALUE_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "PCommand.h"
#include "PDocument.h"

class ChangeValue : public PCommand
{

public:
							ChangeValue();
	
	//++++++++++++++++PCommand
	virtual	BMessage*		Do(PDocument *doc, BMessage *settings);
	virtual	void			Undo(PDocument *doc,BMessage *undo);
	virtual	char*			Name(void){return "ChangeValue";};
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);

protected:
			void			DoChangeValue(BMessage *node, BMessage *valueContainer, BMessage *undo);
	//----------------PCommand
};
#endif
