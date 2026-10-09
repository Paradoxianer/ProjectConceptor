#include "NodeHandles.h"

#include <math.h>


namespace NodeHandles {

static float Min(float a, float b) { return a < b ? a : b; }
static float Max(float a, float b) { return a > b ? a : b; }
static float OnGrid(float value, float grid) { return floorf(value / grid + 0.5f) * grid; }


static bool MovesLeft(int32 handle)
{
	return (handle == TOP_LEFT) || (handle == LEFT) || (handle == BOTTOM_LEFT);
}


static bool MovesRight(int32 handle)
{
	return (handle == TOP_RIGHT) || (handle == RIGHT) || (handle == BOTTOM_RIGHT);
}


static bool MovesTop(int32 handle)
{
	return (handle == TOP_LEFT) || (handle == TOP) || (handle == TOP_RIGHT);
}


static bool MovesBottom(int32 handle)
{
	return (handle == BOTTOM_LEFT) || (handle == BOTTOM) || (handle == BOTTOM_RIGHT);
}


BPoint Position(BRect frame, int32 handle)
{
	float	x	= (frame.left + frame.right) / 2;
	float	y	= (frame.top + frame.bottom) / 2;
	if (MovesLeft(handle))
		x	= frame.left;
	else if (MovesRight(handle))
		x	= frame.right;
	if (MovesTop(handle))
		y	= frame.top;
	else if (MovesBottom(handle))
		y	= frame.bottom;
	return BPoint(x, y);
}


int32 HandleAt(BRect frame, BPoint where, float radius)
{
	int32	best		= NONE;
	float	bestDist	= radius * radius;
	for (int32 handle = 0; handle < COUNT; handle++) {
		BPoint	d		= Position(frame, handle) - where;
		float	dist	= d.x * d.x + d.y * d.y;
		if (dist <= bestDist) {
			best		= handle;
			bestDist	= dist;
		}
	}
	return best;
}


BRect Resized(BRect start, int32 handle, float dx, float dy,
	float minWidth, float minHeight)
{
	BRect	frame	= start;
	if (MovesLeft(handle))
		frame.left		= Min(start.left + dx, start.right - minWidth);
	else if (MovesRight(handle))
		frame.right		= Max(start.right + dx, start.left + minWidth);
	if (MovesTop(handle))
		frame.top		= Min(start.top + dy, start.bottom - minHeight);
	else if (MovesBottom(handle))
		frame.bottom	= Max(start.bottom + dy, start.top + minHeight);
	return frame;
}


BRect Snapped(BRect frame, int32 handle, float grid)
{
	if (grid <= 0)
		return frame;
	if (MovesLeft(handle))
		frame.left		= OnGrid(frame.left, grid);
	else if (MovesRight(handle))
		frame.right		= OnGrid(frame.right, grid);
	if (MovesTop(handle))
		frame.top		= OnGrid(frame.top, grid);
	else if (MovesBottom(handle))
		frame.bottom	= OnGrid(frame.bottom, grid);
	return frame;
}

}
