#include "Zoom.h"

#include <math.h>


namespace Zoom {

static const float	kSteps[]	= { 0.1, 0.25, 0.33, 0.5, 0.67, 0.75, 1.0,
	1.25, 1.5, 2.0, 3.0, 5.0, 10.0 };
// a step counts as reached within this, so 0.33 after Out() from 0.5
// isn't taken for "below 0.33"
static const float	kTolerance	= 0.005;


int32 CountSteps(void)
{
	return sizeof(kSteps) / sizeof(kSteps[0]);
}


float StepAt(int32 index)
{
	if (index < 0)
		return kSteps[0];
	if (index >= CountSteps())
		return kSteps[CountSteps() - 1];
	return kSteps[index];
}


float In(float current)
{
	for (int32 i = 0; i < CountSteps(); i++) {
		if (kSteps[i] > current + kTolerance)
			return kSteps[i];
	}
	return kSteps[CountSteps() - 1];
}


float Out(float current)
{
	for (int32 i = CountSteps() - 1; i >= 0; i--) {
		if (kSteps[i] < current - kTolerance)
			return kSteps[i];
	}
	return kSteps[0];
}


float Fit(BRect content, BRect visible, float margin)
{
	if (!content.IsValid() || (content.Width() <= 0) || (content.Height() <= 0))
		return 1.0;
	float	x		= (visible.Width() - 2 * margin) / content.Width();
	float	y		= (visible.Height() - 2 * margin) / content.Height();
	float	zoom	= x < y ? x : y;
	if (zoom < kSteps[0])
		zoom	= kSteps[0];
	if (zoom > kSteps[CountSteps() - 1])
		zoom	= kSteps[CountSteps() - 1];
	return zoom;
}


BString Label(float zoom)
{
	BString	label;
	label.SetToFormat("%d %%", (int)floorf(zoom * 100 + 0.5f));
	return label;
}

}
