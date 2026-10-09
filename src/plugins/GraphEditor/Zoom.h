#ifndef ZOOM_H
#define ZOOM_H

#include <Rect.h>
#include <String.h>

/** The GraphEditor's zoom steps and "fit to window". */
namespace Zoom {

int32	CountSteps(void);
float	StepAt(int32 index);
/** the next step above / below current */
float	In(float current);
float	Out(float current);
/** the largest step-free zoom showing all of content (document units)
 * inside visible (screen pixels) with margin pixels around it, within
 * the smallest and largest step */
float	Fit(BRect content, BRect visible, float margin);
/** "150 %" */
BString	Label(float zoom);

}

#endif
