#ifndef CALCULATE_H
#define CALCULATE_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "PCommand.h"
#include "PDocument.h"

/**
 * @class Calculate
 * @brief #135 follow-up (user report): basic arithmetic on macro
 * variables - "left <operator> right" (binary) or "<operator> left"
 * (unary: round/floor/ceil/abs), the result stored under
 * "resultVariable" in the macro's value context, same store-only shape
 * Remember uses. "left"/"right" are ordinary float fields - bound to
 * "$someVariable" (PCommandManager::ResolveBindings() resolves that to
 * a literal float on `settings` before Do() ever runs, see Repeat's own
 * "dx=$i"), they read whatever an earlier Remember/Ask/another
 * Calculate stored; left as a literal, they're just a constant.
 *
 * Undo() is a no-op for the same reason Remember's is (see its own class
 * comment): only ever writes into the macro's value context, which
 * PlayMacro() discards once the macro finishes - never document state.
 */
class Calculate : public PCommand
{

public:
							Calculate();

	//++++++++++++++++PCommand
	virtual	void			Undo(PDocument *doc,BMessage *undo);
	virtual	BMessage*		Do(PDocument *doc, BMessage *settings);
	virtual	char*			Name(void){return "Calculate";};
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);
	virtual	const property_info	*PropertyInfo(int32 *count);

protected:
private:
	//----------------PCommand
};
#endif
