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
 * @brief PLayouter implementation (issue #104) that shells out to
 * Graphviz's `dot`: writes the graph as DOT source to a temp file, runs
 * `dot -Tplain <file>`, and parses the plain-format positions back.
 *
 * Deliberately a subprocess, not a link against libgvc - avoids the
 * config6 plugin-lookup problem Graphviz's C API needs at runtime, and
 * keeps this project's own MIT license clean of libgvc's mixed
 * licensing. cmd:dot must be on PATH - see IsAvailable().
 *
 * Node size is a hint *into* the layout (each node's current
 * P_C_NODE_FRAME width/height, converted to inches and pinned via DOT's
 * fixedsize=true), not an output - Layout() only ever changes position,
 * a node's own size always comes back unchanged. Auto-Layout resizing
 * nodes as a side effect would be a surprising thing for a "just
 * reposition" action to do; if that's ever wanted it should be its own,
 * explicit feature.
 */
class DotLayouter : public PLayouter
{
public:
							DotLayouter(void);
	virtual					~DotLayouter(void);

	virtual	const char*		Name(void) {return "Graphviz (dot)";};
	virtual	status_t		Layout(const BList *nodes, const BList *connections,
								BMessage *positions);

	/** true if the `dot` binary could actually be run - check before
	 * offering this layouter in the UI, so a missing Graphviz install
	 * shows up as "not available" rather than every attempt failing with
	 * a generic error.
	 */
			bool			IsAvailable(void);

protected:
			status_t		WriteDotFile(const BString &path, const BList *nodes,
								const BList *connections);
			status_t		ParsePlainOutput(const BString &output, const BList *nodes,
								BMessage *positions);
};
#endif
