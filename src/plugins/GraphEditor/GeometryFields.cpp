#include "GeometryFields.h"

#include <Catalog.h>

#include <math.h>

#include "GraphEditor.h"
#include "StatusBar.h"
#include "StatusField.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "GraphEditor"

static const int32	kDigits		= 5;
static const float	kLabelGap	= 3;
// between the position and the size
static const float	kGroupGap	= 14;


GeometryFields::GeometryFields(const char *name)
	:
	BView(BRect(0, 0, 10, 10), name, B_FOLLOW_LEFT | B_FOLLOW_TOP,
		B_WILL_DRAW | B_FRAME_EVENTS)
{
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	SetLowUIColor(B_PANEL_BACKGROUND_COLOR);
	SetFont(be_plain_font);
	// the text before each field: "Position [x], [y]  Size [w] × [h]"
	fLabels[LEFT]	= B_TRANSLATE_COMMENT("Position", "status bar: before a node's x, y");
	fLabels[TOP]	= ",";
	fLabels[WIDTH]	= B_TRANSLATE_COMMENT("Size", "status bar: before a node's width × height");
	fLabels[HEIGHT]	= "×";
	const char	*names[FIELD_COUNT]	= { "left", "top", "width", "height" };
	const char	*tips[FIELD_COUNT]	= {
		B_TRANSLATE("Left edge of the selected nodes"),
		B_TRANSLATE("Top edge of the selected nodes"),
		B_TRANSLATE("Width of the selected nodes"),
		B_TRANSLATE("Height of the selected nodes")
	};
	for (int32 i = 0; i < FIELD_COUNT; i++) {
		BMessage	*message	= new BMessage(G_E_SET_GEOMETRY);
		message->AddInt32("field", i);
		fFields[i]	= new StatusField(names[i], message, kDigits);
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
	float	total		= 0;
	float	fieldHeight	= 0;
	for (int32 i = 0; i < FIELD_COUNT; i++) {
		float	fieldWidth;
		fFields[i]->GetPreferredSize(&fieldWidth, &fieldHeight);
		total	+= _GapBefore(i) + ceilf(StringWidth(fLabels[i])) + kLabelGap + fieldWidth;
	}
	if (width != NULL)
		*width	= total;
	if (height != NULL)
		*height	= fieldHeight;
}


void GeometryFields::FrameResized(float width, float height)
{
	_Layout();
}


float GeometryFields::_GapBefore(int32 field) const
{
	switch (field) {
		case TOP:		return 0;	// the comma right after the number
		case WIDTH:		return kGroupGap;
		case HEIGHT:	return 4;
	}
	return 0;
}


void GeometryFields::_Layout(void)
{
	float	x	= 0;
	for (int32 i = 0; i < FIELD_COUNT; i++) {
		float	fieldWidth;
		fFields[i]->GetPreferredSize(&fieldWidth, NULL);
		x	+= _GapBefore(i) + ceilf(StringWidth(fLabels[i])) + kLabelGap;
		fFields[i]->MoveTo(x, 0);
		fFields[i]->ResizeTo(fieldWidth, Bounds().Height());
		x	+= fieldWidth;
	}
	Invalidate();
	// a field grew or shrank: the slots after this one move along
	float	width;
	GetPreferredSize(&width, NULL);
	StatusBar	*bar	= dynamic_cast<StatusBar*>(Parent());
	if ((bar != NULL) && (width != Bounds().Width()))
		bar->LayoutSlots();
}


void GeometryFields::Draw(BRect updateRect)
{
	font_height	fh;
	GetFontHeight(&fh);
	// on the fields' baseline (StatusField::FrameResized())
	float	textHeight	= ceilf(fh.ascent + fh.descent);
	float	baseline	= floorf((Bounds().Height() - textHeight) / 2) + fh.ascent;
	SetHighColor(ui_color(B_PANEL_TEXT_COLOR));
	for (int32 i = 0; i < FIELD_COUNT; i++) {
		float	x	= fFields[i]->Frame().left - kLabelGap
			- ceilf(StringWidth(fLabels[i]));
		DrawString(fLabels[i], BPoint(x, baseline));
	}
}
