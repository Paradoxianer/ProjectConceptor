#ifndef GEOMETRY_FIELDS_H
#define GEOMETRY_FIELDS_H

#include <interface/View.h>

#include <vector>

class StatusField;

/** X, Y, width and height of the selected nodes in the status bar (#144),
 * editable. A value differing between the selected nodes stays empty;
 * typing one sets it for all of them. Sends G_E_SET_GEOMETRY with the
 * int32 "field" (one of the Field values) and the float "value".
 */
class GeometryFields : public BView
{
public:
	enum Field { LEFT = 0, TOP, WIDTH, HEIGHT, FIELD_COUNT };

							GeometryFields(const char *name);

			void			SetTarget(BHandler *target);
			/** the selected nodes' frames as drawn */
			void			ShowFrames(const std::vector<BRect> &frames);
			/** a single node's frame, e.g. live while dragging it */
			void			ShowFrame(BRect frame);

	virtual	void			Draw(BRect updateRect);
	virtual	void			GetPreferredSize(float *width, float *height);
	virtual	void			FrameResized(float width, float height);

private:
			void			_Layout(void);
			float			_GapBefore(int32 field) const;

			StatusField		*fFields[FIELD_COUNT];
			const char		*fLabels[FIELD_COUNT];
};

#endif
