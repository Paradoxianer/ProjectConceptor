#ifndef STATUS_TEXTS_TEST_H
#define STATUS_TEXTS_TEST_H

#include <cppunit/extensions/HelperMacros.h>

/** StatusTexts (GraphEditor): what the status bar says. The tests run
 * without a catalog, so they see the English texts. */
class StatusTextsTest : public CppUnit::TestFixture
{
public:
	void NothingSelected(void);
	void CountsUseSingularAndPlural(void);
	void GeometryAndPointerAreRounded(void);

	CPPUNIT_TEST_SUITE(StatusTextsTest);
	CPPUNIT_TEST(NothingSelected);
	CPPUNIT_TEST(CountsUseSingularAndPlural);
	CPPUNIT_TEST(GeometryAndPointerAreRounded);
	CPPUNIT_TEST_SUITE_END();
};

#endif
