#ifndef FONTPANEL_H_
#define FONTPANEL_H_

#include <View.h>
#include <Window.h>
#include <Button.h>
#include <ListView.h>
#include <ListItem.h>
#include <Font.h>
#include <Message.h>
#include <Messenger.h>

#include "FontDefs.h"


class FontWindow;


class FontPanel
{
public:
						FontPanel(BMessenger *target = NULL,
									BMessage *message = NULL,
									float size = 12,
									bool modal = false,
									bool hide_when_done = true);
	virtual				~FontPanel(void);
			void		SelectFont(const BFont &font);
			void		SelectFont(font_family family, font_style style,
									float size = 12);
			void		Show();
			void		Hide();
			bool		IsShowing(void) const;
			void		SetTarget(BMessenger msgr);
			void		SetMessage(BMessage *msg);
			BWindow *	Window(void) const;
			void		SetHideWhenDone(bool value);
			bool		HideWhenDone(void) const;
			void		SetFontSize(uint16 size);
private:
	FontWindow *fWindow;
	
};

#endif
