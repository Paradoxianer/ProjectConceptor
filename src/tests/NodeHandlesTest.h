#ifndef NODE_HANDLES_TEST_H
#define NODE_HANDLES_TEST_H

#include <cppunit/extensions/HelperMacros.h>

/** NodeHandles (GraphEditor): resize handle positions, hit test and the
 * frame a handle drag produces. The drag itself needs a mouse and is
 * tested by hand. */
class NodeHandlesTest : public CppUnit::TestFixture
{
public:
	void HandlesSitOnCornersAndEdgeMiddles(void);
	void HitTestFindsTheNearestHandle(void);
	void EdgeHandleChangesOneAxis(void);
	void TopLeftHandleKeepsTheOppositeCorner(void);
	void ResizeStopsAtTheMinimumSize(void);
	void SnapMovesOnlyTheDraggedEdges(void);

	CPPUNIT_TEST_SUITE(NodeHandlesTest);
	CPPUNIT_TEST(HandlesSitOnCornersAndEdgeMiddles);
	CPPUNIT_TEST(HitTestFindsTheNearestHandle);
	CPPUNIT_TEST(EdgeHandleChangesOneAxis);
	CPPUNIT_TEST(TopLeftHandleKeepsTheOppositeCorner);
	CPPUNIT_TEST(ResizeStopsAtTheMinimumSize);
	CPPUNIT_TEST(SnapMovesOnlyTheDraggedEdges);
	CPPUNIT_TEST_SUITE_END();
};

#endif
