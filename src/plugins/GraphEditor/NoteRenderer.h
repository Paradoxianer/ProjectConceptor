#ifndef NOTE_RENDERER_H
#define NOTE_RENDERER_H

#include <app/Message.h>
#include <interface/Font.h>
#include <support/String.h>

#include <vector>

#include "GraphEditor.h"
#include "NoteLayout.h"
#include "Renderer.h"

/** The value of a note attribute (#157): text over several lines, wrapped
 * to the row, edited in place, with the markup of NoteLayout. The row is
 * as tall as the text, up to kMaxNoteLines. A click on a link opens it,
 * anywhere else edits the text. */
class NoteRenderer: public Renderer
{

public:
							NoteRenderer(GraphEditor *parentEditor,const char *text,
								BRect noteRect,BMessage *message);
	virtual					~NoteRenderer();
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

			void			SetFont(const BFont &font);
			void			SetColors(rgb_color text, rgb_color muted);
			/** opens the text for typing */
			void			StartEditing(void);
			BCursorID		CursorAt(BPoint where);

protected:
			/** the address of the link under where, empty if none */
			BString			LinkAt(BPoint where);
		BMessage			*changeMessage;
		BRect				frame;
		BString				text;
		BFont				font;
		NoteFonts			fonts;
		float				lineHeight;
		float				ascent;
		rgb_color			textColor;
		rgb_color			mutedColor;
		std::vector<NoteLine>	lines;
		/** the text goes on beyond the last line shown */
		bool				cut;
};

const int32		kMaxNoteLines	= 12;

#endif
