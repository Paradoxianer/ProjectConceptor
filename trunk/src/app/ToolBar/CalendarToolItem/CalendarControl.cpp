// Calendar Control version 2.5
// by Al.V. Sarikov.
// Kherson, Ukraine, 2006.
// E-mail: avix@ukrpost.net.
// Home page: http://avix.pp.ru.

// Control which allows to work with dates:
// enter date to text field and choose it from calendar.

// Distributed under BSD license (see LICENSE file).


// -------------------------------------------------------------------------------
// --------------- Parameters to compile controls (see Compile.html) -------------
// -------------------------------------------------------------------------------

// ----------------------------------- Languages ---------------------------------

//#define __LANG_LOCALE  // to compile localized version (only for Zeta)
//#define __LANG_ENGLISH // to compile english version
//#define __LANG_RUSSIAN // to compile russian version
//#define __LANG_FRENCH  // to compile french version

// If nothing is set then english version (if to compile in BeOS)
// or localized version (if to compile in Zeta) is compiled.

// ---------------------------------- Interfaces ---------------------------------

//#define __UNIVERSAL_INTERFACE // to compile with Zeta and BeOS interface
//#define __ZETA_INTERFACE      // to compile only with Zeta interface
//#define __BEOS_INTERFACE      // to compile only with BeOS interface

// If nothing is set then __UNIVERSAL_INTERFACE will be set if to compile in BeOS.
// But while compiling in Zeta only __ZETA_INTERFACE is set no matter
// what interface developer have defined indeed.

// -------------------------------------------------------------------------------
// ---------------------------- End of parameters --------------------------------
// -------------------------------------------------------------------------------


// Choosing language of control
#ifdef __LANG_LOCALE
 #ifndef __ZETA__
  #undef __LANG_LOCALE
 #endif
#endif

#ifdef __LANG_LOCALE
 #undef __LANG_ENGLISH
 #undef __LANG_RUSSIAN
 #undef __LANG_FRENCH
#else
 #ifdef __LANG_ENGLISH
  #undef __LANG_RUSSIAN
  #undef __LANG_FRENCH
 #else
  #ifdef __LANG_RUSSIAN
   #undef __LANG_FRENCH
  #else
   #ifndef __LANG_FRENCH
    #ifdef __ZETA__
     #define __LANG_LOCALE // default while compiling in Zeta
    #else
     #define __LANG_ENGLISH // default while compiling in BeOS
    #endif
   #endif
  #endif
 #endif
#endif


// Choosing interface of control
#ifdef __ZETA__
 // compiling only Zeta interface
 #define __ZETA_INTERFACE
 #undef __BEOS_INTERFACE
 #undef __UNIVERSAL_INTERFACE
#else // compiling in BeOS
 #ifdef __UNIVERSAL_INTERFACE
  #define __ZETA_INTERFACE // these parameters must be set
  #define __BEOS_INTERFACE
 #else
  #ifdef __ZETA_INTERFACE
   #ifdef __BEOS_INTERFACE
    #define __UNIVERSAL_INTERFACE
   #endif
  #else
   #ifndef __BEOS_INTERFACE // nothing is set, so setting universal (default)
    #define __UNIVERSAL_INTERFACE
    #define __ZETA_INTERFACE
    #define __BEOS_INTERFACE
   #endif
  #endif
 #endif
#endif


#include "CalendarControl.h"

#define CalendarControlButtonPressedMessage 'DCBP'

#include "DateTextView.cpp"
#include "MonthWindow.cpp"

#include <AppFileInfo.h>
#include <FindDirectory.h>
#include <File.h>
#include <Path.h>
#include <Point.h>


