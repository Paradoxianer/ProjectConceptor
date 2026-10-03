#include "SmartGuidesTest.h"

#include <interface/Rect.h>
#include <support/List.h>

#include "SmartGuides.h"

CPPUNIT_TEST_SUITE_REGISTRATION(SmartGuidesTest);

static const float kThreshold = 5.0f;


void SmartGuidesTest::NoTargetsProducesNoMatch(void)
{
	BList	targets;
	GuideSnapResult	result	= ComputeGuideSnap(BRect(0,0,50,50), 10, 20, &targets, kThreshold);
	CPPUNIT_ASSERT(!result.horizontal.active);
	CPPUNIT_ASSERT(!result.vertical.active);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(10, result.dx, 0.01);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(20, result.dy, 0.01);
}

void SmartGuidesTest::OutsideThresholdProducesNoMatch(void)
{
	BRect	target(1000,1000,1050,1050);
	BList	targets;
	targets.AddItem(&target);
	GuideSnapResult	result	= ComputeGuideSnap(BRect(0,0,50,50), 0, 0, &targets, kThreshold);
	CPPUNIT_ASSERT(!result.horizontal.active);
	CPPUNIT_ASSERT(!result.vertical.active);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0, result.dx, 0.01);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0, result.dy, 0.01);
}

// Candidate top (after the raw drag offset) is 3 units off a target's top -
// within threshold. Target height deliberately differs from the dragged
// node's so bottom-to-bottom can't tie with top-to-top, and the target sits
// far away on X so nothing on that axis can match either - isolates a
// single top-edge match.
void SmartGuidesTest::TopEdgeAlignsToTopEdge(void)
{
	BRect	target(20000,3,20050,60);
	BList	targets;
	targets.AddItem(&target);
	GuideSnapResult	result	= ComputeGuideSnap(BRect(0,0,50,50), 10000, 0, &targets, kThreshold);
	CPPUNIT_ASSERT(result.horizontal.active);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(3, result.horizontal.linePos, 0.01);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(3, result.dy, 0.01);
	CPPUNIT_ASSERT(!result.vertical.active);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(10000, result.dx, 0.01);
}

// Candidate's left edge lands 3 units short of a target's right edge -
// "flush against the right side of another node". Target sits far away on Y
// so nothing there can match, isolating a single left-to-right match.
void SmartGuidesTest::LeftEdgeAlignsToRightEdge(void)
{
	BRect	target(50,20000,97,20050);
	BList	targets;
	targets.AddItem(&target);
	GuideSnapResult	result	= ComputeGuideSnap(BRect(0,0,50,50), 100, 10000, &targets, kThreshold);
	CPPUNIT_ASSERT(result.vertical.active);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(97, result.vertical.linePos, 0.01);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(97, result.dx, 0.01);
	CPPUNIT_ASSERT(!result.horizontal.active);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(10000, result.dy, 0.01);
}

// Target is wider than the dragged node but exactly center-aligned with it -
// left/right edges individually stay far apart, only the centers coincide.
void SmartGuidesTest::CentersAlign(void)
{
	BRect	target(180,20000,280,20040);
	BList	targets;
	targets.AddItem(&target);
	GuideSnapResult	result	= ComputeGuideSnap(BRect(0,0,60,40), 200, 10000, &targets, kThreshold);
	CPPUNIT_ASSERT(result.vertical.active);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(230, result.vertical.linePos, 0.01);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(200, result.dx, 0.01);
	CPPUNIT_ASSERT(!result.horizontal.active);
}

// Two targets both within threshold on the row axis (top-edge distance 4 and
// 1 respectively) - the closer one (distance 1) must win regardless of which
// was added to the list first.
void SmartGuidesTest::ClosestOfSeveralCandidatesWins(void)
{
	BRect	farther(20000,4,20050,61);
	BRect	closer(20000,1,20050,58);
	BList	targets;
	targets.AddItem(&farther);
	targets.AddItem(&closer);
	GuideSnapResult	result	= ComputeGuideSnap(BRect(0,0,50,50), 10000, 0, &targets, kThreshold);
	CPPUNIT_ASSERT(result.horizontal.active);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1, result.horizontal.linePos, 0.01);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1, result.dy, 0.01);
}

// A node dragged between two horizontal neighbours (overlapping its X range)
// with slightly unequal gaps (17 above, 13 below) snaps to the position that
// makes both gaps 15.
void SmartGuidesTest::EqualSpacingBetweenTwoNeighboursSnaps(void)
{
	BRect	above(10,-50,50,-10);
	BRect	below(10,40,50,80);
	BList	targets;
	targets.AddItem(&above);
	targets.AddItem(&below);
	// startFrame (0,0,40,20) offset by (20,7) -> candidate (20,7,60,27)
	GuideSnapResult	result	= ComputeGuideSnap(BRect(0,0,40,20), 20, 7, &targets, kThreshold);
	CPPUNIT_ASSERT(result.horizontal.active);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(5, result.horizontal.linePos, 0.01);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(5, result.dy, 0.01);
	CPPUNIT_ASSERT(!result.vertical.active);
}

// Same neighbours as above, but the raw position needs an 8-unit correction
// to equalize the gaps - outside threshold, so it must not snap.
void SmartGuidesTest::EqualSpacingOutsideThresholdIsIgnored(void)
{
	BRect	above(10,-50,50,-10);
	BRect	below(10,40,50,80);
	BList	targets;
	targets.AddItem(&above);
	targets.AddItem(&below);
	GuideSnapResult	result	= ComputeGuideSnap(BRect(0,0,40,20), 20, 13, &targets, kThreshold);
	CPPUNIT_ASSERT(!result.horizontal.active);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(13, result.dy, 0.01);
}

// A single target offset by (-3 on X, +3 on Y) from the dragged node
// produces a simultaneous column and row match, independently on each axis.
void SmartGuidesTest::BothAxesCanSnapAtOnce(void)
{
	BRect	target(97,203,147,253);
	BList	targets;
	targets.AddItem(&target);
	GuideSnapResult	result	= ComputeGuideSnap(BRect(0,0,50,50), 100, 200, &targets, kThreshold);
	CPPUNIT_ASSERT(result.horizontal.active);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(203, result.dy, 0.01);
	CPPUNIT_ASSERT(result.vertical.active);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(97, result.dx, 0.01);
}
