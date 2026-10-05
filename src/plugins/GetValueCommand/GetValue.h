#ifndef GET_VALUE_H
#define GET_VALUE_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "PCommand.h"
#include "PDocument.h"

/**
 * @class GetValue
 * @brief The reading counterpart to ChangeValue: reads one attribute of
 * "node" (valueContainer: name, subgroup path, optional index - the same
 * shape ChangeValue takes) into "resultVariable" in the macro's value
 * context. Numbers are stored as float (Calculate's own type), text as
 * string, bools as bool. An attribute added in the editor is a block
 * {Name, Value} - its Value is read. A missing attribute stores "default" if given,
 * otherwise it's a playback error - never a silent 0. The variable is
 * cleared first, so a ForEach pass can't reuse the previous node's value.
 *
 * Undo() is a no-op, same as Calculate's: only the value context changes.
 */
class GetValue : public PCommand
{

public:
							GetValue();

	//++++++++++++++++PCommand
	virtual	void			Undo(PDocument *doc,BMessage *undo);
	virtual	BMessage*		Do(PDocument *doc, BMessage *settings);
	virtual	char*			Name(void){return "GetValue";};
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);
	virtual	const property_info	*PropertyInfo(int32 *count);
};
#endif
