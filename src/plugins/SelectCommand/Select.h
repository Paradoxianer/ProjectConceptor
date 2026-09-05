#ifndef SELECT_H
#define SELECT_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "PCommand.h"
#include "PDocument.h"

class Select : public PCommand
{

public:
							Select();
	
	//++++++++++++++++PCommand
	virtual	void			Undo(PDocument *doc,BMessage *undo);
	virtual	BMessage*		Do(PDocument *doc, BMessage *settings);
	virtual	char*			Name(void){return "Select";};
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);

protected:
	virtual	void			DoSelect(PDocument *doc ,BRect *rect);
	virtual	void			DoSelect(PDocument *doc ,BMessage *container);
	virtual void			DoSelectAll(PDocument *doc);
			/** marks every entry of one of the document's node lists
			 * selected - see DoSelectAll(), which covers both nodes and
			 * connections. */
			void			SelectList(PDocument *doc,BList *all);
private:
	//----------------PCommand
};
#endif