//////////////////////////////////////////////////////////////////////////////////
CalendarControl::CalendarControl(BPoint p,
                                 const char* name,
                                 int day=0,
                                 int month=0,
                                 int year=0,
                                 uint32 flags=CC_DD_MM_YYYY_FORMAT | CC_FULL_YEAR,
                                 uint32 look=CC_DOT_DIVIDER | CC_AUTO_INTERFACE)
                :BControl(BRect(100,100,200,200),name,
                          NULL, NULL,
                          B_FOLLOW_LEFT | B_FOLLOW_TOP,
                          B_WILL_DRAW)
{
 version=NULL; // result will be set after first call of Version() function
               // to save memory; this function will not be used often.
 
 uint32 divider=look & CC_ALL_DIVIDERS;
 
#ifdef __UNIVERSAL_INTERFACE
 // Analysing interface flags. Advantage to universal interface.
 
 interface=look & CC_ALL_INTERFACES;
 if((interface!=CC_BEOS_INTERFACE) && (interface!=CC_ZETA_INTERFACE))
 {
  // CC_AUTO_INTERFACE or flag are not set correctly.
  // Analysing in which system program has started and setting interface flag.
  BPath path;
  if(find_directory(B_BEOS_LIB_DIRECTORY, &path)!=B_OK)
   interface=CC_BEOS_INTERFACE;
  else
  {
   path.Append("libbe.so");
   
   BString libbe_version;
   BFile libbe_file(path.Path(), B_READ_ONLY);
   BAppFileInfo info(&libbe_file);
   version_info version;
   info.GetVersionInfo (&version, B_SYSTEM_VERSION_KIND);
   libbe_version = version.short_info;
   
   char n[3];
   libbe_version.CopyInto(n,0,1);
   n[1]='\0';
   int i=atoi(n);
   if(i<6) // BeOS
    interface=CC_BEOS_INTERFACE;
   else // Zeta
    interface=CC_ZETA_INTERFACE;
  }
 }
#else // __UNIVERSAL_INTERFACE is not defined, so forced setting interface flag 
 #ifdef __ZETA_INTERFACE
  interface=CC_ZETA_INTERFACE;
 #else // __BEOS_INTERFACE and no other variants may be.
  interface=CC_BEOS_INTERFACE;
 #endif
#endif
 
 dtvc=new DateTextView(day,month,year,flags,divider|interface);
 BPicture *pic1=new BPicture();
 pb=new BPictureButton(BRect(70,0,85,15),"PictureButtonAViX",pic1,pic1,
                       new BMessage(CalendarControlButtonPressedMessage),
                       B_ONE_STATE_BUTTON,
                       B_FOLLOW_LEFT | B_FOLLOW_TOP,
                       B_WILL_DRAW);
 LT=p;
 
 AddChild(dtvc);
 dtvc->MoveTo(3,3);
 ResizeTo(dtvc->Bounds().Width()+6,dtvc->Bounds().Height()+7);
 
 AddChild(pb);
 delete pic1;
 
#ifdef __UNIVERSAL_INTERFACE
 if(interface==CC_ZETA_INTERFACE)
 {
#endif

#ifdef __ZETA_INTERFACE
  pb->ResizeTo(Bounds().Height()*0.7+1,Bounds().Height());
  pb->MoveTo(Bounds().right+2,Bounds().top);
  ResizeBy(pb->Bounds().Width()+2, 0);
#endif

#ifdef __UNIVERSAL_INTERFACE
 }
 else // interface==CC_BEOS_INTERFACE
 {
#endif

#ifdef __BEOS_INTERFACE
  pb->ResizeTo(Bounds().Height()*0.7,Bounds().Height()-1);
  pb->MoveTo(Bounds().right+1, Bounds().top);
  ResizeBy(pb->Bounds().Width()+1, 0);
#endif

#ifdef __UNIVERSAL_INTERFACE
 }
#endif
}


///////////////////////////////////
CalendarControl::~CalendarControl()
{
 if(version!=NULL) delete version;
 RemoveChild(dtvc);
 delete dtvc;
 RemoveChild(pb);
 delete pb;
}


////////////////////////////////////////////
void CalendarControl::AttachedToWindow(void)
{
 BControl::AttachedToWindow();
 
 pb->SetTarget(this);
 
 if(Parent()!=NULL) view_color=Parent()->ViewColor();
 else
  view_color.red=view_color.green=view_color.blue=view_color.alpha=255;
 
 SetViewColor(view_color); // function of CalendarControl class

#ifdef __UNIVERSAL_INTERFACE
 if(interface==CC_BEOS_INTERFACE)
#endif

#ifdef __BEOS_INTERFACE
  MakeButton(); // for BeOS interface is called only from here,
                // for Zeta interface if color is changed also (in SetViewColor)
#endif
 
 MoveTo(LT);
}


