#include "ContextBar.h"

#include <View.h>

#include "GraphStyle.h"

// inner padding of a button, gap to the anchor, padding of the bar
static const float	kButtonPadding	= 8;
static const float	kGap			= 10;
static const float	kBarPadding		= 3;


ContextBar::ContextBar(void)
	:
	fVisible(false),
	fHighlight(-1)
{
}


ContextBar::~ContextBar(void)
{
	for (size_t i = 0; i < fButtons.size(); i++)
		delete fButtons[i].message;
}


void ContextBar::AddButton(const char *label, BMessage *message)
{
	Button	button;
	button.label	= label;
	button.message	= message;
	fButtons.push_back(button);
}


void ContextBar::Layout(BRect anchor, BRect visible,
	const std::vector<float> &widths, float height)
{
	float	total	= kBarPadding;
	for (size_t i = 0; (i < fButtons.size()) && (i < widths.size()); i++)
		total	+= widths[i] + 2 * kButtonPadding;
	total	+= kBarPadding;
	float	barHeight	= height + kButtonPadding + 2 * kBarPadding;

	float	left	= (anchor.left + anchor.right) / 2 - total / 2;
	if (left + total > visible.right)
		left	= visible.right - total;
	if (left < visible.left)
		left	= visible.left;
	float	top		= anchor.top - kGap - barHeight;
	if (top < visible.top)
		top		= anchor.bottom + kGap;
	fFrame.Set(left, top, left + total, top + barHeight);

	float	x	= left + kBarPadding;
	for (size_t i = 0; (i < fButtons.size()) && (i < widths.size()); i++) {
		float	w	= widths[i] + 2 * kButtonPadding;
		fButtons[i].frame.Set(x, top + kBarPadding, x + w, fFrame.bottom - kBarPadding);
		x	+= w;
	}
	fVisible	= true;
}


BRect ContextBar::ButtonFrame(int32 index) const
{
	if ((index < 0) || (index >= (int32)fButtons.size()))
		return BRect();
	return fButtons[index].frame;
}


int32 ContextBar::ButtonAt(BPoint where) const
{
	if (!fVisible || !fFrame.Contains(where))
		return -1;
	for (size_t i = 0; i < fButtons.size(); i++) {
		if (fButtons[i].frame.Contains(where))
			return i;
	}
	return -1;
}


BMessage* ContextBar::MessageAt(int32 index) const
{
	if ((index < 0) || (index >= (int32)fButtons.size()))
		return NULL;
	return fButtons[index].message;
}


const char* ContextBar::LabelAt(int32 index) const
{
	if ((index < 0) || (index >= (int32)fButtons.size()))
		return NULL;
	return fButtons[index].label.String();
}


void ContextBar::Draw(BView *view, const GraphStyle &style) const
{
	if (!fVisible)
		return;
	view->SetDrawingMode(B_OP_ALPHA);
	view->SetHighColor(GraphColors::WithAlpha(style.shadow, style.shadow.alpha / 2));
	view->FillRoundRect(fFrame.OffsetByCopy(0, 2), 7, 7);
	view->SetHighColor(style.cardFill);
	view->FillRoundRect(fFrame, 7, 7);
	view->SetPenSize(1);
	view->SetHighColor(style.cardBorder);
	view->StrokeRoundRect(fFrame, 7, 7);
	font_height	fh;
	view->GetFontHeight(&fh);
	for (size_t i = 0; i < fButtons.size(); i++) {
		BRect	frame	= fButtons[i].frame;
		if ((int32)i == fHighlight) {
			view->SetHighColor(GraphColors::WithAlpha(style.accent, 40));
			view->FillRoundRect(frame, 5, 5);
		}
		view->SetHighColor(style.text);
		float	y	= (frame.top + frame.bottom) / 2 + (fh.ascent - fh.descent) / 2;
		view->DrawString(fButtons[i].label.String(), BPoint(frame.left + kButtonPadding, y));
	}
}
