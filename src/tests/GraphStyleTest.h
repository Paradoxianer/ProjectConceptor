#ifndef GRAPH_STYLE_TEST_H
#define GRAPH_STYLE_TEST_H

#include <cppunit/extensions/HelperMacros.h>

/** GraphStyle (GraphEditor): the card look derived from system colors. */
class GraphStyleTest : public CppUnit::TestFixture
{
public:
	void MixReachesBothEnds(void);
	void ContrastOfBlackOnWhiteIs21(void);
	void LightSchemeCardsSitOnADarkerCanvas(void);
	void DarkSchemeLiftsTheCards(void);
	void TextStaysReadableInBothSchemes(void);
	void GroupLabelStaysReadableWithALightAccent(void);
	void SizesFollowTheFont(void);

	CPPUNIT_TEST_SUITE(GraphStyleTest);
	CPPUNIT_TEST(MixReachesBothEnds);
	CPPUNIT_TEST(ContrastOfBlackOnWhiteIs21);
	CPPUNIT_TEST(LightSchemeCardsSitOnADarkerCanvas);
	CPPUNIT_TEST(DarkSchemeLiftsTheCards);
	CPPUNIT_TEST(TextStaysReadableInBothSchemes);
	CPPUNIT_TEST(GroupLabelStaysReadableWithALightAccent);
	CPPUNIT_TEST(SizesFollowTheFont);
	CPPUNIT_TEST_SUITE_END();
};

#endif
