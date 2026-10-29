#ifndef STATUS_BUTTON_H
#define STATUS_BUTTON_H

#include <interface/Control.h>
#include <support/String.h>

class BPopUpMenu;

/** A flat button for the status bar: plain text on the bar's background,
 * a button frame only while the pointer is over it. With a menu it pops
 * that menu up instead of sending its message.
 */
class StatusButton : public BControl
{
public:
							StatusButton(const char *name, const char *label,
								BMessage *message);
	virtual					~StatusButton();

			/** takes ownership; shown on click, a "▾" marks the button */
			void			SetMenu(BPopUpMenu *menu);
			BPopUpMenu*		Menu(void) const { return fMenu; }
			/** the button keeps the width of this text, so a changing
			 * label doesn't shift its neighbours */
			void			SetWidestLabel(const char *label);

	virtual	void			SetLabel(const char *label);
	virtual	void			Draw(BRect updateRect);
	virtual	void			MouseDown(BPoint where);
	virtual	void			MouseMoved(BPoint where, uint32 code,
								const BMessage *dragMessage);
	virtual	void			MouseUp(BPoint where);
	virtual	void			GetPreferredSize(float *width, float *height);

private:
			BString			_Text(const char *label) const;

			BPopUpMenu		*fMenu;
			BString			fWidestLabel;
			bool			fInside;
			bool			fPressed;
};

#endif
