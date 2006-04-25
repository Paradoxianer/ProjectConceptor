#ifndef GROUP_H
#define GROUP_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "PCommand.h"
#include "PDocument.h"

class Group : public PCommand
{

public:
							Group();
	
	//++++++++++++++++PCommand
	virtual	void			Undo(PDocument *doc,BMessage *undo);
	virtual	BMessage*		Do(PDocument *doc, BMessage *settings);
	virtual	char*			Name(void){return "Group";};
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);

protected:
	virtual	void			DoGroup(PDocument *doc ,BRect *rect);
	virtual	void			DoGroup(PDocument *doc ,BMessage *container);
	virtual void			DoGroupAll(PDocument *doc);
private:
	//----------------PCommand
};
#endif