///////////////////////////////////
void CalendarControl::Draw(BRect r)
{
#ifdef __UNIVERSAL_INTERFACE
 if(interface==CC_ZETA_INTERFACE)
 {
#endif

#ifdef __ZETA_INTERFACE
 BRect rect1(Bounds());
 rect1.bottom--;
 rect1.right=pb->Frame().left-2;
 BRect rect2(rect1);
 BPoint p1, p2;
 rgb_color color;
 
 if(IsEnabled())
 {
  if(dtvc->IsFocus() && wc) // window is active and control is focused
                            // (control is enabled)
  {
   SetHighColor(170,50,184,0);
   StrokeRect(rect1);
   rect1.left++;
   rect1.top++;
   rect1.right--;
   rect1.bottom--;
   StrokeRect(rect1);
   SetHighColor(255,255,255,0);
   rect1.left++;
   rect1.top++;
   rect1.right--;
   rect1.bottom--;
   StrokeLine(rect1.LeftBottom(),rect1.RightBottom());
   StrokeLine(rect1.RightTop(),rect1.RightBottom());
   SetHighColor(218,218,218,0);
   StrokeLine(rect1.LeftTop(),rect1.LeftBottom());
   StrokeLine(rect1.LeftTop(),rect1.RightTop());
   SetHighColor(114,114,114,0);
   StrokeLine(rect1.LeftTop(),rect1.LeftTop());
   StrokeLine(rect1.LeftBottom(),rect1.LeftBottom());
   StrokeLine(rect1.RightTop(),rect1.RightTop());
   StrokeLine(rect1.RightBottom(),rect1.RightBottom());
   
   // outward borders
   color.red=170+(uint8)((229-170)*(view_color.red-170)/(255-170));
   color.green=50+(uint8)((194-50)*(view_color.green-50)/(255-50));
   color.blue=184+(uint8)((233-184)*(view_color.blue-184)/(255-184));
   color.alpha=255;
   
   SetHighColor(color);
   p1.Set(rect2.left,rect2.top+1);
   p2.Set(rect2.left+1,rect2.top);
   StrokeLine(p1,p2);
   p1.Set(rect2.right,rect2.top+1);
   p2.Set(rect2.right-1,rect2.top);
   StrokeLine(p1,p2);
   p1.Set(rect2.right,rect2.bottom-1);
   p2.Set(rect2.right-1,rect2.bottom);
   StrokeLine(p1,p2);
   p1.Set(rect2.left,rect2.bottom-1);
   p2.Set(rect2.left+1,rect2.bottom);
   StrokeLine(p1,p2);
   SetHighColor(view_color);
   StrokeLine(rect2.LeftTop(),rect2.LeftTop());
   StrokeLine(rect2.LeftBottom(),rect2.LeftBottom());
   StrokeLine(rect2.RightTop(),rect2.RightTop());
   StrokeLine(rect2.RightBottom(),rect2.RightBottom());
  }
  else // window is not active or control is unfocused (control is enabled)
  {
   SetHighColor(255,255,255,0);
   rect1.left++;
   rect1.top++;
   rect1.right--;
   rect1.bottom--;
   StrokeRect(rect1);
   rect1.left++;
   rect1.top++;
   rect1.right--;
   rect1.bottom--;
   StrokeRect(rect1);
   rect1=rect2;
   
   SetHighColor(0,0,0,0);
   StrokeRect(rect1);
   rect1.left++;
   rect1.top++;
   rect1.right--;
   rect1.bottom--;
   StrokeLine(rect1.LeftTop(),rect1.LeftTop());
   StrokeLine(rect1.LeftBottom(),rect1.LeftBottom());
   StrokeLine(rect1.RightTop(),rect1.RightTop());
   StrokeLine(rect1.RightBottom(),rect1.RightBottom());
   
   SetHighColor(180,180,180,0);
   p1.Set(rect2.left+2,rect2.top+1);
   p2.Set(rect2.right-2,rect2.top+1);
   StrokeLine(p1,p2);
   p1.Set(rect2.left+1,rect2.top+2);
   p2.Set(rect2.left+1,rect2.bottom-2);
   StrokeLine(p1,p2);
   p1.Set(rect2.left+2,rect2.top+2);
   StrokeLine(p1,p1);
   
   SetHighColor(218,218,218,0);
   p1.Set(rect2.left+2,rect2.top+3);
   p2.Set(rect2.left+2,rect2.bottom-1);
   StrokeLine(p1,p2);
   p1.Set(rect2.left+3,rect2.top+2);
   p2.Set(rect2.right-1,rect2.top+2);
   StrokeLine(p1,p2);
   
   // outward borders
   color.red=(uint8)(218*view_color.red/255);
   color.green=(uint8)(218*view_color.green/255);
   color.blue=(uint8)(218*view_color.blue/255);
   color.alpha=255;
   
   SetHighColor(color);
   p1.Set(rect2.left,rect2.top+1);
   p2.Set(rect2.left+1,rect2.top);
   StrokeLine(p1,p2);
   p1.Set(rect2.right,rect2.top+1);
   p2.Set(rect2.right-1,rect2.top);
   StrokeLine(p1,p2);
   p1.Set(rect2.right,rect2.bottom-1);
   p2.Set(rect2.right-1,rect2.bottom);
   StrokeLine(p1,p2);
   p1.Set(rect2.left,rect2.bottom-1);
   p2.Set(rect2.left+1,rect2.bottom);
   StrokeLine(p1,p2);
   SetHighColor(view_color);
   StrokeLine(rect2.LeftTop(),rect2.LeftTop());
   StrokeLine(rect2.LeftBottom(),rect2.LeftBottom());
   StrokeLine(rect2.RightTop(),rect2.RightTop());
   StrokeLine(rect2.RightBottom(),rect2.RightBottom());
  }
 }
 else // control is disabled
 {
  if(dtvc->IsFocus() && wc) // window is active and control is focused
                            // (control is disabled)
  {
   SetHighColor(255,255,255,0);
   rect1.left+=2;
   rect1.top+=2;
   rect1.right-=2;
   rect1.bottom-=2;
   StrokeRect(rect1);
   rect1=rect2;
   
   SetHighColor(231,198,235,0);
   StrokeRect(rect1);
   rect1.left++;
   rect1.top++;
   rect1.right--;
   rect1.bottom--;
   StrokeRect(rect1);
   rect1.left++;
   rect1.top++;
   rect1.right--;
   rect1.bottom--;
   StrokeLine(rect1.LeftTop(),rect1.LeftTop());
   StrokeLine(rect1.LeftBottom(),rect1.LeftBottom());
   StrokeLine(rect1.RightTop(),rect1.RightTop());
   StrokeLine(rect1.RightBottom(),rect1.RightBottom());
   
   // outward borders
   color.red=231+(uint8)((251-231)*(view_color.red-231)/(255-231));
   color.green=198+(uint8)((246-198)*(view_color.green-198)/(255-198));
   color.blue=235+(uint8)((251-235)*(view_color.blue-235)/(255-235));
   color.alpha=255;
   
   SetHighColor(color);
   p1.Set(rect2.left,rect2.top+1);
   p2.Set(rect2.left+1,rect2.top);
   StrokeLine(p1,p2);
   p1.Set(rect2.right,rect2.top+1);
   p2.Set(rect2.right-1,rect2.top);
   StrokeLine(p1,p2);
   p1.Set(rect2.right,rect2.bottom-1);
   p2.Set(rect2.right-1,rect2.bottom);
   StrokeLine(p1,p2);
   SetHighColor(view_color);
   p1.Set(rect2.left,rect2.bottom-1);
   p2.Set(rect2.left+1,rect2.bottom);
   StrokeLine(p1,p2);
   StrokeLine(rect2.LeftTop(),rect2.LeftTop());
   StrokeLine(rect2.LeftBottom(),rect2.LeftBottom());
   StrokeLine(rect2.RightTop(),rect2.RightTop());
   StrokeLine(rect2.RightBottom(),rect2.RightBottom());
  }
  else // window is not active or control is unfocused (control is disabled)
  {
   SetHighColor(255,255,255,0);
   rect1.left++;
   rect1.top++;
   rect1.right--;
   rect1.bottom--;
   StrokeRect(rect1);
   rect1.left++;
   rect1.top++;
   rect1.right--;
   rect1.bottom--;
   StrokeRect(rect1);
   rect1=rect2;
   
   SetHighColor(185,185,185,0);
   StrokeRect(rect1);
   rect1.left++;
   rect1.top++;
   rect1.right--;
   rect1.bottom--;
   StrokeLine(rect1.LeftTop(),rect1.LeftTop());
   StrokeLine(rect1.LeftBottom(),rect1.LeftBottom());
   StrokeLine(rect1.RightTop(),rect1.RightTop());
   StrokeLine(rect1.RightBottom(),rect1.RightBottom());
   
   // outward borders
   color.red=185+(uint8)((245-185)*(view_color.red-185)/(255-185));
   color.green=185+(uint8)((245-185)*(view_color.green-185)/(255-185));
   color.blue=185+(uint8)((245-185)*(view_color.blue-185)/(255-185));
   color.alpha=255;
   
   SetHighColor(color);
   p1.Set(rect2.left,rect2.top+1);
   p2.Set(rect2.left+1,rect2.top);
   StrokeLine(p1,p2);
   p1.Set(rect2.right,rect2.top+1);
   p2.Set(rect2.right-1,rect2.top);
   StrokeLine(p1,p2);
   p1.Set(rect2.right,rect2.bottom-1);
   p2.Set(rect2.right-1,rect2.bottom);
   StrokeLine(p1,p2);
   SetHighColor(view_color);
   p1.Set(rect2.left,rect2.bottom-1);
   p2.Set(rect2.left+1,rect2.bottom);
   StrokeLine(p1,p2);
   StrokeLine(rect2.LeftTop(),rect2.LeftTop());
   StrokeLine(rect2.LeftBottom(),rect2.LeftBottom());
   StrokeLine(rect2.RightTop(),rect2.RightTop());
   StrokeLine(rect2.RightBottom(),rect2.RightBottom());
  }
 }
#endif

#ifdef __UNIVERSAL_INTERFACE
 }
 else // interface==CC_BEOS_INTERFACE
 {
#endif

#ifdef __BEOS_INTERFACE
 BRect rect1(Bounds()),rect2(Bounds());
 rect1.bottom--;
 rect1.right=pb->Frame().left-1;
 rect2.top++;
 rect2.left++;
 rect2.bottom-=2;
 rect2.right=rect1.right-1;
 
 if(IsEnabled()) SetHighColor(255,255,255,255);
 else SetHighColor(239,239,239,255);

 StrokeLine(rect1.LeftBottom(),rect1.RightBottom());
 StrokeLine(rect1.RightTop(),rect1.RightBottom());
 
 if(IsEnabled()) SetHighColor(184,184,184,0);
 else SetHighColor(216,216,216,0);
 
 StrokeLine(rect1.LeftTop(),rect1.RightTop());
 StrokeLine(rect1.LeftTop(),rect1.LeftBottom());
 
 if(dtvc->IsFocus() && wc)
 {
  SetHighColor(0,0,229,0);
  StrokeRect(rect2);
 }
 else
 {
  SetHighColor(216,216,216,0);
  StrokeLine(rect2.LeftBottom(),rect2.RightBottom());
  StrokeLine(rect2.RightTop(),rect2.RightBottom());
  
  if(IsEnabled()) SetHighColor(96,96,96,0);
  else SetHighColor(152,152,152,0);
  
  StrokeLine(rect2.LeftTop(),rect2.RightTop());
  StrokeLine(rect2.LeftTop(),rect2.LeftBottom());
 }
 
 if(IsEnabled()) SetHighColor(255,255,255,255);
 else SetHighColor(239,239,239,255);
 
 rect2.top++;
 rect2.left++;
 rect2.right--;
 rect2.bottom--;
 StrokeRect(rect2);
#endif 

#ifdef __UNIVERSAL_INTERFACE
 }
#endif
}


