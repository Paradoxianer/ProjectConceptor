#ifndef NOTE_LAYOUT_TEST_H
#define NOTE_LAYOUT_TEST_H

#include <cppunit/extensions/HelperMacros.h>

/** NoteLayout (GraphEditor): markup of a note and how it is put in lines */
class NoteLayoutTest : public CppUnit::TestFixture
{
public:
	void PlainTextIsOneSpan(void);
	void BoldItalicAndEscapes(void);
	void LinksWithTextAndBare(void);
	void LonelyStarsStayText(void);
	void LinesBreakAtNewlinesAndWidth(void);
	void LongWordIsBroken(void);
	void ListItemsHangUnderTheirText(void);
	void HeadingIsBold(void);
	void TooManyLinesEndInEllipsis(void);

	CPPUNIT_TEST_SUITE(NoteLayoutTest);
	CPPUNIT_TEST(PlainTextIsOneSpan);
	CPPUNIT_TEST(BoldItalicAndEscapes);
	CPPUNIT_TEST(LinksWithTextAndBare);
	CPPUNIT_TEST(LonelyStarsStayText);
	CPPUNIT_TEST(LinesBreakAtNewlinesAndWidth);
	CPPUNIT_TEST(LongWordIsBroken);
	CPPUNIT_TEST(ListItemsHangUnderTheirText);
	CPPUNIT_TEST(HeadingIsBold);
	CPPUNIT_TEST(TooManyLinesEndInEllipsis);
	CPPUNIT_TEST_SUITE_END();
};

#endif
