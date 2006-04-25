#ifndef DELETE_H
#define DELETE_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "PCommand.h"
#include "PDocument.h"

class Move : public PCommand
{

public:
							Move();
	
	//++++++++++++++++PCommand
	virtual	BMessage*		Do(PDocument *doc, BMessage *settings);
	virtual	void			Undo(PDocument *doc,BMessage *undo);
	virtual	char*			Name(void){return "Move";};
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);

protected:
	//----------------PCommand
};
#endif
