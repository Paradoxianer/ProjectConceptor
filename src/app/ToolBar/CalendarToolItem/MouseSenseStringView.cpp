// Calendar Control version 2.5
// by Al.V. Sarikov.
// Kherson, Ukraine, 2006.
// E-mail: avix@ukrpost.net.
// Home page: http://avix.pp.ru.

// Control which allows to work with dates:
// enter date to text field and choose it from calendar.

// Distributed under BSD license (see LICENSE file).

#include <Message.h>
#include <Messenger.h>
#include <Point.h>
#include <StringView.h>

class MouseSenseStringView:public BStringView
{
 public:
  MouseSenseStringView(BMessage *msg,
                       BMessenger *msng,
                       BRect frame,
                       const char *name,
                       const char *text,
                       uint32 resizingMode=B_FOLLOW_LEFT|B_FOLLOW_TOP,
                       uint32 flags=B_WILL_DRAW);
  virtual void MouseDown(BPoint p);
 private:
  BMessage *msg;
  BMessenger *msng;
};

MouseSenseStringView::MouseSenseStringView(BMessage *msg,
                                           BMessenger *msng,
                                           BRect frame,
                                           const char *name,
                                           const char *text,
                                           uint32 resizingMode=B_FOLLOW_LEFT|B_FOLLOW_TOP,
                                           uint32 flags=B_WILL_DRAW)
                     :BStringView(frame,name,text,resizingMode,flags)
{
 this->msg=msg;
 this->msng=msng;
}


void MouseSenseStringView::MouseDown(BPoint p)
{
 if(msg!=NULL) if(msng!=NULL)
  msng->SendMessage(msg);
}
