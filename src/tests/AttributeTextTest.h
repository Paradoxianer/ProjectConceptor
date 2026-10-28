#ifndef ATTRIBUTE_TEXT_TEST_H
#define ATTRIBUTE_TEXT_TEST_H

#include <cppunit/extensions/HelperMacros.h>

/** AttributeText (libProjectConceptor): attribute values as text and
 * numbers with units read from text (#146) */
class AttributeTextTest : public CppUnit::TestFixture
{
public:
	void ValueTextOfEveryKind(void);
	void NoValueIsAnError(void);
	void NumbersWithUnits(void);
	void SeparatorsDecideThePoint(void);
	void TextWithoutNumberIsRefused(void);

	CPPUNIT_TEST_SUITE(AttributeTextTest);
	CPPUNIT_TEST(ValueTextOfEveryKind);
	CPPUNIT_TEST(NoValueIsAnError);
	CPPUNIT_TEST(NumbersWithUnits);
	CPPUNIT_TEST(SeparatorsDecideThePoint);
	CPPUNIT_TEST(TextWithoutNumberIsRefused);
	CPPUNIT_TEST_SUITE_END();
};

#endif
