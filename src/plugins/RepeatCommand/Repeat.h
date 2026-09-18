#ifndef REPEAT_H
#define REPEAT_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "PCommand.h"
#include "PDocument.h"

/**
 * @class Repeat
 * @brief #135: runs its "PCommand::subPCommand" children (the loop body)
 * "count" times, publishing the current 0-based iteration index as
 * "counterVariable" (if given) into the macro's value context before each
 * one, so a child field bound to "$<counterVariable>" (see MacroText.cpp's
 * "$name" syntax / PCommandManager::ResolveBindings()) can use it.
 *
 * PCommand::subPCommand stays the loop body TEMPLATE, never touched by
 * Do() (see RunSubCommandsOnce()) - each iteration's actually-executed
 * state (needed for Undo()) is recorded separately, under "Repeat::
 * executed", one "iteration" entry per pass. Overwriting the template
 * itself with an unrolled N-times expansion would destroy the recorded
 * macro's own loop definition after a single playback - the exact pitfall
 * flagged on issue #135 before this was written.
 */
class Repeat : public PCommand
{

public:
							Repeat();

	//++++++++++++++++PCommand
	virtual	void			Undo(PDocument *doc,BMessage *undo);
	virtual	BMessage*		Do(PDocument *doc, BMessage *settings);
	virtual	char*			Name(void){return "Repeat";};
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);
	virtual	const property_info	*PropertyInfo(int32 *count);

protected:
private:
	//----------------PCommand
};
#endif
