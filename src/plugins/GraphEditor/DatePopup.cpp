#include "DatePopup.h"

#include <Catalog.h>
#include <interface/Button.h>
#include <interface/LayoutBuilder.h>
#include <interface/StringView.h>
#include <support/DateTime.h>

#include <CalendarView.h>

#include <stdio.h>
#include <string.h>

#include "ProjectConceptorDefs.h"
#include "ValueRenderer.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DatePopup"

using BPrivate::BCalendarView;

static const uint32	kDayPicked		= 'dpDP';
static const uint32	kMonthBack		= 'dpMB';
static const uint32	kMonthForward	= 'dpMF';
static const uint32	kToday			= 'dpTD';
static const uint32	kNoDate			= 'dpND';


DatePopup::DatePopup(BPoint where, const BString &isoDate,
		const BMessage &forChange, BMessenger forDocument)
	: BWindow(BRect(where,where + BPoint(10,10)),B_TRANSLATE("Date"),
		B_BORDERED_WINDOW_LOOK,B_FLOATING_APP_WINDOW_FEEL,
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS
			| B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE),
	change(forChange),
	document(forDocument)
{
	calendar	= new BCalendarView("calendar");
	calendar->SetSelectionMessage(new BMessage(kDayPicked));
	calendar->SetInvocationMessage(new BMessage(kDayPicked));
	int32	year = 0, month = 0, day = 0;
	BDate	date;
	if ((sscanf(isoDate.String(),"%" B_SCNd32 "-%" B_SCNd32 "-%" B_SCNd32,
			&year,&month,&day) == 3) && date.SetDate(year,month,day))
		calendar->SetDate(date);
	else
		calendar->SetDate(BDate::CurrentDate(B_LOCAL_TIME));

	monthLabel	= new BStringView("month","");
	monthLabel->SetAlignment(B_ALIGN_CENTER);
	monthLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED,B_SIZE_UNSET));
	BButton	*back		= new BButton("back","◀",new BMessage(kMonthBack));
	BButton	*forward	= new BButton("forward","▶",new BMessage(kMonthForward));
	float	height		= back->MinSize().height;
	back->SetExplicitSize(BSize(height,height));
	forward->SetExplicitSize(BSize(height,height));

	BLayoutBuilder::Group<>(this,B_VERTICAL,B_USE_HALF_ITEM_SPACING)
		.SetInsets(B_USE_HALF_ITEM_INSETS)
		.AddGroup(B_HORIZONTAL,0)
			.Add(back)
			.Add(monthLabel)
			.Add(forward)
		.End()
		.Add(calendar)
		.AddGroup(B_HORIZONTAL)
			.Add(new BButton("today",B_TRANSLATE("Today"),new BMessage(kToday)))
			.Add(new BButton("none",B_TRANSLATE("No date"),new BMessage(kNoDate)))
		.End();
	ShowMonth();
	calendar->MakeFocus(true);
	ResizeToPreferred();
}


void DatePopup::ShowMonth(void)
{
	BDate	date	= calendar->Date();
	BString	text(date.LongMonthName(date.Month()));
	text << " " << date.Year();
	monthLabel->SetText(text.String());
}


void DatePopup::Commit(const char *isoDate)
{
	BMessage	*command	= ValueRenderer::ChangeOf(change,P_C_ATTRIBUTE_VALUE,
		B_STRING_TYPE,isoDate,strlen(isoDate) + 1);
	document.SendMessage(command);
	delete command;
	PostMessage(B_QUIT_REQUESTED);
}


void DatePopup::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case kDayPicked:
		case kToday: {
			BDate	date	= (message->what == kToday)
				? BDate::CurrentDate(B_LOCAL_TIME) : calendar->Date();
			char	iso[16];
			snprintf(iso,sizeof(iso),"%04" B_PRId32 "-%02" B_PRId32 "-%02" B_PRId32,
				date.Year(),date.Month(),date.Day());
			Commit(iso);
			break;
		}
		case kNoDate:
			Commit("");
			break;
		case kMonthBack:
		case kMonthForward: {
			BDate	date	= calendar->Date();
			date.AddMonths((message->what == kMonthBack) ? -1 : 1);
			calendar->SetDate(date);
			ShowMonth();
			break;
		}
		default:
			BWindow::MessageReceived(message);
	}
}


void DatePopup::WindowActivated(bool active)
{
	BWindow::WindowActivated(active);
	if (!active)
		PostMessage(B_QUIT_REQUESTED);
}
