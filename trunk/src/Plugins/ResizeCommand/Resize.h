#ifndef INSERT_H
#define INSERT_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "PCommand.h"
#include "PDocument.h"

class Resize : public PCommand
{

public:
							Resize();
	
	//++++++++++++++++PCommand
	virtual	BMessage*		Do(PDocument *doc, BMessage *settings);
	virtual	void			Undo(PDocument *doc,BMessage *undo);
	virtual	char*			Name(void){return "Resize";};
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);

protected:
			void			DoResize(PDocument *doc,BRect *rect,BMessage *settings);
	//----------------PCommand
};
#endif