////////////////////////////////////////////////////////////////
void CalendarControl::KeyDown(const char *bytes, int32 numBytes)
{
 BControl::KeyDown(bytes, numBytes);
 if(bytes[0]==B_TAB) Draw(Bounds());
}


//////////////////////////////////////////////////
void CalendarControl::MakeFocus(bool focused=true)
{
 dtvc->MakeFocus(focused);
}


////////////////////////////////////////////////////
void CalendarControl::MessageReceived(BMessage *msg)
{
 switch(msg->what)
 {
  case CalendarControlButtonPressedMessage:
  {
   if(IsEnabled())
   {
    MakeFocus(true);
    int day, month, year;
    int first_year, last_year;
    GetDate(&day, &month, &year);
    GetYearRange(&first_year, &last_year);
    new MonthWindow(ConvertToScreen(BPoint(Bounds().left+1,Bounds().bottom+1)),
                                    new BMessenger(this),
                                    day, month, year, first_year, last_year);
   }
   break;
  }
  case 'MVME': // message has come from window with calendar
  {
   int32 day, month, year;
   msg->FindInt32("day",&day);
   msg->FindInt32("month",&month);
   msg->FindInt32("year",&year);
   SetDate((int)day, (int)month, (int)year);
   break;
  }
  default:
   BControl::MessageReceived(msg);
 }
}


