#ifndef VECTOR_ICON_H
#define VECTOR_ICON_H

#include <SupportDefs.h>

class BBitmap;

/** toolbar icons are drawn this many pixels square */
const float	kToolIconSize	= 20;

class BResources;

/** Renders the HVIF resource name ('VICN', see tools/icons) of res as a
 * size x size bitmap. NULL if there is no such icon. The caller owns the
 * bitmap. */
BBitmap*	LoadVectorIcon(BResources *res, const char *name, float size);

/** the same from the application's own resources */
BBitmap*	LoadAppVectorIcon(const char *name, float size);

#endif
