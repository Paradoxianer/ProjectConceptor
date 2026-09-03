#ifndef P_LAYOUTER_H
#define P_LAYOUTER_H

#include <app/Message.h>
#include <support/List.h>

/**
 * @class PLayouter
 *
 * @brief Narrow interface for a graph layout engine: nodes and
 * connections in, computed positions out. Kept intentionally free of
 * engine-specific detail (Graphviz, a future custom layout, ...) so more
 * than one implementation can coexist and callers don't need to know
 * which one they're using.
 *
 * Pure computation - a PLayouter never touches the document, never
 * issues a PCommand, and knows nothing about undo. Turning `positions`
 * into a real, undoable change is entirely up to the caller.
 *
 * @see DotLayouter (issue #104), LayoutEditor (issue #105)
 */
class PLayouter
{
public:
	virtual					~PLayouter(void) {};

	/**
	 * short, human-readable name of this layouter - e.g. for a settings
	 * dropdown, if more than one is ever registered at once
	 */
	virtual	const char*		Name(void) = 0;

	/**
	 * Computes a position (and size) for every node in `nodes`.
	 *
	 * `positions` is filled in as N parallel pairs, same indexing
	 * convention Move::Undo already uses for its own node/oldFrame
	 * pairs (see src/plugins/MoveCommand/Move.cpp):
	 *
	 *   positions->AddPointer("node", <matching entry from *nodes*>);
	 *   positions->AddRect("frame", <computed position/size>);
	 *
	 * Raw node pointers, not persisted IDs - this is an in-memory,
	 * single-process handoff to the caller, not something written to
	 * disk (Indexer's stable-ID rule is about the save format, not
	 * live command data - see docs/notes.md).
	 *
	 * Never mutates `nodes`/`connections` or the node BMessages
	 * themselves, and never touches the document - purely nodes +
	 * connections in, `positions` out. On failure, returns a real error
	 * and leaves `positions` exactly as it found it - never invents a
	 * placeholder position.
	 *
	 * @param nodes BList of BMessage* (P_C_CLASS_TYPE / P_C_GROUP_TYPE) -
	 *        the nodes to place
	 * @param connections BList of BMessage* (P_C_CONNECTION_TYPE) -
	 *        context for layouters that use edges (e.g. Graphviz);
	 *        ignored by ones that don't need them
	 * @param positions computed node/frame pairs are appended here
	 * @return B_OK on success, a real error code otherwise
	 */
	virtual	status_t		Layout(const BList *nodes, const BList *connections,
								BMessage *positions) = 0;
};

#endif
