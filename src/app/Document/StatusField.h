#ifndef STATUS_FIELD_H
#define STATUS_FIELD_H

#include <app/Invoker.h>
#include <interface/TextView.h>

/** A number the status bar shows as plain text, editable in place: a
 * frame appears while the pointer is over it or while it is edited. It is
 * as wide as its text; when that changes, its parent gets FrameResized()
 * to place it and its neighbours again.
 * Enter or leaving the field sends its message with the value as the
 * float "value"; Escape restores the shown value.
 */
class StatusField : public BTextView, public BInvoker
{
public:
							StatusField(const char *name, BMessage *message,
								int32 digits);

			/** shown unless the field is being edited */
			void			SetValue(float value);
			/** nothing shown: e.g. the selected nodes differ */
			void			SetMixed(void);

	virtual	void			Draw(BRect updateRect);
	virtual	void			KeyDown(const char *bytes, int32 numBytes);
	virtual	void			MakeFocus(bool focus = true);
	virtual	void			MouseMoved(BPoint where, uint32 code,
								const BMessage *dragMessage);
	virtual	void			GetPreferredSize(float *width, float *height);
	virtual	void			FrameResized(float width, float height);

protected:
	virtual	void			InsertText(const char *text, int32 length,
								int32 offset, const text_run_array *runs);
	virtual	void			DeleteText(int32 fromOffset, int32 toOffset);

private:
			void			_Show(const char *text);
			void			_Commit(void);
			void			_TextChanged(void);

			BString			fShown;
			int32			fDigits;
			bool			fInside;
};

#endif