//////////////////////////////////////////////
void CalendarControl::SetEnabled(bool enabled)
{
 if(enabled==IsEnabled()) return;
 BControl::SetEnabled(enabled);
 pb->SetEnabled(enabled);
 dtvc->SetEnabled(enabled);
 Invalidate();
}


///////////////////////////////////////////////////
void CalendarControl::SetViewColor(rgb_color color)
{
 view_color=color;
 BControl::SetViewColor(view_color);
 
#ifdef __UNIVERSAL_INTERFACE
 if(interface==CC_ZETA_INTERFACE)
#endif 

#ifdef __ZETA_INTERFACE
  MakeButton();
#endif 
 Draw(Bounds());
 Invalidate();
 return;
}


///////////////////////////////////////////////////////////////
void CalendarControl::SetViewColor(uchar red, uchar green,
                                   uchar blue, uchar alpha=255)
{
 rgb_color color={red, green, blue, alpha};
 SetViewColor(color);
 return;
}


//////////////////////////////////////////////////
void CalendarControl::WindowActivated(bool active)
{
 wc=active; // true if window where control is placed is active
 Draw(Bounds());
}


/////////////////////////////////////////
const char* CalendarControl::Text() const
{
 return dtvc->Text();
}


//////////////////////////////////////////////////////////////
void CalendarControl::GetDate(int *day, int *month, int *year)
{
 dtvc->GetDate(day,month,year);
}


/////////////////////////////////////////////////////////////////
void CalendarControl::SetDate(int day=0, int month=0, int year=0)
{
 dtvc->SetDate(day,month,year);
}


////////////////////////////////////////////////
void CalendarControl::SetDate(const char *tdate)
{
 dtvc->SetDate(tdate);
}


///////////////////////////////////////////////////////////////////
void CalendarControl::GetYearRange(int *first_year, int *last_year)
{
 dtvc->GetYearRange(first_year, last_year);
}


/////////////////////////////////
uint32 CalendarControl::GetLook()
{
 return (dtvc->GetDivider() | interface);
}


//////////////////////////////////////////
void CalendarControl::SetLook(uint32 look)
{
 dtvc->SetDivider(look & CC_ALL_DIVIDERS);
}


//////////////////////////////////
uint32 CalendarControl::GetFlags()
{
 return dtvc->GetDateFlags();
}


////////////////////////////////////////////
void CalendarControl::SetFlags(uint32 flags)
{
 dtvc->SetDateFlags(flags);
}


////////////////////////////////////////////////
BTextView *CalendarControl::TextView(void) const
{
 return (BTextView *)dtvc;
}


