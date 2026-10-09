#ifndef STATUS_TEXTS_H
#define STATUS_TEXTS_H

#include <Point.h>
#include <Rect.h>
#include <String.h>

/** What GraphEditor shows in the window's status bar. */
namespace StatusTexts {

/** "3 nodes, 2 connections", "No selection" */
BString	Selection(int32 nodes, int32 connections);
/** a node's place and size, in document units */
BString	Geometry(BRect frame);
/** the pointer on the canvas, in document units (not zoomed) */
BString	Pointer(BPoint where);

}

#endif
