#ifndef LAYOUT_COMMAND_BUILDER_H
#define LAYOUT_COMMAND_BUILDER_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <app/Message.h>
#include <support/List.h>

/**
 * Shared "positions BMessage -> undoable command" step, used by both
 * LayoutEditor's toolbar-triggered Auto-Layout and the Layout PCommand
 * plugin (#55) - one implementation, not two.
 */

/** Shifts every "frame" entry in `positions` (parallel "node"/"frame"
 * pairs, PLayouter::Layout()'s own convention) so the new layout's
 * bounding-box center lands on the old bounding-box center (from
 * `nodes`'s current P_C_NODE_FRAME) instead of wherever the layouter's
 * own coordinate space happens to put it - a fresh layout starts near its
 * own origin, which reads as "graph jammed into a corner" once applied.
 * No-op if either bounding box is empty.
 */
void	LayoutCenterOnOldBounds(const BList *nodes, BMessage *positions);

/** Appends one "ChangeValue" "PCommand::subPCommand" entry per "node"/
 * "frame" pair in `positions` directly onto `target` - ChangeValue, not
 * Move: Move applies one dx/dy to the whole selection, not an absolute
 * per-node position. Returns the number of subcommands appended (0 if
 * `positions` had no usable pairs).
 */
int32	LayoutAppendSubCommands(BMessage *positions, BMessage *target);

/** Wraps LayoutAppendSubCommands() in a fresh "Batch" P_C_EXECUTE_COMMAND
 * message - single undo step. Returns NULL if `positions` had no usable
 * pairs. Caller owns the returned message.
 */
BMessage*	LayoutBuildBatchCommand(BMessage *positions);

#endif
