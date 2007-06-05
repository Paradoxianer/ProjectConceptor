#ifndef DELETE_H
#define DELETE_H
 /**
 * @class Delete
 * @brief Delete Command for ProjectConceptor
 *
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2007/05/20
 *
 * @todo if group is deletet delete also all subnodes
 */
#include "PCommand.h"
#include "PDocument.h"

class Delete : public PCommand
{

public:
							Delete();
	
	//++++++++++++++++PCommand
	virtual	BMessage*		Do(PDocument *doc, BMessage *settings);
	virtual	void			Undo(PDocument *doc,BMessage *undo);
	virtual	char*			Name(void){return "Delete";};
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);

protected:
	//----------------PCommand
};
#endif
