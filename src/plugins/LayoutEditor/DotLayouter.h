#ifndef DOT_LAYOUTER_H
#define DOT_LAYOUTER_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "PLayouter.h"

#include <support/String.h>

/**
 * @class DotLayouter
 *
 * @brief PLayouter (#104) shelling out to Graphviz's `dot -Tplain`
 * (subprocess, not linked against libgvc - avoids its config6
 * plugin-lookup requirement and mixed licensing). cmd:dot must be on
 * PATH, see IsAvailable().
 *
 * Node size is input only (current P_C_NODE_FRAME, pinned via DOT's
 * fixedsize=true) - Layout() repositions, never resizes.
 */
class DotLayouter : public PLayouter
{
public:
							DotLayouter(void);
	virtual					~DotLayouter(void);

	virtual	const char*		Name(void) {return "Graphviz (dot)";};
	virtual	status_t		Layout(const BList *nodes, const BList *connections,
								BMessage *positions);

	/** true if `dot` could actually be run. */
	virtual	bool			IsAvailable(void);

protected:
			status_t		WriteDotFile(const BString &path, const BList *nodes,
								const BList *connections);
			status_t		ParsePlainOutput(const BString &output, const BList *nodes,
								BMessage *positions);
};
#endif
