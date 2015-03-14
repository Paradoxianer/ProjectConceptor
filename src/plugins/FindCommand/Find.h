#ifndef FIND_H
#define FIND_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "PCommand.h"
#include "PDocument.h"

class Find : public PCommand
{

public:
							Find();
	
	//++++++++++++++++PCommand
	virtual	void			Undo(PDocument *doc,BMessage *undo);
	virtual	BMessage*		Do(PDocument *doc, BMessage *settings);
	virtual	char*			Name(void){return "Find";};
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);

protected:
	virtual	void			DoFind(PDocument *doc ,BRect *rect);
	virtual	void			DoFind(PDocument *doc ,BMessage *container);
	virtual void			DoFindAll(PDocument *doc);
private:
	//----------------PCommand
};
#endif
