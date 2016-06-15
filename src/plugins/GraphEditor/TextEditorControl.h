#ifndef TEXT_EDITOR_CONTROL_H
#define TEXT_EDITOR_CONTROL_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <support/Debug.h>

#include <app/Invoker.h>
#include <app/Message.h>
#include <interface/Rect.h>
#include <interface/TextView.h>


class TextEditorControl: public BTextView, public BInvoker
{

public:
						TextEditorControl(BRect rect, const char *name, BMessage *message, uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP, uint32 flags = B_WILL_DRAW | B_PULSE_NEEDED | B_NAVIGABLE);
	virtual void		AttachedToWindow(void);
	virtual void		DetachedFromWindow(void);

	virtual	void		MakeFocus(bool flag = true);
	virtual	void		MouseUp(BPoint point);
	virtual	void		MouseDown(BPoint point);
	virtual void		KeyDown(const char *bytes, int32 numBytes);
	virtual status_t	Invoke(BMessage *message = NULL);
protected:
			bool		singleLine;
			bool		changed;
			bool		commit;
private:
};
#endif
