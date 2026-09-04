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
 * @brief PLayouter (#104) shelling out to Graphviz's `dot` (subprocess,
 * not linked against libgvc - avoids its config6 plugin-lookup
 * requirement and mixed licensing). cmd:dot must be on PATH, see
 * IsAvailable().
 *
 * Node size is input only (current P_C_NODE_FRAME, pinned via DOT's
 * fixedsize=true) - Layout() repositions, never resizes. Exception:
 * a P_C_GROUP_TYPE node's frame comes out of Layout() sized to fit
 * its own laid-out children (see WriteDotNode()/ParseClusterBounds()).
 *
 * Grouped nodes (P_C_NODE_PARENT set) are nested inside a
 * `subgraph cluster_<index>` block instead of being flat top-level
 * nodes, so dot lays out each group's insides and positions the group
 * as a unit.
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
			void			WriteDotNode(BString &source, BMessage *node,
								const BList *nodes);
			status_t		RunDot(const BString &dotPath, const char *format,
								BString *output);
			/** Parses leaf (P_C_CLASS_TYPE) node positions from `-Tplain`
			 * output; P_C_GROUP_TYPE entries are skipped here - their
			 * frame comes from ParseClusterBounds() instead. Also fills
			 * *graphHeightPoints from the "graph" line, needed to Y-flip
			 * the cluster bounding boxes the same way.
			 */
			status_t		ParsePlainOutput(const BString &output, const BList *nodes,
								BMessage *positions, float *graphHeightPoints);
			/** Parses each P_C_GROUP_TYPE node's `bb=` from `-Tdot` output
			 * (not available in `-Tplain`) and appends its frame the same
			 * way ParsePlainOutput() does for leaves.
			 */
			status_t		ParseClusterBounds(const BString &dotOutput, const BList *nodes,
								float graphHeightPoints, BMessage *positions);
};
#endif
