#include "ZoomTest.h"

#include "Zoom.h"

CPPUNIT_TEST_SUITE_REGISTRATION(ZoomTest);


void ZoomTest::InAndOutMoveOneStep(void)
{
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1.25, Zoom::In(1.0), 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.75, Zoom::Out(1.0), 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.33, Zoom::Out(0.5), 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.25, Zoom::Out(0.33), 0.001);
}


void ZoomTest::StepsStopAtTheEnds(void)
{
	CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, Zoom::In(10.0), 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1, Zoom::Out(0.1), 0.001);
}


void ZoomTest::InBetweenGoesToTheNeighbours(void)
{
	// after "fit" the zoom is rarely a step
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, Zoom::In(0.8), 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.75, Zoom::Out(0.8), 0.001);
}


void ZoomTest::FitUsesTheTighterAxis(void)
{
	// 1000 x 200 into 520 x 520 with 10 px margin: width limits to 0.5
	float	zoom	= Zoom::Fit(BRect(0, 0, 1000, 200), BRect(0, 0, 520, 520), 10);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, zoom, 0.001);
}


void ZoomTest::FitStaysWithinTheSteps(void)
{
	CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0,
		Zoom::Fit(BRect(0, 0, 10, 10), BRect(0, 0, 1000, 1000), 0), 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1,
		Zoom::Fit(BRect(0, 0, 100000, 100), BRect(0, 0, 500, 500), 0), 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0,
		Zoom::Fit(BRect(0, 0, -1, -1), BRect(0, 0, 500, 500), 0), 0.001);
}


void ZoomTest::LabelIsRoundedPercent(void)
{
	CPPUNIT_ASSERT(Zoom::Label(1.0) == "100 %");
	CPPUNIT_ASSERT(Zoom::Label(0.333) == "33 %");
	CPPUNIT_ASSERT(Zoom::Label(0.676) == "68 %");
}
