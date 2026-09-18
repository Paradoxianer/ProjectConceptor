#ifndef IF_H
#define IF_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "PCommand.h"
#include "PDocument.h"

/**
 * @class If
 * @brief #135: runs its "PCommand::subPCommand" children once, only if
 * "searchString" is found somewhere in "scope" (nodes/connections/both,
 * default nodes - same field names/semantics as Find, and the same
 * underlying search, NodeSearch.h's NodeMatchesSearch()). Unlike Find,
 * never touches the document's selection itself - it only checks.
 */
class If : public PCommand
{

public:
							If();

	//++++++++++++++++PCommand
	virtual	void			Undo(PDocument *doc,BMessage *undo);
	virtual	BMessage*		Do(PDocument *doc, BMessage *settings);
	virtual	char*			Name(void){return "If";};
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);
	virtual	const property_info	*PropertyInfo(int32 *count);

protected:
private:
	//----------------PCommand
};
#endif
