#ifndef SHADOW_CACHE_H
#define SHADOW_CACHE_H

#include <GraphicsDefs.h>
#include <Rect.h>
#include <String.h>

#include <map>

class BBitmap;
class BView;
class NodeShape;

/** Soft drop shadows. The app_server can't blur, so a shadow is a node's
 * silhouette drawn once into a bitmap, blurred on the CPU and kept per
 * size and shape - the same idea as HVIF's gradient shadows, but for any
 * outline.
 */
class ShadowCache
{
public:
							ShadowCache(void);
							~ShadowCache(void);

	/** Draws the shadow of shape (laid out for frame, rounded rectangle
	 * with cornerRadius when it has no path) below frame. */
			void			Draw(BView *view, const NodeShape &shape,
								const char *shapeName, BRect frame,
								float cornerRadius, rgb_color color,
								float blur, float offsetY);
			void			Clear(void);

	/** The blurred mask itself, for tests: 0..255 per pixel, row by row,
	 * (width + 2 * margin) x (height + 2 * margin). */
	static	void			BoxBlur(uint8 *mask, int32 width, int32 height,
								int32 radius);

private:
			BBitmap*		Build(const NodeShape &shape, BRect frame,
								float cornerRadius, rgb_color color,
								int32 margin);

			std::map<BString, BBitmap*>	fShadows;
};

#endif
