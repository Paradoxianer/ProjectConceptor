#ifndef INSERT_H
#define INSERT_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "PCommand.h"
#include "PDocument.h"

class Insert : public PCommand
{

public:
							Insert();
	
	//++++++++++++++++PCommand
	virtual	BMessage*		Do(PDocument *doc, BMessage *settings);
	virtual	void			Undo(PDocument *doc,BMessage *undo);
	virtual	char*			Name(void){return "Insert";};
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);

protected:
	//----------------PCommand
};
#endif
