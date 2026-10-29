#include "GeometryFields.h"

#include <Catalog.h>

#include <math.h>

#include "GraphEditor.h"
#include "StatusField.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "GraphEditor"

static const int32	kDigits		= 5;
static const float	kLabelGap	= 3;
static const float	kFieldGap	= 8;


GeometryFields::GeometryFields(const char *name)
	:
	BView(BRect(0, 0, 10, 10), name, B_FOLLOW_LEFT | B_FOLLOW_TOP,
		B_WILL_DRAW | B_FRAME_EVENTS)
{
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	SetLowUIColor(B_PANEL_BACKGROUND_COLOR);
	SetFont(be_plain_font);
	// short: the status bar has little room
	fLabels[LEFT]	= B_TRANSLATE_COMMENT("X", "status bar: a node's left edge");
	fLabels[TOP]	= B_TRANSLATE_COMMENT("Y", "status bar: a node's top edge");
	fLabels[WIDTH]	= B_TRANSLATE_COMMENT("W", "status bar: a node's width");
	fLabels[HEIGHT]	= B_TRANSLATE_COMMENT("H", "status bar: a node's height");
	const char	*tips[FIELD_COUNT]	= {
		B_TRANSLATE("Left edge of the selected nodes"),
		B_TRANSLATE("Top edge of the selected nodes"),
		B_TRANSLATE("Width of the selected nodes"),
		B_TRANSLATE("Height of the selected nodes")
	};
	for (int32 i = 0; i < FIELD_COUNT; i++) {
		BMessage	*message	= new BMessage(G_E_SET_GEOMETRY);
		message->AddInt32("field", i);
		fFields[i]	= new StatusField(fLabels[i], message, kDigits);
		fFields[i]->SetToolTip(tips[i]);
		AddChild(fFields[i]);
	}
}


void GeometryFields::SetTarget(BHandler *target)
{
	for (int32 i = 0; i < FIELD_COUNT; i++)
		fFields[i]->SetTarget(target);
}


void GeometryFields::ShowFrame(BRect frame)
{
	fFields[LEFT]->SetValue(frame.left);
	fFields[TOP]->SetValue(frame.top);
	fFields[WIDTH]->SetValue(frame.Width());
	fFields[HEIGHT]->SetValue(frame.Height());
}


void GeometryFields::ShowFrames(const std::vector<BRect> &frames)
{
	float	values[FIELD_COUNT];
	bool	same[FIELD_COUNT]	= { true, true, true, true };
	for (size_t i = 0; i < frames.size(); i++) {
		float	these[FIELD_COUNT]	= { frames[i].left, frames[i].top,
			frames[i].Width(), frames[i].Height() };
		for (int32 f = 0; f < FIELD_COUNT; f++) {
			// compared as shown: whole numbers
			these[f]	= floorf(these[f] + 0.5f);
			if (i == 0)
				values[f]	= these[f];
			else if (values[f] != these[f])
				same[f]		= false;
		}
	}
	for (int32 f = 0; f < FIELD_COUNT; f++) {
		if (!frames.empty() && same[f])
			fFields[f]->SetValue(values[f]);
		else
			fFields[f]->SetMixed();
	}
}


void GeometryFields::GetPreferredSize(float *width, float *height)
{
	float	fieldWidth, fieldHeight;
	fFields[0]->GetPreferredSize(&fieldWidth, &fieldHeight);
	float	total	= 0;
	for (int32 i = 0; i < FIELD_COUNT; i++)
		total	+= ceilf(StringWidth(fLabels[i])) + kLabelGap + fieldWidth;
	total	+= kFieldGap * (FIELD_COUNT - 1);
	if (width != NULL)
		*width	= total;
	if (height != NULL)
		*height	= fieldHeight;
}


void GeometryFields::FrameResized(float width, float height)
{
	_Layout();
}


void GeometryFields::_Layout(void)
{
	float	fieldWidth, fieldHeight;
	fFields[0]->GetPreferredSize(&fieldWidth, &fieldHeight);
	float	x	= 0;
	for (int32 i = 0; i < FIELD_COUNT; i++) {
		x	+= ceilf(StringWidth(fLabels[i])) + kLabelGap;
		fFields[i]->MoveTo(x, 0);
		fFields[i]->ResizeTo(fieldWidth, Bounds().Height());
		x	+= fieldWidth + kFieldGap;
	}
	Invalidate();
}


void GeometryFields::Draw(BRect updateRect)
{
	font_height	fh;
	GetFontHeight(&fh);
	float	baseline	= floorf((Bounds().Height() + fh.ascent - fh.descent) / 2);
	// dimmed: halfway between text and background
	rgb_color	text		= ui_color(B_PANEL_TEXT_COLOR);
	rgb_color	background	= ViewColor();
	SetHighColor((text.red + background.red) / 2, (text.green + background.green) / 2,
		(text.blue + background.blue) / 2);
	for (int32 i = 0; i < FIELD_COUNT; i++) {
		float	x	= fFields[i]->Frame().left - kLabelGap
			- ceilf(StringWidth(fLabels[i]));
		DrawString(fLabels[i], BPoint(x, baseline));
	}
}
