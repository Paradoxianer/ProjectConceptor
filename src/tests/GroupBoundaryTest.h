#ifndef GROUP_BOUNDARY_TEST_H
#define GROUP_BOUNDARY_TEST_H

#include <cppunit/extensions/HelperMacros.h>

/** ComputeGroupBoundary() (GroupBoundary.cpp) is the pure geometry behind a
 * group's outline (issue #38) - exercised directly here rather than only
 * through a live GroupRenderer::Draw(), which needs a running GraphEditor.
 */
class GroupBoundaryTest : public CppUnit::TestFixture
{
public:
	void OverlappingChildrenProduceNoSelfIntersection(void);
	void NonOverlappingChildrenGetACorridorNotABowtie(void);
	void ChainOfNonOverlappingChildrenAllGetCorridors(void);
	void ExistingFixtureShapeIsUnchanged(void);

	CPPUNIT_TEST_SUITE(GroupBoundaryTest);
	CPPUNIT_TEST(OverlappingChildrenProduceNoSelfIntersection);
	CPPUNIT_TEST(NonOverlappingChildrenGetACorridorNotABowtie);
	CPPUNIT_TEST(ChainOfNonOverlappingChildrenAllGetCorridors);
	CPPUNIT_TEST(ExistingFixtureShapeIsUnchanged);
	CPPUNIT_TEST_SUITE_END();
};

#endif
