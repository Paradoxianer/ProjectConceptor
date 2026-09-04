#ifndef BATCH_H
#define BATCH_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "PCommand.h"
#include "PDocument.h"

/**
 * @class Batch
 *
 * @brief Registers PCommand's own default Do()/Undo() under the name
 * "Batch": runs every "PCommand::subPCommand" entry as one undo step.
 * Not layout-specific; used by LayoutEditor (#105) for one ChangeValue
 * per repositioned node.
 */
class Batch : public PCommand
{

public:
							Batch();

	//++++++++++++++++PCommand
	// Do()/Undo() intentionally not overridden - PCommand's own default
	// implementation already is exactly this class's whole purpose.
	virtual	char*			Name(void){return "Batch";};
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);

protected:
	//----------------PCommand
};
#endif
