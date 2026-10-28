#ifndef DATE_POPUP_H
#define DATE_POPUP_H

#include <app/Messenger.h>
#include <interface/Window.h>
#include <support/String.h>

class BStringView;
namespace BPrivate {
	class BCalendarView;
}

/** A calendar under a date attribute (#143, #146). A click on a day sets
 * it and closes; the window also closes when it loses focus or on Esc.
 * The change goes to the document as the ChangeValue in change with the
 * date as YYYY-MM-DD. */
class DatePopup: public BWindow
{
public:
							DatePopup(BPoint where, const BString &isoDate,
								const BMessage &change, BMessenger document);
	virtual	void			MessageReceived(BMessage *message);
	virtual	void			WindowActivated(bool active);

private:
			void			ShowMonth(void);
			void			Commit(const char *isoDate);

		BMessage			change;
		BMessenger			document;
		BPrivate::BCalendarView	*calendar;
		BStringView			*monthLabel;
};

#endif
