#include "GraphStyleTest.h"

#include "GraphStyle.h"

using namespace GraphColors;

static const rgb_color	kWhite		= {255, 255, 255, 255};
static const rgb_color	kBlack		= {0, 0, 0, 255};
// Haiku's default B_CONTROL_HIGHLIGHT_COLOR, light and dark scheme
static const rgb_color	kAccent		= {102, 152, 203, 255};
static const rgb_color	kDarkAccent	= {75, 124, 168, 255};
static const rgb_color	kDarkBack	= {35, 35, 38, 255};
static const rgb_color	kDarkText	= {225, 225, 225, 255};


static bool SameColor(rgb_color a, rgb_color b)
{
	return (a.red == b.red) && (a.green == b.green) && (a.blue == b.blue)
		&& (a.alpha == b.alpha);
}


void GraphStyleTest::MixReachesBothEnds(void)
{
	CPPUNIT_ASSERT(SameColor(kWhite, Mix(kWhite, kBlack, 0)));
	CPPUNIT_ASSERT(SameColor(kBlack, Mix(kWhite, kBlack, 1)));
	rgb_color	middle	= Mix(kWhite, kBlack, 0.5f);
	CPPUNIT_ASSERT(middle.red >= 127 && middle.red <= 128);
	// out-of-range amounts are clamped
	CPPUNIT_ASSERT(SameColor(kBlack, Mix(kWhite, kBlack, 3)));
}


void GraphStyleTest::ContrastOfBlackOnWhiteIs21(void)
{
	CPPUNIT_ASSERT_DOUBLES_EQUAL(21.0, Contrast(kBlack, kWhite), 0.01);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, Contrast(kAccent, kAccent), 0.001);
}


void GraphStyleTest::LightSchemeCardsSitOnADarkerCanvas(void)
{
	GraphStyle	style	= GraphStyle::Card(kWhite, kBlack, kAccent, 12);
	CPPUNIT_ASSERT(SameColor(kWhite, style.cardFill));
	CPPUNIT_ASSERT(Luminance(style.canvas) < Luminance(style.cardFill));
	CPPUNIT_ASSERT(Luminance(style.cardBorder) < Luminance(style.cardFill));
	CPPUNIT_ASSERT(SameColor(kAccent, style.accent));
}


void GraphStyleTest::DarkSchemeLiftsTheCards(void)
{
	GraphStyle	light	= GraphStyle::Card(kWhite, kBlack, kAccent, 12);
	GraphStyle	dark	= GraphStyle::Card(kDarkBack, kDarkText, kDarkAccent, 12);
	CPPUNIT_ASSERT(Luminance(dark.cardFill) > Luminance(dark.canvas));
	// a shadow has to be stronger to show on a dark canvas
	CPPUNIT_ASSERT(dark.shadow.alpha > light.shadow.alpha);
}


void GraphStyleTest::TextStaysReadableInBothSchemes(void)
{
	GraphStyle	styles[]	= {
		GraphStyle::Card(kWhite, kBlack, kAccent, 12),
		GraphStyle::Card(kDarkBack, kDarkText, kDarkAccent, 12)
	};
	for (int32 i = 0; i < 2; i++) {
		const GraphStyle	&style	= styles[i];
		CPPUNIT_ASSERT(Contrast(style.text, style.cardFill) >= 7.0f);
		CPPUNIT_ASSERT(Contrast(style.mutedText, style.cardFill) >= 4.5f);
		// muted really is muted
		CPPUNIT_ASSERT(!SameColor(style.mutedText, style.text));
		CPPUNIT_ASSERT(Contrast(style.groupLabel, style.canvas) >= 4.5f);
	}
}


void GraphStyleTest::GroupLabelStaysReadableWithALightAccent(void)
{
	const rgb_color	yellow	= {255, 230, 0, 255};
	GraphStyle	style	= GraphStyle::Card(kWhite, kBlack, yellow, 12);
	CPPUNIT_ASSERT(Contrast(style.groupLabel, style.canvas) >= 4.5f);
}


void GraphStyleTest::SizesFollowTheFont(void)
{
	GraphStyle	normal	= GraphStyle::Card(kWhite, kBlack, kAccent, 12);
	GraphStyle	large	= GraphStyle::Card(kWhite, kBlack, kAccent, 18);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(8.0, normal.cornerRadius, 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(12.0, large.cornerRadius, 0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(19.0, large.nameFontSize, 0.001);
	CPPUNIT_ASSERT(large.paddingX > normal.paddingX);
	// line widths stay crisp, they don't scale
	CPPUNIT_ASSERT_DOUBLES_EQUAL(normal.selectionWidth, large.selectionWidth, 0.001);
}
