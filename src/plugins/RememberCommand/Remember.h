#ifndef REMEMBER_H
#define REMEMBER_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "PCommand.h"
#include "PDocument.h"

/**
 * @class Remember
 * @brief #135: saves the current selection (every node/connection with
 * Node::selected) under "variable" in the macro's value context - store-
 * only, deliberately. Getting the selection back is just a plain Select
 * with its own "node" field bound to "$<variable>" (PCommandManager::
 * ResolveBindings() copies every one of the variable's own pointer
 * entries into Select's repeated "node" field, exactly reproducing the
 * remembered selection) - reusing Select's own already-correct, already-
 * tested behavior (including its "deselect" flag) instead of duplicating
 * a second, parallel "restore" implementation here for no real benefit.
 *
 * Undo() is a no-op (matches Copy::Undo(), the only other command in this
 * codebase with an empty one, for the same reason): this only ever writes
 * into the macro's value context, which PlayMacro() discards entirely
 * once the macro finishes regardless - never part of the document state
 * Undo() is responsible for.
 */
class Remember : public PCommand
{

public:
							Remember();

	//++++++++++++++++PCommand
	virtual	void			Undo(PDocument *doc,BMessage *undo);
	virtual	BMessage*		Do(PDocument *doc, BMessage *settings);
	virtual	char*			Name(void){return "Remember";};
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);
	virtual	const property_info	*PropertyInfo(int32 *count);

protected:
private:
	//----------------PCommand
};
#endif
