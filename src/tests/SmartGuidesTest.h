#ifndef SMART_GUIDES_TEST_H
#define SMART_GUIDES_TEST_H

#include <cppunit/extensions/HelperMacros.h>

/** ComputeGuideSnap() (SmartGuides.cpp) is the pure geometry behind #127's
 * alignment guides - exercised directly here rather than only through a
 * live drag, which needs a running GraphEditor.
 */
class SmartGuidesTest : public CppUnit::TestFixture
{
public:
	void NoTargetsProducesNoMatch(void);
	void OutsideThresholdProducesNoMatch(void);
	void TopEdgeAlignsToTopEdge(void);
	void LeftEdgeAlignsToRightEdge(void);
	void CentersAlign(void);
	void ClosestOfSeveralCandidatesWins(void);
	void EqualSpacingBetweenTwoNeighboursSnaps(void);
	void EqualSpacingOutsideThresholdIsIgnored(void);
	void BothAxesCanSnapAtOnce(void);

	CPPUNIT_TEST_SUITE(SmartGuidesTest);
	CPPUNIT_TEST(NoTargetsProducesNoMatch);
	CPPUNIT_TEST(OutsideThresholdProducesNoMatch);
	CPPUNIT_TEST(TopEdgeAlignsToTopEdge);
	CPPUNIT_TEST(LeftEdgeAlignsToRightEdge);
	CPPUNIT_TEST(CentersAlign);
	CPPUNIT_TEST(ClosestOfSeveralCandidatesWins);
	CPPUNIT_TEST(EqualSpacingBetweenTwoNeighboursSnaps);
	CPPUNIT_TEST(EqualSpacingOutsideThresholdIsIgnored);
	CPPUNIT_TEST(BothAxesCanSnapAtOnce);
	CPPUNIT_TEST_SUITE_END();
};

#endif
