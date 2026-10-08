#include "GraphStyleTest.h"

#include "GraphStyle.h"
#include "ShadowCache.h"

#include <vector>

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


void GraphStyleTest::ShadowBlurSpreadsAndKeepsItsMass(void)
{
	// a filled square in the middle of an empty mask, big enough to keep
	// a fully dark core under a radius-3 blur
	const int32		size	= 40;
	std::vector<uint8>	mask(size * size, 0);
	long			before	= 0;
	for (int32 y = 10; y < 30; y++) {
		for (int32 x = 10; x < 30; x++) {
			mask[y * size + x]	= 255;
			before += 255;
		}
	}
	ShadowCache::BoxBlur(&mask[0], size, size, 3);
	long	after	= 0;
	for (int32 i = 0; i < size * size; i++)
		after += mask[i];
	// the core stays dark, the old edge is half-tone, pixels outside the
	// square now carry some shadow, far away stays clear
	CPPUNIT_ASSERT(mask[20 * size + 20] > 250);
	CPPUNIT_ASSERT(mask[20 * size + 10] > 100 && mask[20 * size + 10] < 180);
	CPPUNIT_ASSERT(mask[20 * size + 7] > 30);
	CPPUNIT_ASSERT(mask[0] == 0);
	// blurring moves the shadow around, it doesn't add or lose much of it
	CPPUNIT_ASSERT(after > before * 0.9 && after < before * 1.02);
}
