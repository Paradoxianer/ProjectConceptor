#ifndef FOREACH_H
#define FOREACH_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "PCommand.h"
#include "PDocument.h"

/**
 * @class ForEach
 * @brief #135: runs its "PCommand::subPCommand" children (the loop body)
 * once per node in the selection at the moment Do() runs (a snapshot - a
 * child that itself changes the selection doesn't change how many times
 * this loop runs), publishing the current node as "nodeVariable" (if
 * given) into the macro's value context before each pass, so a child
 * field bound to "$<nodeVariable>" can act on exactly that node (e.g.
 * ChangeValue's "node" field).
 *
 * Selection-driven commands elsewhere in this codebase (ChangeValue,
 * AddAttribute, ...) already act on the *whole* selection at once via
 * Node::selected=true - this is what makes "once per node, in turn"
 * possible at all, needed for anything that has to tell the nodes apart
 * (numbering them, say).
 *
 * Same subPCommand-template/executed-record split as Repeat - see there.
 */
class ForEach : public PCommand
{

public:
							ForEach();

	//++++++++++++++++PCommand
	virtual	void			Undo(PDocument *doc,BMessage *undo);
	virtual	BMessage*		Do(PDocument *doc, BMessage *settings);
	virtual	char*			Name(void){return "ForEach";};
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);
	virtual	const property_info	*PropertyInfo(int32 *count);

protected:
private:
	//----------------PCommand
};
#endif
