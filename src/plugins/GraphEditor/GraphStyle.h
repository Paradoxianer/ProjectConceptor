#ifndef GRAPH_STYLE_H
#define GRAPH_STYLE_H

#include <GraphicsDefs.h>

/** Colors and measures GraphEditor draws with: the "card" look of #145.
 * Everything derives from the system's document colors, its control
 * highlight color and the plain font size, so a dark color scheme and a
 * bigger system font carry over without settings of their own.
 */
struct GraphStyle {
	// canvas
	rgb_color	canvas;
	rgb_color	gridDot;

	// node card
	rgb_color	cardFill;
	rgb_color	cardBorder;
	rgb_color	text;
	rgb_color	mutedText;
	float		cornerRadius;
	float		stripeHeight;
	float		paddingX;
	float		paddingY;
	float		nameFontSize;
	float		attributeFontSize;
	float		rowSpacing;

	// soft shadow below a card: shadow is the color at its darkest point
	rgb_color	shadow;
	float		shadowBlur;
	float		shadowOffsetY;

	// selection outline, drawn selectionGap outside the card
	rgb_color	accent;
	float		selectionWidth;
	float		selectionGap;

	// connections
	rgb_color	connection;
	float		connectionWidth;

	// groups
	rgb_color	groupFill;
	rgb_color	groupBorder;
	rgb_color	groupLabel;
	float		groupCornerRadius;

	/** The card style for the given colors and font size. */
	static	GraphStyle	Card(rgb_color documentBackground,
							rgb_color documentText, rgb_color accent,
							float fontSize);
	/** Card() for the current system colors and be_plain_font. */
	static	GraphStyle	SystemCard(void);
};


/** Color arithmetic the style is built from. */
namespace GraphColors {

/** amount 0 gives from, 1 gives to; alpha is mixed too. */
rgb_color	Mix(rgb_color from, rgb_color to, float amount);
rgb_color	WithAlpha(rgb_color color, uint8 alpha);
/** Relative luminance 0..1 (sRGB, as in WCAG). */
float		Luminance(rgb_color color);
/** WCAG contrast ratio 1..21. */
float		Contrast(rgb_color a, rgb_color b);
bool		IsDark(rgb_color color);
/** color mixed toward background just as far as it keeps minRatio
 * contrast against it. */
rgb_color	Soften(rgb_color color, rgb_color background, float amount,
				float minRatio);

}

#endif
