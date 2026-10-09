#ifndef RESIZE_H
#define RESIZE_H
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
	virtual	const property_info	*PropertyInfo(int32 *count);
	//----------------PCommand
};
#endif
