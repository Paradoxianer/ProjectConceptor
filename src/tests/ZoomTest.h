#ifndef ZOOM_TEST_H
#define ZOOM_TEST_H

#include <cppunit/extensions/HelperMacros.h>

/** Zoom (GraphEditor): steps in and out, fit to window, the label. */
class ZoomTest : public CppUnit::TestFixture
{
public:
	void InAndOutMoveOneStep(void);
	void StepsStopAtTheEnds(void);
	void InBetweenGoesToTheNeighbours(void);
	void FitUsesTheTighterAxis(void);
	void FitStaysWithinTheSteps(void);
	void LabelIsRoundedPercent(void);

	CPPUNIT_TEST_SUITE(ZoomTest);
	CPPUNIT_TEST(InAndOutMoveOneStep);
	CPPUNIT_TEST(StepsStopAtTheEnds);
	CPPUNIT_TEST(InBetweenGoesToTheNeighbours);
	CPPUNIT_TEST(FitUsesTheTighterAxis);
	CPPUNIT_TEST(FitStaysWithinTheSteps);
	CPPUNIT_TEST(LabelIsRoundedPercent);
	CPPUNIT_TEST_SUITE_END();
};

#endif
