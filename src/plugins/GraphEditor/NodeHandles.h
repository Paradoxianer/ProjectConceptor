#ifndef NODE_HANDLES_H
#define NODE_HANDLES_H

#include <Point.h>
#include <Rect.h>

/** The eight resize handles of a selected node: where they sit, which one
 * is under the mouse, and the frame a drag on one of them produces.
 * Corners change both axes, edge handles only one.
 */
namespace NodeHandles {

enum handle {
	NONE	= -1,
	TOP_LEFT,
	TOP,
	TOP_RIGHT,
	RIGHT,
	BOTTOM_RIGHT,
	BOTTOM,
	BOTTOM_LEFT,
	LEFT,
	COUNT
};

BPoint	Position(BRect frame, int32 handle);
/** the handle within radius of where, NONE if there is none */
int32	HandleAt(BRect frame, BPoint where, float radius);
/** start dragged by (dx, dy) on handle: the edges the handle owns move,
 * the opposite ones stay; never smaller than minWidth x minHeight */
BRect	Resized(BRect start, int32 handle, float dx, float dy,
			float minWidth, float minHeight);
/** the edges handle owns, moved onto the grid */
BRect	Snapped(BRect frame, int32 handle, float grid);

}

#endif
