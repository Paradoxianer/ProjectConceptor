#include "NoteLayoutTest.h"

#include "NoteLayout.h"


static BString LineText(const NoteLine &line)
{
	BString	text;
	for (uint32 i = 0; i < line.size(); i++) {
		if (i > 0)
			text << "|";
		text << line[i].text;
	}
	return text;
}


void NoteLayoutTest::PlainTextIsOneSpan(void)
{
	std::vector<NoteSpan>	spans;
	ParseNoteInline("just some words",0,&spans);
	CPPUNIT_ASSERT_EQUAL((size_t)1,spans.size());
	CPPUNIT_ASSERT(spans[0].text == "just some words");
	CPPUNIT_ASSERT_EQUAL((uint32)0,spans[0].style);
}


void NoteLayoutTest::BoldItalicAndEscapes(void)
{
	std::vector<NoteSpan>	spans;
	ParseNoteInline("a **bold** *it* and **b *bi* b** \\*no\\*",0,&spans);
	const char	*texts[]	= { "a ", "bold", " ", "it", " and ", "b ", "bi", " b", " *no*" };
	uint32		styles[]	= { 0, kNoteBold, 0, kNoteItalic, 0, kNoteBold,
		kNoteBold | kNoteItalic, kNoteBold, 0 };
	CPPUNIT_ASSERT_EQUAL((size_t)9,spans.size());
	for (uint32 i = 0; i < 9; i++) {
		CPPUNIT_ASSERT(spans[i].text == texts[i]);
		CPPUNIT_ASSERT_EQUAL(styles[i],spans[i].style);
	}
}


void NoteLayoutTest::LinksWithTextAndBare(void)
{
	std::vector<NoteSpan>	spans;
	ParseNoteInline("see [Haiku](https://haiku-os.org) or https://example.com.",0,&spans);
	CPPUNIT_ASSERT_EQUAL((size_t)5,spans.size());
	CPPUNIT_ASSERT(spans[1].text == "Haiku");
	CPPUNIT_ASSERT_EQUAL((uint32)kNoteLink,spans[1].style);
	CPPUNIT_ASSERT(spans[1].url == "https://haiku-os.org");
	CPPUNIT_ASSERT(spans[3].text == "https://example.com");
	CPPUNIT_ASSERT(spans[3].url == "https://example.com");
	CPPUNIT_ASSERT(spans[4].text == ".");
}


void NoteLayoutTest::LonelyStarsStayText(void)
{
	std::vector<NoteSpan>	spans;
	ParseNoteInline("2 * 3 = 6 and a* b",0,&spans);
	CPPUNIT_ASSERT_EQUAL((size_t)1,spans.size());
	CPPUNIT_ASSERT(spans[0].text == "2 * 3 = 6 and a* b");
}


void NoteLayoutTest::LinesBreakAtNewlinesAndWidth(void)
{
	NoteFonts	fonts(*be_plain_font);
	float		width	= be_plain_font->StringWidth("aaaa bbbb");
	std::vector<NoteLine>	lines;
	bool		cut		= true;
	LayoutNote("aaaa bbbb cccc\n\ndddd",fonts,width,12,&lines,&cut);
	CPPUNIT_ASSERT(!cut);
	CPPUNIT_ASSERT_EQUAL((size_t)4,lines.size());
	CPPUNIT_ASSERT(LineText(lines[0]) == "aaaa bbbb");
	CPPUNIT_ASSERT(LineText(lines[1]) == "cccc");
	CPPUNIT_ASSERT(lines[2].empty());
	CPPUNIT_ASSERT(LineText(lines[3]) == "dddd");
	for (uint32 i = 0; i < lines.size(); i++) {
		for (uint32 r = 0; r < lines[i].size(); r++)
			CPPUNIT_ASSERT(lines[i][r].x + lines[i][r].width <= width + 0.01f);
	}
}


void NoteLayoutTest::LongWordIsBroken(void)
{
	NoteFonts	fonts(*be_plain_font);
	float		width	= be_plain_font->StringWidth("abcd");
	std::vector<NoteLine>	lines;
	bool		cut		= false;
	LayoutNote("abcdefghij",fonts,width,12,&lines,&cut);
	CPPUNIT_ASSERT(lines.size() >= 3);
	BString	joined;
	for (uint32 i = 0; i < lines.size(); i++) {
		joined << LineText(lines[i]);
		CPPUNIT_ASSERT(be_plain_font->StringWidth(LineText(lines[i]).String()) <= width + 0.01f);
	}
	CPPUNIT_ASSERT(joined == "abcdefghij");
}


void NoteLayoutTest::ListItemsHangUnderTheirText(void)
{
	NoteFonts	fonts(*be_plain_font);
	float		width	= be_plain_font->StringWidth("• aaaa bbbb");
	std::vector<NoteLine>	lines;
	bool		cut		= false;
	LayoutNote("- aaaa bbbb cccc\n12. dddd",fonts,width,12,&lines,&cut);
	CPPUNIT_ASSERT_EQUAL((size_t)3,lines.size());
	CPPUNIT_ASSERT(LineText(lines[0]) == "•|aaaa bbbb");
	CPPUNIT_ASSERT(LineText(lines[1]) == "cccc");
	// the second line starts where the item's text did
	CPPUNIT_ASSERT_DOUBLES_EQUAL(lines[0][1].x,lines[1][0].x,0.01);
	CPPUNIT_ASSERT(LineText(lines[2]) == "12.|dddd");
}


void NoteLayoutTest::HeadingIsBold(void)
{
	NoteFonts	fonts(*be_plain_font);
	std::vector<NoteLine>	lines;
	bool		cut		= false;
	LayoutNote("## Title\n#hashtag",fonts,500,12,&lines,&cut);
	CPPUNIT_ASSERT_EQUAL((size_t)2,lines.size());
	CPPUNIT_ASSERT(LineText(lines[0]) == "Title");
	CPPUNIT_ASSERT_EQUAL((uint32)kNoteBold,lines[0][0].style);
	CPPUNIT_ASSERT(LineText(lines[1]) == "#hashtag");
	CPPUNIT_ASSERT_EQUAL((uint32)0,lines[1][0].style);
}


void NoteLayoutTest::TooManyLinesEndInEllipsis(void)
{
	NoteFonts	fonts(*be_plain_font);
	std::vector<NoteLine>	lines;
	bool		cut		= false;
	LayoutNote("1\n2\n3\n4\n5",fonts,500,3,&lines,&cut);
	CPPUNIT_ASSERT(cut);
	CPPUNIT_ASSERT_EQUAL((size_t)3,lines.size());
	CPPUNIT_ASSERT(LineText(lines[2]) == "3|…");
}