//////////////////////////////////////
void CalendarControl::MakeButton(void)
{
 // Creates and sets buttons for control
 BPicture *pic1, *pic2, *pic3;
 
#ifdef __UNIVERSAL_INTERFACE
 if(interface==CC_ZETA_INTERFACE)
 {
#endif 

#ifdef __ZETA_INTERFACE
 rgb_color color;
 
 pic1=new BPicture();
 pb->BeginPicture(pic1);
 BRect r(pb->Bounds());
 pb->SetHighColor(245,245,245,0);
 pb->FillRect(r);
 
 pb->SetHighColor(view_color);
 pb->StrokeLine(r.LeftTop(),r.LeftTop());
 BPoint p1(r.left, r.bottom-2);
 BPoint p2(r.left+2, r.bottom);
 BRect r1(p1,p2);
 pb->FillRect(r1);
 p1.Set(r.right-2, r.top);
 p2.Set(r.right, r.top+2);
 r1.Set(p1.x,p1.y,p2.x,p2.y);
 pb->FillRect(r1);
 p1.Set(r.right-2, r.bottom-2);
 p2.Set(r.right, r.bottom);
 r1.Set(p1.x,p1.y,p2.x,p2.y);
 pb->FillRect(r1);
 
 pb->SetHighColor(0,0,0,0);
 p1.Set(r.left+2, r.top);
 p2.Set(r.right-4, r.top);
 pb->StrokeLine(p1,p2);
 p1.Set(r.left+1, r.top+1);
 pb->StrokeLine(p1,p1);
 p1.Set(r.left, r.top+2);
 p2.Set(r.left, r.bottom-4);
 pb->StrokeLine(p1,p2);
 p1.Set(r.left+1, r.bottom-3);
 pb->StrokeLine(p1,p1);
 p1.Set(r.left+2, r.bottom-2);
 p2.Set(r.right-4, r.bottom-2);
 pb->StrokeLine(p1,p2);
 p1.Set(r.right-3, r.bottom-3);
 pb->StrokeLine(p1,p1);
 p1.Set(r.right-2, r.top+2);
 p2.Set(r.right-2, r.bottom-4);
 pb->StrokeLine(p1,p2);
 p1.Set(r.right-3, r.top+1);
 pb->StrokeLine(p1,p1);
 
 color.red=(uint8)(218*view_color.red/255);
 color.green=(uint8)(218*view_color.green/255);
 color.blue=(uint8)(218*view_color.blue/255);
 color.alpha=255;
 
 pb->SetHighColor(color);
 p1.Set(r.left+1, r.top);
 p2.Set(r.left, r.top+1);
 pb->StrokeLine(p1,p2);
 p1.Set(r.left+2, r.bottom-1);
 pb->StrokeLine(p1,p1);
 p1.Set(r.left+3, r.bottom);
 p2.Set(r.right-3, r.bottom);
 pb->StrokeLine(p1,p2);
 p1.Set(r.right, r.bottom-3);
 pb->StrokeLine(p1,p2);
 p2.Set(r.right, r.top+3);
 pb->StrokeLine(p1,p2);
 p1.Set(r.right-1, r.top+2);
 pb->StrokeLine(p1,p2);
 
 color.red=(uint8)(186*view_color.red/255);
 color.green=(uint8)(186*view_color.green/255);
 color.blue=(uint8)(186*view_color.blue/255);
 color.alpha=255;
 
 pb->SetHighColor(color);
 p1.Set(r.left, r.bottom-3);
 p2.Set(r.left+1, r.bottom-2);
 pb->StrokeLine(p1,p2);
 p1.Set(r.right-3, r.top);
 p2.Set(r.right-2, r.top+1);
 pb->StrokeLine(p1,p2);
 
 color.red=(uint8)(154*view_color.red/255);
 color.green=(uint8)(154*view_color.green/255);
 color.blue=(uint8)(154*view_color.blue/255);
 color.alpha=255;
 
 pb->SetHighColor(color);
 p1.Set(r.right-2, r.bottom-3);
 p2.Set(r.right-3, r.bottom-2);
 pb->StrokeLine(p1,p2);
 
 color.red=(uint8)(180*view_color.red/255);
 color.green=(uint8)(180*view_color.green/255);
 color.blue=(uint8)(180*view_color.blue/255);
 color.alpha=255;
 
 pb->SetHighColor(color);
 p1.Set(r.left+3, r.bottom-1);
 p2.Set(r.right-3, r.bottom-1);
 pb->StrokeLine(p1,p2);
 p1.Set(r.right-1, r.bottom-3);
 pb->StrokeLine(p1,p2);
 p2.Set(r.right-1, r.top+3);
 pb->StrokeLine(p1,p2);
 pic1=pb->EndPicture();
 
 
 // Second button
 pic2=new BPicture();
 pb->BeginPicture(pic2);
 r=(pb->Bounds());
 pb->SetHighColor(view_color);
 pb->FillRect(r);
 
 r.left++;
 r.top++;
 r.right-=3;
 r.bottom-=3;
 pb->SetHighColor(245,245,245,0);
 pb->FillRect(r);
 
 pb->SetHighColor(0,0,0,0);
 pb->StrokeLine(r.LeftTop(),r.LeftTop());
 pb->StrokeLine(r.RightTop(),r.RightTop());
 pb->StrokeLine(r.LeftBottom(),r.LeftBottom());
 pb->StrokeLine(r.RightBottom(),r.RightBottom());
 
 r.left--;
 r.top--;
 r.right++;
 r.bottom++;
 pb->StrokeRect(r);
 
 // outward borders
 pb->SetHighColor(view_color);
 pb->StrokeLine(r.LeftTop(),r.LeftTop());
 pb->StrokeLine(r.RightTop(),r.RightTop());
 pb->StrokeLine(r.LeftBottom(),r.LeftBottom());
 pb->StrokeLine(r.RightBottom(),r.RightBottom());
 
 color.red=(uint8)(218*view_color.red/255);
 color.green=(uint8)(218*view_color.green/255);
 color.blue=(uint8)(218*view_color.blue/255);
 color.alpha=255;
 
 pb->SetHighColor(color);
 p1.Set(r.left+1, r.top);
 p2.Set(r.left, r.top+1);
 pb->StrokeLine(p1,p2);
 p1.Set(r.right-1, r.top);
 p2.Set(r.right, r.top+1);
 pb->StrokeLine(p1,p2);
 p1.Set(r.left+1, r.bottom);
 p2.Set(r.left, r.bottom-1);
 pb->StrokeLine(p1,p2);
 p1.Set(r.right-1, r.bottom);
 p2.Set(r.right, r.bottom-1);
 pb->StrokeLine(p1,p2);
 
 // inside
 r.left+=2;
 r.top+=2;
 r.right--;
 r.bottom--;
 pb->SetHighColor(210,210,210,0);
 pb->StrokeLine(r.LeftTop(),r.LeftBottom());
 pb->StrokeLine(r.LeftTop(),r.RightTop());
 
 pb->SetHighColor(173,173,173,0);
 pb->StrokeLine(r.LeftTop(),r.LeftTop());
 r.top--;
 r.right--;
 pb->StrokeLine(r.LeftTop(),r.RightTop());
 r.top++;
 r.left--;
 r.bottom--;
 pb->StrokeLine(r.LeftTop(),r.LeftBottom());
 pic2=pb->EndPicture();
 
 // Third button
 pic3=new BPicture();
 pb->BeginPicture(pic3);
 r=(pb->Bounds());
 pb->SetHighColor(view_color);
 pb->FillRect(r);
 
 r.left++;
 r.top++;
 r.right-=3;
 r.bottom-=3;
 pb->SetHighColor(251,251,251,255);
 pb->FillRect(r);
 
 pb->SetHighColor(185,185,185,255); // original border of inactive button is 182.
                                    // But I have 185 because border of inactive
                                    // TextView is 185 (and in system also).
 pb->StrokeLine(r.LeftTop(),r.LeftTop());
 pb->StrokeLine(r.RightTop(),r.RightTop());
 pb->StrokeLine(r.LeftBottom(),r.LeftBottom());
 pb->StrokeLine(r.RightBottom(),r.RightBottom());
 
 r.left--;
 r.top--;
 r.right++;
 r.bottom++;
 pb->StrokeRect(r);
 
 color.red=185+(uint8)((245-185)*(view_color.red-185)/(255-185));
 color.green=185+(uint8)((245-185)*(view_color.green-185)/(255-185));
 color.blue=185+(uint8)((245-185)*(view_color.blue-185)/(255-185));
 color.alpha=255;
 
 pb->SetHighColor(color);
 p1.Set(r.left+1, r.top);
 p2.Set(r.left, r.top+1);
 pb->StrokeLine(p1,p2);
 p1.Set(r.right-1, r.top);
 p2.Set(r.right, r.top+1);
 pb->StrokeLine(p1,p2);
 p1.Set(r.right-1, r.bottom);
 p2.Set(r.right, r.bottom-1);
 pb->StrokeLine(p1,p2);
 
 pb->SetHighColor(view_color);
 p1.Set(r.left+1, r.bottom);
 p2.Set(r.left, r.bottom-1);
 pb->StrokeLine(p1,p2);
 
 pb->StrokeLine(r.LeftTop(),r.LeftTop());
 pb->StrokeLine(r.RightTop(),r.RightTop());
 pb->StrokeLine(r.LeftBottom(),r.LeftBottom());
 pb->StrokeLine(r.RightBottom(),r.RightBottom());
 
 pic3=pb->EndPicture();
#endif 

#ifdef __UNIVERSAL_INTERFACE
 }
 else // interface==CC_BEOS_INTERFACE
 {
#endif 

#ifdef __BEOS_INTERFACE
 pic1=new BPicture();
 pb->BeginPicture(pic1);
 BRect r(pb->Bounds());
 pb->SetHighColor(96,96,96,0);
 pb->StrokeRect(r);
 pb->SetHighColor(216,216,216,0);
 pb->StrokeLine(r.LeftBottom(),r.LeftBottom());
 pb->StrokeLine(r.RightBottom(),r.RightBottom());
 pb->StrokeLine(r.LeftTop(),r.LeftTop());
 pb->StrokeLine(r.RightTop(),r.RightTop());
 r.top++;
 r.left++;
 r.bottom--;
 r.right--;
 pb->SetHighColor(152,152,152,0);
 pb->StrokeLine(r.LeftBottom(),r.RightBottom());
 pb->StrokeLine(r.RightTop(),r.RightBottom());
 pb->SetHighColor(232,232,232,0);
 pb->StrokeLine(r.LeftTop(),r.RightTop());
 pb->StrokeLine(r.LeftTop(),r.LeftBottom());
 r.top++;
 r.left++;
 r.bottom--;
 r.right--;
 pb->SetHighColor(216,216,216,0);
 pb->StrokeLine(r.LeftBottom(),r.RightBottom());
 pb->StrokeLine(r.RightTop(),r.RightBottom());
 pb->SetHighColor(255,255,255,0);
 pb->StrokeLine(r.LeftTop(),r.RightTop());
 pb->StrokeLine(r.LeftTop(),r.LeftBottom());
 r.top++;
 r.left++;
 r.bottom--;
 r.right--;
 pb->SetHighColor(232,232,232,0);
 pb->FillRect(r);
 pb->SetHighColor(255,255,255,0);
 pb->StrokeLine(r.LeftTop(),r.RightTop());
 pb->StrokeLine(r.LeftTop(),r.LeftBottom());
 pic1=pb->EndPicture();
 
 
 // Second button
 pic2=new BPicture();
 pb->BeginPicture(pic2);
 r=(pb->Bounds());
 pb->SetHighColor(96,96,96,0);
 pb->StrokeRect(r);
 pb->SetHighColor(216,216,216,0);
 pb->StrokeLine(r.LeftBottom(),r.LeftBottom());
 pb->StrokeLine(r.RightBottom(),r.RightBottom());
 pb->StrokeLine(r.LeftTop(),r.LeftTop());
 pb->StrokeLine(r.RightTop(),r.RightTop());
 r.top++;
 r.left++;
 r.bottom--;
 r.right--;
 pb->SetHighColor(103,103,103,0);
 pb->StrokeLine(r.LeftBottom(),r.RightBottom());
 pb->StrokeLine(r.RightTop(),r.RightBottom());
 pb->SetHighColor(23,23,23,0);
 pb->StrokeLine(r.LeftTop(),r.RightTop());
 pb->StrokeLine(r.LeftTop(),r.LeftBottom());
 r.top++;
 r.left++;
 r.bottom--;
 r.right--;
 pb->SetHighColor(39,39,39,0);
 pb->StrokeLine(r.LeftBottom(),r.RightBottom());
 pb->StrokeLine(r.RightTop(),r.RightBottom());
 pb->SetHighColor(0,0,0,0);
 pb->StrokeLine(r.LeftTop(),r.RightTop());
 pb->StrokeLine(r.LeftTop(),r.LeftBottom());
 r.top++;
 r.left++;
 r.bottom--;
 r.right--;
 pb->SetHighColor(23,23,23,0);
 pb->FillRect(r);
 pb->SetHighColor(0,0,0,0);
 pb->StrokeLine(r.LeftTop(),r.RightTop());
 pb->StrokeLine(r.LeftTop(),r.LeftBottom());
 pic2=pb->EndPicture();
 
 // Third button
 pic3=new BPicture();
 pb->BeginPicture(pic3);
 r=(pb->Bounds());
 pb->SetHighColor(152,152,152,0);
 pb->StrokeRect(r);
 pb->SetHighColor(216,216,216,0);
 pb->StrokeLine(r.LeftBottom(),r.LeftBottom());
 pb->StrokeLine(r.RightBottom(),r.RightBottom());
 pb->StrokeLine(r.LeftTop(),r.LeftTop());
 pb->StrokeLine(r.RightTop(),r.RightTop());
 r.top++;
 r.left++;
 r.bottom--;
 r.right--;
 pb->SetHighColor(216,216,216,0); // this color is set already but just in case
 pb->StrokeLine(r.LeftBottom(),r.RightBottom());
 pb->StrokeLine(r.RightTop(),r.RightBottom());
 pb->SetHighColor(232,232,232,0);
 pb->StrokeLine(r.LeftTop(),r.RightTop());
 pb->StrokeLine(r.LeftTop(),r.LeftBottom());
 r.top++;
 r.left++;
 r.bottom--;
 r.right--;
 pb->SetHighColor(216,216,216,0);
 pb->StrokeLine(r.LeftBottom(),r.RightBottom());
 pb->StrokeLine(r.RightTop(),r.RightBottom());
 pb->SetHighColor(255,255,255,0);
 pb->StrokeLine(r.LeftTop(),r.RightTop());
 pb->StrokeLine(r.LeftTop(),r.LeftBottom());
 r.top++;
 r.left++;
 r.bottom--;
 r.right--;
 pb->SetHighColor(232,232,232,0);
 pb->FillRect(r);
 pb->SetHighColor(255,255,255,0);
 pb->StrokeLine(r.LeftTop(),r.RightTop());
 pb->StrokeLine(r.LeftTop(),r.LeftBottom());
 pic3=pb->EndPicture();
#endif

#ifdef __UNIVERSAL_INTERFACE
 }
#endif 
 
 
 pb->SetEnabledOn(pic2);
 pb->SetEnabledOff(pic1);
 pb->SetDisabledOn(pic3);
 pb->SetDisabledOff(pic3);
 pb->Invalidate();
 delete pic1;
 delete pic2;
 delete pic3;
}


//////////////////////////////////////
const char* CalendarControl::Version()
{
 if(version==NULL)
 {
  version=new BString("Version ");
  version->Append(CC_VERSION);
 
#ifdef __ZETA__
  version->Append(" compiled in Zeta,");
#else
  version->Append(" compiled in BeOS,");
#endif
 
#ifdef __LANG_LOCALE
  version->Append(" localized,");
#endif
#ifdef __LANG_ENGLISH
  version->Append(" english,");
#endif
#ifdef __LANG_RUSSIAN
  version->Append(" russian,");
#endif
#ifdef __LANG_FRENCH
  version->Append(" french,");
#endif

#ifdef __UNIVERSAL_INTERFACE
  version->Append(" with universal interface.");
#else
 #ifdef __ZETA_INTERFACE
   version->Append(" with Zeta interface.");
 #endif
 #ifdef __BEOS_INTERFACE
   version->Append(" with BeOS interface.");
 #endif
#endif
 }
 return version->String();
}
