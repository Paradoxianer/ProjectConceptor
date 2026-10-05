#ifndef SELECT_CONNECTED_H
#define SELECT_CONNECTED_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "PCommand.h"
#include "PDocument.h"

/**
 * @class SelectConnected
 * @brief Adds every node reachable from the current selection over
 * connections to the selection. "direction": "both" (default),
 * "outgoing" (follow from -> to) or "incoming" (to -> from). "depth":
 * how many hops, 0 (default) = until nothing new is found - a cycle in
 * the graph can't loop forever. Undo restores the previous selection.
 */
class SelectConnected : public PCommand
{

public:
							SelectConnected();

	//++++++++++++++++PCommand
	virtual	void			Undo(PDocument *doc,BMessage *undo);
	virtual	BMessage*		Do(PDocument *doc, BMessage *settings);
	virtual	char*			Name(void){return "SelectConnected";};
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);
	virtual	const property_info	*PropertyInfo(int32 *count);

protected:
			void			MarkSelected(PDocument *doc, BMessage *node);
};
#endif
