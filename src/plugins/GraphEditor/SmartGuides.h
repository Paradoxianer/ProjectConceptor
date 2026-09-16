#ifndef SMART_GUIDES_H
#define SMART_GUIDES_H
/*
 * Pure geometry behind #127's alignment guides - a node being dragged
 * snaps to another node's edge/center, or to equal spacing with its
 * nearest neighbours, the way Keynote/Figma/Illustrator "smart guides"
 * work. Deliberately free of BView/BMessage so it's unit-testable
 * without a live drag, the same way GroupBoundary.cpp is (see
 * GroupBoundaryTest).
 */
#include <interface/Rect.h>
#include <support/List.h>

struct GuideMatch {
	bool	active;
	float	linePos;
	float	lineStart, lineEnd;
};

// horizontal = a row, matched/drawn on the Y-coordinate, adjusts dy
// vertical   = a column, matched/drawn on the X-coordinate, adjusts dx
struct GuideSnapResult {
	GuideMatch	horizontal, vertical;
	float		dx, dy;
};

// startFrame: the dragged node's frame before this drag started.
// rawDx/rawDy: the unsnapped offset from startFrame the mouse currently asks for.
// targetFrames: a BList of BRect* for every other node to align against -
// caller filters out the dragged node itself, anything else currently
// selected/moving, and connections before calling this.
// threshold: max distance (document units) a match may be off by.
GuideSnapResult ComputeGuideSnap(BRect startFrame, float rawDx, float rawDy,
	BList *targetFrames, float threshold);

#endif
