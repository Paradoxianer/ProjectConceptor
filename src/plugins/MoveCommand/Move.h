#ifndef MOVE_H
#define MOVE_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "PCommand.h"
#include "PDocument.h"
#include "ProjectConceptorDefs.h"

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
	
			void			MoveNode(PDocument *doc, set<BMessage*> *changed, BMessage *node, float dx, float dy,BMessage *undoMessage);
			void			ResizeParent(PDocument *doc, set<BMessage*> *changed, BMessage *node, float dx, float dy,BMessage *undoMessage);

protected:
	//----------------PCommand
};
#endif
