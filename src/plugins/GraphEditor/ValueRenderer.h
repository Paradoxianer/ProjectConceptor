#ifndef VALUE_RENDERER_H
#define VALUE_RENDERER_H

#include <app/Message.h>
#include <interface/Font.h>
#include <support/String.h>

#include "GraphEditor.h"
#include "Renderer.h"

/** The value of a link, file, number, choice, color or date attribute
 * (#146), in one line. A click does what the kind is for: a link or file
 * opens, a choice shows its list, a color its picker, a date a calendar,
 * a number opens for typing. Edit() changes the value itself - for a link
 * or file the address, which a click on the text would open instead. */
class ValueRenderer: public Renderer
{

public:
							ValueRenderer(GraphEditor *parentEditor,
								const BMessage &attribute,BRect valueRect,
								BMessage *change);
	virtual					~ValueRenderer();
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
			BCursorID		CursorAt(BPoint where);

			void			SetColors(rgb_color text, rgb_color muted);
			/** changes the value - what the context menu's "Change…" does */
			void			Edit(void);
			/** whether Edit() does something a click doesn't */
			bool			HasEdit(void);

	/** a copy of change that sets field of the attribute to data */
	static	BMessage*		ChangeOf(const BMessage &change, const char *field,
								type_code type, const void *data, ssize_t size);
	/** the text of a link that opens it: a bare host gets https:// */
	static	BString			Address(const BString &link);

protected:
			/** the text shown, empty when there is no value yet */
			BString			DisplayText(void);
			BString			Placeholder(void);
			/** where the text is drawn, for clicks on a link or file */
			BRect			TextFrame(void);
			/** change, with the valueContainer naming field and type, for an
			 * editor that adds newValue itself */
			BMessage*		EditorMessage(const char *field, type_code type);
			/** a text editor over this row; message as for TextEditorControl */
			void			OpenTextEditor(const char *text, BMessage *message,
								bool singleLine);
			void			Open(void);
			void			ShowChoices(void);
			/** the row's left bottom corner on screen */
			BPoint			ScreenPoint(void);

		BMessage			*change;
		BMessage			attribute;
		uint32				kind;
		BString				value;
		BRect				frame;
		BFont				font;
		float				ascent;
		rgb_color			textColor;
		rgb_color			mutedColor;
};

#endif
