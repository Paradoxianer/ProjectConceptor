#include "GraphStyle.h"

#include <Font.h>
#include <InterfaceDefs.h>

#include <math.h>


namespace GraphColors {

rgb_color Mix(rgb_color from, rgb_color to, float amount)
{
	if (amount < 0)
		amount = 0;
	else if (amount > 1)
		amount = 1;
	rgb_color	mixed;
	mixed.red	= (uint8)(from.red + (to.red - from.red) * amount + 0.5f);
	mixed.green	= (uint8)(from.green + (to.green - from.green) * amount + 0.5f);
	mixed.blue	= (uint8)(from.blue + (to.blue - from.blue) * amount + 0.5f);
	mixed.alpha	= (uint8)(from.alpha + (to.alpha - from.alpha) * amount + 0.5f);
	return mixed;
}


rgb_color WithAlpha(rgb_color color, uint8 alpha)
{
	color.alpha	= alpha;
	return color;
}


static float Linear(uint8 channel)
{
	float	c	= channel / 255.0f;
	return (c <= 0.03928f) ? c / 12.92f : powf((c + 0.055f) / 1.055f, 2.4f);
}


float Luminance(rgb_color color)
{
	return 0.2126f * Linear(color.red) + 0.7152f * Linear(color.green)
		+ 0.0722f * Linear(color.blue);
}


float Contrast(rgb_color a, rgb_color b)
{
	float	la	= Luminance(a);
	float	lb	= Luminance(b);
	if (la < lb) {
		float	swap	= la;
		la	= lb;
		lb	= swap;
	}
	return (la + 0.05f) / (lb + 0.05f);
}


bool IsDark(rgb_color color)
{
	return Luminance(color) < 0.25f;
}


rgb_color Soften(rgb_color color, rgb_color background, float amount,
	float minRatio)
{
	for (; amount > 0; amount -= 0.02f) {
		rgb_color	softened	= Mix(color, background, amount);
		if (Contrast(softened, background) >= minRatio)
			return softened;
	}
	return color;
}

}	// namespace GraphColors


using namespace GraphColors;


GraphStyle GraphStyle::Card(rgb_color documentBackground,
	rgb_color documentText, rgb_color accent, float fontSize)
{
	const rgb_color	black	= {0, 0, 0, 255};
	const float		scale	= fontSize / 12.0f;
	const bool		dark	= IsDark(documentBackground);
	documentBackground.alpha	= 255;
	documentText.alpha			= 255;
	accent.alpha				= 255;

	GraphStyle	style;
	// cards sit on a slightly darker canvas; in a dark scheme the cards
	// are lifted toward the text color instead
	style.cardFill		= dark ? Mix(documentBackground, documentText, 0.07f)
		: documentBackground;
	style.canvas		= dark ? documentBackground
		: Mix(documentBackground, documentText, 0.035f);
	style.gridDot		= Mix(style.canvas, documentText, 0.16f);

	style.text			= documentText;
	style.mutedText		= Soften(documentText, style.cardFill, 0.5f, 4.5f);
	style.cardBorder	= Mix(style.cardFill, documentText, dark ? 0.2f : 0.13f);
	style.cornerRadius	= 8 * scale;
	style.stripeHeight	= 4 * scale;
	style.paddingX		= 12 * scale;
	style.paddingY		= 8 * scale;
	style.nameFontSize	= fontSize + 1;
	style.attributeFontSize	= fontSize;
	style.rowSpacing	= 4 * scale;

	style.shadow		= WithAlpha(black, dark ? 110 : 46);
	style.shadowBlur	= 8 * scale;
	style.shadowOffsetY	= 3 * scale;

	style.accent		= accent;
	style.selectionWidth	= 2;
	style.selectionGap	= 3 * scale;

	style.connection	= Mix(documentText, style.canvas, dark ? 0.35f : 0.45f);
	style.connectionWidth	= 1.5f;

	style.groupFill		= WithAlpha(accent, dark ? 28 : 18);
	style.groupBorder	= WithAlpha(accent, dark ? 70 : 46);
	// tinted toward the accent only as far as it stays readable
	float	towardAccent	= 0.4f;
	style.groupLabel	= Mix(documentText, accent, towardAccent);
	while ((towardAccent > 0) && (Contrast(style.groupLabel, style.canvas) < 4.5f)) {
		towardAccent		-= 0.05f;
		style.groupLabel	= Mix(documentText, accent, towardAccent);
	}
	style.groupCornerRadius	= 14 * scale;
	return style;
}


GraphStyle GraphStyle::SystemCard(void)
{
	return Card(ui_color(B_DOCUMENT_BACKGROUND_COLOR),
		ui_color(B_DOCUMENT_TEXT_COLOR), ui_color(B_CONTROL_HIGHLIGHT_COLOR),
		be_plain_font->Size());
}
