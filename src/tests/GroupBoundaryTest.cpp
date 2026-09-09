#include "GroupBoundaryTest.h"

#include <interface/Rect.h>

#include <vector>

#include "GroupBoundary.h"

CPPUNIT_TEST_SUITE_REGISTRATION(GroupBoundaryTest);

namespace {

// ComputeGroupBoundary()'s result is always axis-aligned, so two edges can
// only actually cross (not just share an endpoint, which is normal for a
// closed polyline) if one is horizontal, the other vertical, and each
// strictly spans the other's coordinate - a self-intersecting "bowtie" is
// exactly this happening between two non-adjacent edges.
bool SelfIntersects(const std::vector<BPoint> &poly)
{
	size_t	n	= poly.size();
	for (size_t i=0; i<n; i++) {
		BPoint	a1	= poly[i];
		BPoint	a2	= poly[(i+1)%n];
		bool	aHorizontal	= (a1.y == a2.y);
		for (size_t j=i+1; j<n; j++) {
			bool	adjacent	= (j == i+1) || ((i==0) && (j==n-1));
			if (adjacent)
				continue;
			BPoint	b1	= poly[j];
			BPoint	b2	= poly[(j+1)%n];
			bool	bHorizontal	= (b1.y == b2.y);
			if (aHorizontal == bHorizontal)
				continue;
			BPoint	h1	= aHorizontal ? a1 : b1;
			BPoint	h2	= aHorizontal ? a2 : b2;
			BPoint	v1	= aHorizontal ? b1 : a1;
			BPoint	v2	= aHorizontal ? b2 : a2;
			float	hy		= h1.y;
			float	hxMin	= std::min(h1.x,h2.x), hxMax = std::max(h1.x,h2.x);
			float	vx		= v1.x;
			float	vyMin	= std::min(v1.y,v2.y), vyMax = std::max(v1.y,v2.y);
			if ((vx > hxMin) && (vx < hxMax) && (hy > vyMin) && (hy < vyMax))
				return true;
		}
	}
	return false;
}

}


void GroupBoundaryTest::OverlappingChildrenProduceNoSelfIntersection(void)
{
	// the shape this session's main #38 work was built and confirmed
	// against - a chain of children whose y-ranges do overlap
	std::vector<BRect>	rects;
	rects.push_back(BRect(164,330,277,383));
	rects.push_back(BRect(364,265,477,358));
	rects.push_back(BRect(364,350,477,443));
	rects.push_back(BRect(564,305,677,398));
	rects.push_back(BRect(564,390,677,483));

	std::vector<BPoint>	poly	= ComputeGroupBoundary(rects,22.8f);
	CPPUNIT_ASSERT(!SelfIntersects(poly));
}

void GroupBoundaryTest::NonOverlappingChildrenGetACorridorNotABowtie(void)
{
	// regression test: two children with *no* y-overlap at all (one
	// entirely above the other) - the valley/mountain envelope used to
	// cross itself in the gap column between them instead of bridging it
	std::vector<BRect>	rects;
	rects.push_back(BRect(100,100,200,200));
	rects.push_back(BRect(400,500,450,570));

	std::vector<BPoint>	poly	= ComputeGroupBoundary(rects,20);
	CPPUNIT_ASSERT(!SelfIntersects(poly));

	// and each child's own tight column must still be untouched by the
	// corridor fix - only the gap between them should have moved
	bool	foundChild1Top		= false;
	bool	foundChild2Bottom	= false;
	for (size_t i=0; i<poly.size(); i++) {
		if ((poly[i].x == 100) && (poly[i].y == 80))		foundChild1Top		= true;
		if ((poly[i].x == 450) && (poly[i].y == 570))		foundChild2Bottom	= true;
	}
	CPPUNIT_ASSERT(foundChild1Top);
	CPPUNIT_ASSERT(foundChild2Bottom);
}

void GroupBoundaryTest::ChainOfNonOverlappingChildrenAllGetCorridors(void)
{
	// the general case the user actually described: every child after the
	// first sits either entirely above or entirely below its neighbour
	std::vector<BRect>	rects;
	rects.push_back(BRect(100,100,200,200));
	rects.push_back(BRect(300,500,350,540));
	rects.push_back(BRect(500,900,550,950));

	std::vector<BPoint>	poly	= ComputeGroupBoundary(rects,20);
	CPPUNIT_ASSERT(!SelfIntersects(poly));
}

void GroupBoundaryTest::ExistingFixtureShapeIsUnchanged(void)
{
	// the corridor pass must be a no-op whenever nothing is actually
	// inverted - same fixture/labelSpace as
	// OverlappingChildrenProduceNoSelfIntersection, checked against the
	// exact point sequence verified by hand this session, so a corridor
	// pass that fires when it shouldn't would show up as a shape change
	std::vector<BRect>	rects;
	rects.push_back(BRect(164,330,277,383));
	rects.push_back(BRect(364,265,477,358));
	rects.push_back(BRect(364,350,477,443));
	rects.push_back(BRect(564,305,677,398));
	rects.push_back(BRect(564,390,677,483));

	std::vector<BPoint>	poly	= ComputeGroupBoundary(rects,22.8f);

	std::vector<BPoint>	expected;
	expected.push_back(BPoint(164,307.2f));
	expected.push_back(BPoint(364,307.2f));
	expected.push_back(BPoint(364,265));
	expected.push_back(BPoint(477,265));
	expected.push_back(BPoint(477,305));
	expected.push_back(BPoint(677,305));
	expected.push_back(BPoint(677,483));
	expected.push_back(BPoint(564,483));
	expected.push_back(BPoint(564,443));
	expected.push_back(BPoint(364,443));
	expected.push_back(BPoint(364,383));
	expected.push_back(BPoint(164,383));

	CPPUNIT_ASSERT_EQUAL(expected.size(),poly.size());
	for (size_t i=0; i<expected.size(); i++) {
		CPPUNIT_ASSERT_DOUBLES_EQUAL(expected[i].x,poly[i].x,0.01);
		CPPUNIT_ASSERT_DOUBLES_EQUAL(expected[i].y,poly[i].y,0.01);
	}
}
