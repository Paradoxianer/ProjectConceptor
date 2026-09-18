#ifndef ASK_H
#define ASK_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "PCommand.h"
#include "PDocument.h"

/**
 * @class Ask
 * @brief #135: shows a blocking text-input dialog (the existing
 * InputRequest UI, already used by GraphEditor/NavigatorCommands
 * interactively - this is the same mechanism, just reachable as a
 * PCommand so it can be part of a recorded/hand-written macro too) and
 * stores whatever was typed as a string under "variable" in the macro's
 * value context, for a later field bound to "$<variable>".
 *
 * Undo() removes the variable again rather than restoring a shadowed
 * prior value under the same name, if there ever was one - a known,
 * accepted limitation (an Ask into an already-used variable name is an
 * edge case, not the intended pattern) rather than something this claims
 * to handle correctly.
 */
class Ask : public PCommand
{

public:
							Ask();

	//++++++++++++++++PCommand
	virtual	void			Undo(PDocument *doc,BMessage *undo);
	virtual	BMessage*		Do(PDocument *doc, BMessage *settings);
	virtual	char*			Name(void){return "Ask";};
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);
	virtual	const property_info	*PropertyInfo(int32 *count);

protected:
private:
	//----------------PCommand
};
#endif
