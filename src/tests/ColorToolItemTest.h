#ifndef COLOR_TOOL_ITEM_TEST_H
#define COLOR_TOOL_ITEM_TEST_H

#include <cppunit/extensions/HelperMacros.h>

/** ColorToolItem's targets (fill, border, text): which field a picked
 * color goes to. The picker window itself is tested by hand. */
class ColorToolItemTest : public CppUnit::TestFixture
{
public:
	void WithoutTargetsTheFillIsColored(void);
	void ChosenTargetSurvivesARebuild(void);
	void ColorForFindsTheField(void);

	CPPUNIT_TEST_SUITE(ColorToolItemTest);
	CPPUNIT_TEST(WithoutTargetsTheFillIsColored);
	CPPUNIT_TEST(ChosenTargetSurvivesARebuild);
	CPPUNIT_TEST(ColorForFindsTheField);
	CPPUNIT_TEST_SUITE_END();
};

#endif
