#ifndef CONTEXT_BAR_TEST_H
#define CONTEXT_BAR_TEST_H

#include <cppunit/extensions/HelperMacros.h>

/** ContextBar (GraphEditor): where the bar goes and which button a click
 * hits. Clicking it needs a mouse and is tested by hand. */
class ContextBarTest : public CppUnit::TestFixture
{
public:
	void SitsCenteredAboveTheSelection(void);
	void GoesBelowWithoutRoomAbove(void);
	void StaysInsideTheView(void);
	void HitTestFindsTheButton(void);
	void HiddenBarIsNeverHit(void);

	CPPUNIT_TEST_SUITE(ContextBarTest);
	CPPUNIT_TEST(SitsCenteredAboveTheSelection);
	CPPUNIT_TEST(GoesBelowWithoutRoomAbove);
	CPPUNIT_TEST(StaysInsideTheView);
	CPPUNIT_TEST(HitTestFindsTheButton);
	CPPUNIT_TEST(HiddenBarIsNeverHit);
	CPPUNIT_TEST_SUITE_END();
};

#endif
