#include "NodeHandlesTest.h"

#include "NodeHandles.h"

CPPUNIT_TEST_SUITE_REGISTRATION(NodeHandlesTest);

using namespace NodeHandles;

static const BRect	kFrame(100, 100, 300, 200);


void NodeHandlesTest::HandlesSitOnCornersAndEdgeMiddles(void)
{
	CPPUNIT_ASSERT(Position(kFrame, TOP_LEFT) == BPoint(100, 100));
	CPPUNIT_ASSERT(Position(kFrame, TOP) == BPoint(200, 100));
	CPPUNIT_ASSERT(Position(kFrame, TOP_RIGHT) == BPoint(300, 100));
	CPPUNIT_ASSERT(Position(kFrame, RIGHT) == BPoint(300, 150));
	CPPUNIT_ASSERT(Position(kFrame, BOTTOM_RIGHT) == BPoint(300, 200));
	CPPUNIT_ASSERT(Position(kFrame, BOTTOM) == BPoint(200, 200));
	CPPUNIT_ASSERT(Position(kFrame, BOTTOM_LEFT) == BPoint(100, 200));
	CPPUNIT_ASSERT(Position(kFrame, LEFT) == BPoint(100, 150));
}


void NodeHandlesTest::HitTestFindsTheNearestHandle(void)
{
	CPPUNIT_ASSERT_EQUAL((int32)BOTTOM_RIGHT, HandleAt(kFrame, BPoint(303, 197), 5));
	CPPUNIT_ASSERT_EQUAL((int32)TOP, HandleAt(kFrame, BPoint(201, 98), 5));
	CPPUNIT_ASSERT_EQUAL((int32)NONE, HandleAt(kFrame, BPoint(150, 150), 5));
	CPPUNIT_ASSERT_EQUAL((int32)NONE, HandleAt(kFrame, BPoint(310, 200), 5));
}


void NodeHandlesTest::EdgeHandleChangesOneAxis(void)
{
	CPPUNIT_ASSERT(Resized(kFrame, RIGHT, 40, 30, 70, 30) == BRect(100, 100, 340, 200));
	CPPUNIT_ASSERT(Resized(kFrame, TOP, 40, -30, 70, 30) == BRect(100, 70, 300, 200));
}


void NodeHandlesTest::TopLeftHandleKeepsTheOppositeCorner(void)
{
	CPPUNIT_ASSERT(Resized(kFrame, TOP_LEFT, -20, -10, 70, 30) == BRect(80, 90, 300, 200));
}


void NodeHandlesTest::ResizeStopsAtTheMinimumSize(void)
{
	CPPUNIT_ASSERT(Resized(kFrame, LEFT, 500, 0, 70, 30) == BRect(230, 100, 300, 200));
	CPPUNIT_ASSERT(Resized(kFrame, BOTTOM_RIGHT, -500, -500, 70, 30) == BRect(100, 100, 170, 130));
}


void NodeHandlesTest::SnapMovesOnlyTheDraggedEdges(void)
{
	BRect	frame(103, 107, 288, 211);
	CPPUNIT_ASSERT(Snapped(frame, BOTTOM_RIGHT, 50) == BRect(103, 107, 300, 200));
	CPPUNIT_ASSERT(Snapped(frame, LEFT, 50) == BRect(100, 107, 288, 211));
}
