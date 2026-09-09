#include "GroupBoundary.h"

#include <algorithm>
#include <set>

using std::vector;
using std::set;


static void PushIfNew(vector<BPoint> &points, BPoint p)
{
	if (points.empty() || (points.back() != p))
		points.push_back(p);
}


// The tightest *orthogonally convex* outline around the children (issue
// #38) - a string pulled taut around them, but bending only at right
// angles. A plain convex hull is wrong twice over: it draws diagonals, and
// it bulges out to the tallest/widest neighbour, swallowing a short child
// two columns over. A per-column silhouette ("skyline") fixes the
// swallowing but goes too far the other way - it happily carves a bay, an
// empty pocket walled in on three sides, wherever a shallow child sits
// between two deeper ones, and a taut string would never dip into one.
//
// Orthogonal convexity is exactly the property that rules bays out: every
// horizontal and vertical line must meet the shape in one run. Per column
// the shape is one run by construction, so what remains is that the top
// edge be valley-shaped (falling, then rising) and the bottom edge
// mountain-shaped. The tightest such pair is a prefix/suffix scan - see
// ComputeGroupBoundary() - and it needs no special case for a column no
// child covers, since the scans bridge those on their own.
vector<BPoint> ComputeGroupBoundary(const vector<BRect> &rects, float labelSpace)
{
	vector<BPoint>	polygon;
	if (rects.empty())
		return polygon;

	set<float>	xset;
	for (uint32 i=0; i<rects.size(); i++) {
		xset.insert(rects[i].left);
		xset.insert(rects[i].right);
	}
	vector<float>	xs(xset.begin(),xset.end());
	int32	n	= xs.size()-1;
	if (n <= 0)
		return polygon;

	// per column (interval between two consecutive critical x values): the
	// tightest top/bottom among whichever rects actually cover it. A
	// column no child covers keeps the outward sentinel, i.e. contributes
	// no constraint of its own - the envelope scans below bridge it.
	const float		kFar	= 1e30;
	vector<float>	topY(n,kFar), bottomY(n,-kFar);
	for (int32 i=0; i<n; i++) {
		float	midX	= (xs[i]+xs[i+1])/2;
		for (uint32 r=0; r<rects.size(); r++) {
			if ((rects[r].left <= midX) && (midX < rects[r].right)) {
				if (rects[r].top < topY[i]) topY[i] = rects[r].top;
				if (rects[r].bottom > bottomY[i]) bottomY[i] = rects[r].bottom;
			}
		}
	}
	// The name (and this group's own attribute rows) sit above the
	// leftmost child, so that column's top edge is its child's top raised
	// by labelSpace. Fold it in before the scans so the raised level is
	// what they carry, not the child's bare top.
	topY[0]	-= labelSpace;

	// Tightest valley-shaped top / mountain-shaped bottom (see the note
	// above the function). Largest valley below a set of constraints is
	// the pointwise max of the running minima taken from each end, and
	// the mirror image gives the smallest mountain above them.
	vector<float>	topPre(n), topSuf(n), botPre(n), botSuf(n);
	float	run	= kFar;
	for (int32 i=0; i<n; i++)		{ run = std::min(run,topY[i]);    topPre[i] = run; }
	run	= kFar;
	for (int32 i=n-1; i>=0; i--)	{ run = std::min(run,topY[i]);    topSuf[i] = run; }
	run	= -kFar;
	for (int32 i=0; i<n; i++)		{ run = std::max(run,bottomY[i]); botPre[i] = run; }
	run	= -kFar;
	for (int32 i=n-1; i>=0; i--)	{ run = std::max(run,bottomY[i]); botSuf[i] = run; }
	for (int32 i=0; i<n; i++) {
		topY[i]		= std::max(topPre[i],topSuf[i]);
		bottomY[i]	= std::min(botPre[i],botSuf[i]);
	}

	// A column can still come out inverted (topY below bottomY) - the
	// valley/mountain envelope is only guaranteed non-inverted where a
	// child actually covers the column; in a gap between two children with
	// no y-overlap at all (one entirely above the other), the "valley
	// pushed up from below" and the "mountain pushed down from above" cross
	// past each other, which drawn as-is is a self-intersecting bowtie, not
	// just an odd shape. Bridge that gap with a fixed-height corridor
	// instead of the impossible envelope. Every column in one gap run
	// carries an identical (topY,bottomY) pair - neither prefix-min nor
	// suffix-min changes across a run of columns nothing covers - so
	// recentering each column independently on the midpoint of its own
	// (invalid) range reproduces one level band across the whole run with
	// no separate run-boundary tracking needed.
	// Wider than it first looks it needs to be: RoundCorners() clamps its
	// rounding radius to at most half of the shortest edge meeting at a
	// corner, so a corridor much narrower than twice the usual corner
	// radius rounds almost its whole height away at both ends and reads
	// as a thin line instead of a band.
	const float		kCorridorHeight	= 32;
	vector<bool>	isCorridor(n,false);
	for (int32 i=0; i<n; i++) {
		if (topY[i] > bottomY[i]) {
			isCorridor[i]	= true;
			float	center	= (topY[i]+bottomY[i])/2;
			topY[i]		= center-kCorridorHeight/2;
			bottomY[i]	= center+kCorridorHeight/2;
		}
	}

	// Where the corridor meets a real child, both topY and bottomY step at
	// the very same x (the child's own edge) - the top edge's drop and the
	// bottom edge's drop land on the exact same vertical line, which draws
	// as one line, not as a corridor with any width of its own along that
	// drop (this is what "just two lines lying on top of each other" -
	// your description - actually is). The two steps don't overlap with
	// either the child's own rect or the corridor's flat span, though (the
	// coincident stretch is the empty run between them), so nudging them
	// a few px apart - one edge steps a little early, the other a little
	// late - costs nothing and gives that whole connecting run real,
	// recognizable width instead of a single line. Ordinary transitions
	// (not a corridor boundary) are untouched.
	//
	// Which edge goes early matters: stepping the wrong one re-creates
	// the exact inversion this file exists to avoid, since for the x
	// range between the two steps, one side already carries its new
	// value while the other still carries its old one - that pairing
	// must itself satisfy top<bottom, and only one of the two pairings
	// (new top vs. old bottom, or old top vs. new bottom) does. Skip the
	// padding on the rare transition where neither does (columns too
	// narrow) - falling back to the coincident-but-valid single line
	// beats stepping into a fresh bowtie.
	const float		kCorridorSidePadding	= 8;
	vector<float>	topStepX(n), bottomStepX(n);
	for (int32 k=0; k<n; k++) {
		topStepX[k]		= xs[k];
		bottomStepX[k]	= xs[k];
	}
	for (int32 k=1; k<n; k++) {
		if ((isCorridor[k] != isCorridor[k-1])
				&& (topY[k] != topY[k-1]) && (bottomY[k] != bottomY[k-1])) {
			float	leftRoom	= xs[k]-xs[k-1];
			float	rightRoom	= xs[k+1]-xs[k];
			float	pad			= std::min(kCorridorSidePadding,std::min(leftRoom,rightRoom)/2);
			if (topY[k] < bottomY[k-1]) {
				topStepX[k]		= xs[k]-pad;
				bottomStepX[k]	= xs[k]+pad;
			} else if (topY[k-1] < bottomY[k]) {
				bottomStepX[k]	= xs[k]-pad;
				topStepX[k]		= xs[k]+pad;
			}
		}
	}

	// top boundary, left to right
	polygon.push_back(BPoint(xs[0],topY[0]));
	float	prevTop	= topY[0];
	for (int32 i=1; i<n; i++) {
		if (topY[i] != prevTop) {
			PushIfNew(polygon,BPoint(topStepX[i],prevTop));
			polygon.push_back(BPoint(topStepX[i],topY[i]));
			prevTop	= topY[i];
		}
	}
	PushIfNew(polygon,BPoint(xs[n],prevTop));

	// right edge, then bottom boundary, right to left
	float	prevBottom	= bottomY[n-1];
	PushIfNew(polygon,BPoint(xs[n],prevBottom));
	for (int32 i=n-2; i>=0; i--) {
		if (bottomY[i] != prevBottom) {
			PushIfNew(polygon,BPoint(bottomStepX[i+1],prevBottom));
			polygon.push_back(BPoint(bottomStepX[i+1],bottomY[i]));
			prevBottom	= bottomY[i];
		}
	}
	PushIfNew(polygon,BPoint(xs[0],prevBottom));
	// left edge back up to the label notch is implicit - StrokePolygon/
	// FillPolygon close the polygon back to its first point on their own

	return polygon;
}
