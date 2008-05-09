// Calendar Control version 2.5
// by Al.V. Sarikov.
// Kherson, Ukraine, 2006.
// E-mail: avix@ukrpost.net.
// Home page: http://avix.pp.ru.

// Control which allows to work with dates:
// enter date to text field and choose it from calendar.

// Distributed under BSD license (see LICENSE file).

#define CC_VERSION "2.5"

#include <Control.h>
#include <PictureButton.h>
#include <TextView.h>

class DateTextView;

// Formats of date
#define CC_DD_MM_YYYY_FORMAT 0 // DD.MM.YYYY
#define CC_MM_DD_YYYY_FORMAT 1 // MM.DD.YYYY

// Full/short year
#define CC_FULL_YEAR 0  // DD.MM.YYYY
#define CC_SHORT_YEAR 8 // DD.MM.YY

// When to begin
#define CC_FULL_CENTURY 0  // first year is first year of century (01-00)
#define CC_HALF_CENTURY 16 // first year is 51th year of century (51-50)

// Dividers
#define CC_DOT_DIVIDER 0   // .
#define CC_SLASH_DIVIDER 1 // /
#define CC_MINUS_DIVIDER 2 // -
#define CC_ALL_DIVIDERS 3  // 2 bits, and some one bit is reserved

// Interfaces
#define CC_AUTO_INTERFACE 0x0  //
#define CC_BEOS_INTERFACE 0x8  //
#define CC_ZETA_INTERFACE 0x10 //
#define CC_ALL_INTERFACES 0x18 // 2 bits


class CalendarControl: public BControl
{
 public:
  CalendarControl(BPoint p,
                  const char* name,
                  int day=0,
                  int month=0,
                  int year=0,
                  uint32 flags=CC_DD_MM_YYYY_FORMAT | CC_FULL_YEAR,
                  uint32 look=CC_DOT_DIVIDER | CC_AUTO_INTERFACE);
  ~CalendarControl();
  virtual void AttachedToWindow(void);
  virtual void Draw(BRect r);
  virtual void KeyDown(const char *bytes, int32 numBytes);
  virtual void MakeFocus(bool focused=true);
  virtual void MessageReceived(BMessage *msg);
  virtual void SetEnabled(bool enabled);
  virtual void SetViewColor(rgb_color color);
  void SetViewColor(uchar red, uchar green, uchar blue, uchar alpha=255);
  virtual void WindowActivated(bool active);
  
  void GetDate(int *day, int *month, int *year);
  void SetDate(int day=0, int month=0, int year=0);
  void SetDate(const char *tdate);
  void GetYearRange(int *first_year, int *last_year);
  uint32 GetLook();
  void SetLook(uint32 look);
  uint32 GetFlags();
  void SetFlags(uint32 flags);
  const char* Text() const;
  BTextView *TextView(void) const;
  const char* Version();
 private:
  void MakeButton();
  
  DateTextView *dtvc;
  BPictureButton *pb;
  bool wc;
  BPoint LT;
  rgb_color view_color;
  uint32 interface; // BeOS or Zeta (for universal version)
  BString *version;
};
