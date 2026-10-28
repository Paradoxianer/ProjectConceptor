#ifndef IMAGE_RENDERER_H
#define IMAGE_RENDERER_H

#include <app/Message.h>
#include <interface/Bitmap.h>
#include <interface/Font.h>
#include <support/String.h>

#include "GraphEditor.h"
#include "Renderer.h"

/** The value of an image attribute (#146): the picture a path points to,
 * as wide as the row and at most kMaxImageHeight high. The document keeps
 * the path only, not the picture. A click opens the image; without one it
 * offers to choose a file, as Edit() always does. */
class ImageRenderer: public Renderer
{

public:
							ImageRenderer(GraphEditor *parentEditor,
								const char *path,BRect imageRect,BMessage *change);
	virtual					~ImageRenderer();
			void			ValueChanged(void){};
			void			Draw(BView *drawOn, BRect updateRect);
			void			MouseDown(BPoint where, int32 buttons =0,
									int32 clicks =0, int32 modifiers =0);
			void			MouseMoved(BPoint pt, uint32 code, const BMessage *msg){};
			void			MouseUp(BPoint where){};
			void			SetFrame(BRect newRect);
			BRect			Frame(void){return frame;};
			void			MoveBy(float dx, float dy){frame.OffsetBy(dx,dy);};
			void			ResizeBy(float dx,float dy){frame.right+=dx;SetFrame(frame);};
			bool			Selected(void){return false;};
			bool			Caught(BPoint where){return frame.Contains(where);};
			BCursorID		CursorAt(BPoint where){return B_CURSOR_ID_FOLLOW_LINK;};

			void			SetColors(rgb_color text, rgb_color muted);
			/** chooses another image */
			void			Edit(void);

protected:
		BMessage			*change;
		BString				path;
		/** owned by the cache, see ImageRenderer.cpp */
		BBitmap				*bitmap;
		BRect				frame;
		BFont				font;
		float				ascent;
		rgb_color			mutedColor;
};

const float		kMaxImageHeight	= 160;

#endif
