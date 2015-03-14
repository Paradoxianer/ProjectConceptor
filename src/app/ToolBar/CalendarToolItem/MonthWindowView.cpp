// Calendar Control version 2.5
// by Al.V. Sarikov.
// Kherson, Ukraine, 2006.
// E-mail: avix@ukrpost.net.
// Home page: http://avix.pp.ru.

// Control which allows to work with dates:
// enter date to text field and choose it from calendar.

// Distributed under BSD license (see LICENSE file).

#include <Bitmap.h>
#include <View.h>
#include <time.h>
#include "MouseSenseStringView.cpp"

#ifdef __LANG_LOCALE // __ZETA__ is defined in this case
                     // because it is verified while defining __LANG_LOCALE
 #include <locale/Locale.h>
#endif

#define VIEW_COLOR 216, 216, 216, 255
#define ACTIVE_COLOR 0, 0, 200
#define NOACTIVE_COLOR 120, 120, 120
#define LINE_COLOR 184, 184, 184

class MonthWindowView: public BView
{
 public:
  MonthWindowView(int day, int month, int year, int first_year, int last_year);
  ~MonthWindowView();
  virtual void AttachedToWindow(void);
  virtual void Draw(BRect r);
  virtual void KeyDown(const char *bytes, int32 numBytes);
  virtual void MessageReceived(BMessage *msg);
  virtual void MouseDown(BPoint p);
  virtual void MouseUp(BPoint p);
 private:
  void DrawMonth();
  
  BMessenger *msng;
  
  MouseSenseStringView *yearMStringView[2];
  MouseSenseStringView *monthMStringView[2];
  
  MouseSenseStringView *todayStringView;
  BStringView *monthStringView;
  BStringView *yearStringView;
  char *weekdayNames[7];
  char *monthNames[12];
  int monthDays[12];
  
  BBitmap *Bmp;
  BView *BmpView;
  
  int cday;
  int cmonth;
  int cyear; // current date in window
  int cwday1; // day of week of 1st of month in window
  int cwday; // day of week of current day of month in window
  
  int first_year;
  int last_year;
  
  int tday;
  int tmonth;
  int tyear; // today
  int twday; // is needed to calculate days of week
  
  float h_cell;
  float w_cell; // height and width of cell for one number of day of nonth
  
  BRect cursor; // rectangle arounding chosen date
  
  BRect cursorPressed; // rectangle arounding date where mouse was pressed
  bool MousePressed; // is mouse pressed on date
  int dayPressed; // date where mouse is pressed
  
  bool NewMonth; // if true field must be cleared
  
  int which_focused; // 0 - if month, 1 - if year,
                     // 2 - if days of month, 3 - if today
};


////////////////////////////// ///////////////////////////////////
MonthWindowView::MonthWindowView(int day, int month, int year,
                                 int first_year, int last_year)
                :BView(BRect(0,0,100,100), "MonthWindowViewAViX",
                       B_FOLLOW_LEFT, B_WILL_DRAW)
{
 this->cday=day;
 this->cmonth=month;
 this->cyear=year;
 
 this->first_year=first_year;
 this->last_year=last_year;
 
 struct tm *dat;
 time_t tmp;
 time(&tmp);
 dat=localtime(&tmp);
 
 tday=dat->tm_mday;
 tmonth=dat->tm_mon+1;
 tyear=dat->tm_year+1900; // new day coming when window is opened is not handled
                          // because it is very rear case
 twday=dat->tm_wday;

#ifdef __LANG_LOCALE
 weekdayNames[0]=(char*)_T("Mo");
 weekdayNames[1]=(char*)_T("Tu");
 weekdayNames[2]=(char*)_T("We");
 weekdayNames[3]=(char*)_T("Th");
 weekdayNames[4]=(char*)_T("Fr");
 weekdayNames[5]=(char*)_T("Sa");
 weekdayNames[6]=(char*)_T("Su");
 
 monthNames[0]=(char*)_T("January");
 monthNames[1]=(char*)_T("February");
 monthNames[2]=(char*)_T("March");
 monthNames[3]=(char*)_T("April");
 monthNames[4]=(char*)_T("May");
 monthNames[5]=(char*)_T("June");
 monthNames[6]=(char*)_T("July");
 monthNames[7]=(char*)_T("August");
 monthNames[8]=(char*)_T("September");
 monthNames[9]=(char*)_T("October");
 monthNames[10]=(char*)_T("November");
 monthNames[11]=(char*)_T("December");
#endif

#ifdef __LANG_ENGLISH
 weekdayNames[0]="Mo";
 weekdayNames[1]="Tu";
 weekdayNames[2]="We";
 weekdayNames[3]="Th";
 weekdayNames[4]="Fr";
 weekdayNames[5]="Sa";
 weekdayNames[6]="Su";
 
 monthNames[0]="January";
 monthNames[1]="February";
 monthNames[2]="March";
 monthNames[3]="April";
 monthNames[4]="May";
 monthNames[5]="June";
 monthNames[6]="July";
 monthNames[7]="August";
 monthNames[8]="September";
 monthNames[9]="October";
 monthNames[10]="November";
 monthNames[11]="December";
#endif

#ifdef __LANG_RUSSIAN
 weekdayNames[0]="Пн";
 weekdayNames[1]="Вт";
 weekdayNames[2]="Ср";
 weekdayNames[3]="Чт";
 weekdayNames[4]="Пт";
 weekdayNames[5]="Сб";
 weekdayNames[6]="Вс";
 
 monthNames[0]="Январь";
 monthNames[1]="Февраль";
 monthNames[2]="Март";
 monthNames[3]="Апрель";
 monthNames[4]="Май";
 monthNames[5]="Июнь";
 monthNames[6]="Июль";
 monthNames[7]="Август";
 monthNames[8]="Сентябрь";
 monthNames[9]="Октябрь";
 monthNames[10]="Ноябрь";
 monthNames[11]="Декабрь";
#endif

#ifdef __LANG_FRENCH
 // short days of week
 weekdayNames[0]="Lu";
 weekdayNames[1]="Ma";
 weekdayNames[2]="Me";
 weekdayNames[3]="Je";
 weekdayNames[4]="Ve";
 weekdayNames[5]="Sa";
 weekdayNames[6]="Di";
 
 // month names
 monthNames[0]="Janvier";
 monthNames[1]="Février";
 monthNames[2]="Mars";
 monthNames[3]="Avril";
 monthNames[4]="Mai";
 monthNames[5]="Juin";
 monthNames[6]="Juillet";
 monthNames[7]="Aout";
 monthNames[8]="Septembre";
 monthNames[9]="Octobre";
 monthNames[10]="Novembre";
 monthNames[11]="Décembre";
#endif
 
 monthDays[0]=31;
 monthDays[1]=28;
 monthDays[2]=31;
 monthDays[3]=30;
 monthDays[4]=31;
 monthDays[5]=30;
 monthDays[6]=31;
 monthDays[7]=31;
 monthDays[8]=30;
 monthDays[9]=31;
 monthDays[10]=30;
 monthDays[11]=31;
 
 NewMonth=false; // first field is cleared and clearing is not needed
 
 MousePressed=false;
}


///////////////////////////////////
MonthWindowView::~MonthWindowView()
{
 delete todayStringView;
 delete monthStringView;
 delete monthMStringView[0];
 delete monthMStringView[1];
 delete yearStringView;
 delete yearMStringView[0];
 delete yearMStringView[1];
 delete msng;
 Bmp->RemoveChild(BmpView);
 delete BmpView;
 delete Bmp;
}


////////////////////////////////////////////
void MonthWindowView::AttachedToWindow(void)
{
 SetFont(be_plain_font);
 
 SetViewColor(VIEW_COLOR);
 
 // Calculate size of cell needed for number of day of month
 font_height h;
 be_plain_font->GetHeight(&h);
 h_cell=(int)(h.ascent+h.descent+h.leading+6);
 w_cell=StringWidth("4")*4;
 
 

#ifdef __LANG_RUSSIAN
 
 BString s("");
 s<<tday;
 s.Append(" ");
 switch(tmonth)
 {
  case 1:
  {
   s.Append("января ");
   break;
  }
  case 2:
  {
   s.Append("февраля ");
   break;
  }
  case 3:
  {
   s.Append("марта ");
   break;
  }
  case 4:
  {
   s.Append("апреля ");
   break;
  }
  case 5:
  {
   s.Append("мая ");
   break;
  }
  case 6:
  {
   s.Append("июня ");
   break;
  }
  case 7:
  {
   s.Append("июля ");
   break;
  }
  case 8:
  {
   s.Append("августа ");
   break;
  }
  case 9:
  {
   s.Append("сентября ");
   break;
  }
  case 10:
  {
   s.Append("октября ");
   break;
  }
  case 11:
  {
   s.Append("ноября ");
   break;
  }
  case 12:
  {
   s.Append("декабря ");
   break;
  }
 }
 s<<tyear;
 s.Append(" г.");

#else // localized, english and french
 
 BString s("");
 s<<tday;
 s.Append(" ");
 s.Append(monthNames[tmonth-1]);
 s.Append(" ");
 s<<tyear;

#endif
 
 msng=new BMessenger(this);
 todayStringView=new MouseSenseStringView(new BMessage('TODA'), msng,
                                          BRect(10,10,100,100),"todayMStringViewAViX",
                                          s.String());
 AddChild(todayStringView);
 todayStringView->ResizeToPreferred();
 todayStringView->SetViewColor(VIEW_COLOR);
 
 monthStringView=new BStringView(BRect(10,10,100,100),"monthStringViewAViX",
                                 monthNames[8]);
 monthStringView->SetAlignment(B_ALIGN_CENTER);
 AddChild(monthStringView);
 monthStringView->ResizeToPreferred();
 monthStringView->SetText(monthNames[cmonth-1]);
 monthStringView->SetViewColor(VIEW_COLOR);
 
 s.SetTo("");
 if(cyear<10) s.Append("000");
 else if(cyear<100) s.Append("00");
 else if(cyear<1000) s.Append("0");
 s<<cyear;
 
 yearStringView=new BStringView(BRect(10,10,100,100),"yearStringViewAViX",
                                "0000");
 AddChild(yearStringView);
 yearStringView->ResizeToPreferred();
 yearStringView->SetText(s.String());
 yearStringView->SetViewColor(VIEW_COLOR);
 
 ResizeTo(w_cell*7+1,h_cell*7+3+16+yearStringView->Bounds().bottom+todayStringView->Bounds().bottom);
 Window()->ResizeTo(Bounds().right, Bounds().bottom);
 
 yearMStringView[0]=new MouseSenseStringView(new BMessage('YEA0'),msng,
                                             BRect(10,10,100,100),
                                             "yearMStringViewAViX0",
                                             "<<");
 AddChild(yearMStringView[0]);
 yearMStringView[0]->ResizeToPreferred();
 yearMStringView[0]->SetViewColor(VIEW_COLOR);
 
 yearMStringView[1]=new MouseSenseStringView(new BMessage('YEA1'),msng,
                                             BRect(10,10,100,100),
                                             "yearMStringViewAViX1",
                                             ">>");
 AddChild(yearMStringView[1]);
 yearMStringView[1]->ResizeToPreferred();
 yearMStringView[1]->SetViewColor(VIEW_COLOR);
 
 monthMStringView[0]=new MouseSenseStringView(new BMessage('MON0'),msng,
                                              BRect(10,10,100,100),
                                              "monthMStringViewAViX0",
                                              "<<");
 AddChild(monthMStringView[0]);
 monthMStringView[0]->ResizeToPreferred();
 monthMStringView[0]->SetViewColor(VIEW_COLOR);
 
 monthMStringView[1]=new MouseSenseStringView(new BMessage('MON1'),msng,
                                              BRect(10,10,100,100),
                                              "monthMStringViewAViX1",
                                              ">>");
 AddChild(monthMStringView[1]);
 monthMStringView[1]->ResizeToPreferred();
 monthMStringView[1]->SetViewColor(VIEW_COLOR);
 
 todayStringView->MoveTo((Bounds().right-todayStringView->Bounds().right)/2,
                         Bounds().bottom-todayStringView->Bounds().bottom-2);
 if(tyear<first_year || tyear>last_year) todayStringView->SetHighColor(NOACTIVE_COLOR);
  
 yearMStringView[1]->MoveTo(Bounds().right-yearMStringView[1]->Bounds().right,5);
 yearStringView->MoveTo(yearMStringView[1]->Frame().left-yearStringView->Bounds().right,5);
 yearMStringView[0]->MoveTo(yearStringView->Frame().left-yearMStringView[0]->Bounds().right,5);
 
 
 monthStringView->MoveTo((yearMStringView[0]->Frame().left-monthStringView->Bounds().right)/2,5);
 monthMStringView[0]->MoveTo(monthStringView->Frame().left-monthMStringView[0]->Bounds().right,5);
 monthMStringView[1]->MoveTo(monthStringView->Frame().right,5);
 
 which_focused=2; // days of month
 
 
 Bmp=new BBitmap(BRect(Frame()),B_RGB32,true);
 BmpView=new BView(Bmp->Bounds(),"BV",0,B_WILL_DRAW);
 Bmp->AddChild(BmpView);
 
 Bmp->Lock();
 BmpView->SetHighColor(VIEW_COLOR);
 BmpView->FillRect(BmpView->Frame());
 
 BmpView->SetHighColor(LINE_COLOR);
 BmpView->StrokeLine(BPoint(3,todayStringView->Frame().top-5),
                     BPoint(Bounds().right-3,todayStringView->Frame().top-5));
 BmpView->StrokeLine(BPoint(3,yearStringView->Frame().bottom+2),
                     BPoint(Bounds().right-3,yearStringView->Frame().bottom+2));
 BmpView->SetHighColor(0,0,0,0);
 
 float y=yearStringView->Frame().bottom+h_cell;
 float x=0;
 for(int i=0;i<7;i++)
 {
  BmpView->DrawString(weekdayNames[i],BPoint(x+(w_cell-StringWidth(weekdayNames[i]))/2,y));
  x+=w_cell;
 }
 
 BmpView->Sync();
 Bmp->Unlock();
 DrawMonth();
}


///////////////////////////////////
void MonthWindowView::Draw(BRect r)
{
 Window()->Lock();
 DrawBitmap(Bmp, BPoint(0,0));
 Window()->Unlock();
 return;
}


/////////////////////////////////
void MonthWindowView::DrawMonth()
{
 Bmp->Lock();
 
 float y=yearStringView->Frame().bottom+h_cell;
 float x=0;
 
 if(NewMonth)
 {
  BmpView->SetHighColor(VIEW_COLOR);
  BmpView->FillRect(BRect(0,y+1,
                    BmpView->Bounds().right,todayStringView->Frame().top-6));
  BmpView->SetHighColor(0,0,0,0);
  NewMonth=false;
 }
 
 int byear=cyear; // base year
 if(tyear<byear) byear=tyear;
  
 int day1=0, m=0, k=byear;
 
 while(k<cyear)
 {
  day1++;

  if(k%4==0) // leap year?
   if((k%100!=0) || (k%400==0)) day1++; // yes
  
  k++;
 }
 while(++m<cmonth)
 {
  day1+=(monthDays[m-1]-28);
  if(m==2) if((cyear%4)==0) if((cyear%100!=0) || (cyear%400==0)) day1++;
 }
 day1++; // day1 is number of 1st day of chosen month in chosen year
 day1=day1%7;
 
 int day2=0;
 m=0;
 k=byear;
 while(k<tyear)
 {
  day2++;
  if((k%4)==0) if((k%100!=0) || (k%400==0)) day2++;
  k++;
 }
 while(++m<tmonth)
 {
  day2+=(monthDays[m-1]-28);
  if(m==2) if((tyear%4)==0) if((tyear%100!=0) || (tyear%400==0)) day2++;
 }
 day2+=tday; // day2 - number of today's day in today's year
 day2=day2%7;
 
 k=(twday==0) ? 6 : twday-1;
 
 k=k-day2+day1;
 while(k<0) k+=7;
 k=k%7;
 cwday1=k;
 
 x=w_cell*k+1;
 y+=h_cell;
 
 int qu_days=monthDays[cmonth-1]; // quantity of days in month
 
 if(cmonth==2) if(cyear%4==0) if((cyear%100!=0) || (cyear%400==0)) qu_days=29;
 
 BString s;
 int t=0;
 while(t<qu_days)
 {
  t++;
  
  s<<t;
  
  if(cyear==tyear) if(cmonth==tmonth) if(t==tday) BmpView->SetHighColor(200,0,0,0);
  BmpView->DrawString(s.String(),BPoint(x+(w_cell-StringWidth(s.String()))/2,y));
  if(cyear==tyear) if(cmonth==tmonth) if(t==tday) BmpView->SetHighColor(0,0,0,0);
  
  if(t==cday)
  {
   cwday=k;
   if(which_focused==2) BmpView->SetHighColor(ACTIVE_COLOR);
   else BmpView->SetHighColor(NOACTIVE_COLOR);
   cursor.Set(x,y-h_cell+5,x+w_cell-1,y+4);
   BmpView->StrokeRect(cursor);
   BmpView->SetHighColor(0,0,0,0);
  }
  
  x+=w_cell;
  k++;
  s.SetTo("");
  
  if(k==7)
  {
   k=0;
   y+=h_cell;
   x=1;
  }
 }
 
 BmpView->Sync();
 Bmp->Unlock();
 Draw(Bounds());
}


////////////////////////////////////////////////////////////////
void MonthWindowView::KeyDown(const char *bytes, int32 numBytes)
{
 switch(bytes[0])
 {
  case B_TAB:
  {
   Bmp->Lock();
   switch(which_focused)
   {
    case 0: // months
    {
     BmpView->SetHighColor(VIEW_COLOR);
     BmpView->StrokeLine(BPoint(monthMStringView[0]->Frame().left,
                                monthMStringView[0]->Frame().bottom+1),
                         BPoint(monthMStringView[1]->Frame().right,
                                monthMStringView[1]->Frame().bottom+1));
     BmpView->SetHighColor(0,0,0);
     break;
    }
    case 1: // years
    {
     BmpView->SetHighColor(VIEW_COLOR);
     BmpView->StrokeLine(BPoint(yearMStringView[0]->Frame().left,
                                yearMStringView[0]->Frame().bottom+1),
                         BPoint(yearMStringView[1]->Frame().right,
                                yearMStringView[1]->Frame().bottom+1));
     BmpView->SetHighColor(0,0,0);
     break;
    }
    case 2: // days of month
    {
     BmpView->SetHighColor(NOACTIVE_COLOR);
     BmpView->StrokeRect(cursor);
     BmpView->SetHighColor(0,0,0);
     break;
    }
    case 3: // today
    {
     BmpView->SetHighColor(VIEW_COLOR);
     BmpView->StrokeLine(BPoint(todayStringView->Frame().left,
                                todayStringView->Frame().bottom+1),
                         BPoint(todayStringView->Frame().right,
                                todayStringView->Frame().bottom+1));
     BmpView->SetHighColor(0,0,0);
     break;
    }
   }
   
   // changing which_focused
   if(modifiers() & B_SHIFT_KEY)
   {
    if(which_focused==0)
      which_focused=3;
    else which_focused--;
   }
   else // simply Tab
   {
    if(which_focused==3)
     which_focused=0;
    else which_focused++;
   }
   
   // drawing with new which_focused
   switch(which_focused)
   {
    case 0: // months
    {
     BmpView->SetHighColor(ACTIVE_COLOR);
     BmpView->StrokeLine(BPoint(monthMStringView[0]->Frame().left,
                                monthMStringView[0]->Frame().bottom+1),
                         BPoint(monthMStringView[1]->Frame().right,
                                monthMStringView[1]->Frame().bottom+1));
     BmpView->SetHighColor(0,0,0);
     break;
    }
    case 1: // years
    {
     BmpView->SetHighColor(ACTIVE_COLOR);
     BmpView->StrokeLine(BPoint(yearMStringView[0]->Frame().left,
                                yearMStringView[0]->Frame().bottom+1),
                         BPoint(yearMStringView[1]->Frame().right,
                                yearMStringView[1]->Frame().bottom+1));
     BmpView->SetHighColor(0,0,0);
     break;
    }
    case 2: // days of month
    {
     BmpView->SetHighColor(ACTIVE_COLOR);
     BmpView->StrokeRect(cursor);
     BmpView->SetHighColor(0,0,0);
     break;
    }
    case 3: // today
    {
     BmpView->SetHighColor(ACTIVE_COLOR);
     BmpView->StrokeLine(BPoint(todayStringView->Frame().left,
                                todayStringView->Frame().bottom+1),
                         BPoint(todayStringView->Frame().right,
                                todayStringView->Frame().bottom+1));
     BmpView->SetHighColor(0,0,0);
     break;
    }
   }
   BmpView->Sync();
   Bmp->Unlock();
   Draw(Bounds());
   break;
  } // B_TAB
  
  case B_DOWN_ARROW:
  {
   if(which_focused==0) // month
   { 
    BMessage msg('MON0');
    MessageReceived(&msg);
   }
   else if(which_focused==1) // year
   {
    BMessage msg('YEA0');
    MessageReceived(&msg);
   }
   else if(which_focused==2) // days of month
   {
    int qu_days=monthDays[cmonth-1];
    if(cmonth==2) if(cyear%4==0) if((cyear%100!=0) || (cyear%400==0)) qu_days=29;
    if((cday+7)<=qu_days)
    {
     Bmp->Lock();
     BmpView->SetHighColor(VIEW_COLOR);
     BmpView->StrokeRect(cursor);
     cursor.OffsetBySelf(0, h_cell);
     cday+=7;
     BmpView->SetHighColor(ACTIVE_COLOR);
     BmpView->StrokeRect(cursor);
     BmpView->SetHighColor(0,0,0);
     BmpView->Sync();
     Bmp->Unlock();
     Draw(Bounds());
    }
   }
   break;
  }
  
  case B_UP_ARROW:
  {
   // Test whether Ctrl+UpArrow is pressed
   if(modifiers() & B_CONTROL_KEY)
    Window()->PostMessage(B_QUIT_REQUESTED);
   else
   {
    if(which_focused==0) // month
    {
     BMessage msg('MON1');
     MessageReceived(&msg);
    }
    else if(which_focused==1) // year
    {
     BMessage msg('YEA1');
     MessageReceived(&msg);
    }
    else if(which_focused==2) // days of month
    {
     if((cday-7)>=1)
     {
      Bmp->Lock();
      BmpView->SetHighColor(VIEW_COLOR);
      BmpView->StrokeRect(cursor);
      cursor.OffsetBySelf(0, -h_cell);
      cday-=7;
      BmpView->SetHighColor(ACTIVE_COLOR);
      BmpView->StrokeRect(cursor);
      BmpView->SetHighColor(0,0,0);
      BmpView->Sync();
      Bmp->Unlock();
      Draw(Bounds());
     }
    }
   }
   break;
  }
  
  case B_LEFT_ARROW:
  {
   if(which_focused==0) // month
   {
    BMessage msg('MON0');
    MessageReceived(&msg);
   }
   else if(which_focused==1) // year
   {
    BMessage msg('YEA0');
    MessageReceived(&msg);
   }
   else if(which_focused==2) // days of month
   {
    if(cday>1)
    {
     Bmp->Lock();
     BmpView->SetHighColor(VIEW_COLOR);
     BmpView->StrokeRect(cursor);
     if(cwday>0) // by dates no matter of day of week
     {
      cursor.OffsetBySelf(-w_cell,0);
      cwday--;
     }
     else // cwday==0
     {
      cursor.OffsetBySelf(w_cell*6,-h_cell);
      cwday=6;
     }
     cday--;
     BmpView->SetHighColor(ACTIVE_COLOR);
     BmpView->StrokeRect(cursor);
     BmpView->SetHighColor(0,0,0);
     BmpView->Sync();
     Bmp->Unlock();
     Draw(Bounds());
    }
    else // 1st of month, go month before
    {
     if(cyear>first_year || cmonth>1) // one can go
     {
      int cm=(cmonth==1) ? 12 : cmonth-1;
      int qu_days=monthDays[cm-1];
      if(cm==2) if(cyear%4==0) 
       if((cyear%100!=0) || (cyear%400==0)) qu_days=29;
       // it is correct not to pay attention to year changing
       // because if we moved to february then year was not changed
       // but if month is other then february 
       // then it has the same number of days every year.
      cday=qu_days;
      
      BMessage msg('MON0');
      MessageReceived(&msg); // here month, cwday and year (if needed) will be changed
     }
    }
   }
   break;
  }
  
  case B_RIGHT_ARROW:
  {
   if(which_focused==0) // month
   {
    BMessage msg('MON1');
    MessageReceived(&msg);
   }
   else if(which_focused==1) // year
   {
    BMessage msg('YEA1');
    MessageReceived(&msg);
   }
   else if(which_focused==2) // days of month
   {
    int qu_days=monthDays[cmonth-1];
    if(cmonth==2) if(cyear%4==0) if((cyear%100!=0) || (cyear%400==0)) qu_days=29;
    if(cday<qu_days)
    {
     Bmp->Lock();
     BmpView->SetHighColor(VIEW_COLOR);
     BmpView->StrokeRect(cursor);
     if(cwday<6) // by dates no matter of day of week
     {
      cursor.OffsetBySelf(w_cell,0);
      cwday++;
     }
     else // cwday==6
     {
      cursor.OffsetBySelf(-w_cell*6,h_cell);
      cwday=0;
     }
     cday++;
     BmpView->SetHighColor(ACTIVE_COLOR);
     BmpView->StrokeRect(cursor);
     BmpView->SetHighColor(0,0,0);
     BmpView->Sync();
     Bmp->Unlock();
     Draw(Bounds());
    }
    else // last of month, go next month
    {
     if(cyear<last_year || cmonth<12) // one can go
     {
      cday=1;
      BMessage msg('MON1');
      MessageReceived(&msg); // here month, cwday and year (if needed) will be changed
     }
    }
   }
   break;
  }
  
  case B_ESCAPE:
  {
   Window()->PostMessage(B_QUIT_REQUESTED);
   break;
  }
  
  case B_ENTER:
  {
   if(which_focused==2)
   {
    // here it is needed to send dayPressed (==cwday), cyear, cmonth
    // to window and leave
    BMessage *msg=new BMessage('MVME');
    msg->AddInt32("day",cday);
    msg->AddInt32("month",cmonth);
    msg->AddInt32("year",cyear);
    Window()->PostMessage(msg);
   }
   else if(which_focused==3)
   {
    BMessage msg('TODA');
    MessageReceived(&msg);
   }
   break;
  }
  
  case B_SPACE: // the same as B_ENTER
  {
   if(which_focused==2)
   {
    // here it is needed to send dayPressed (==cwday), cyear, cmonth
    // to window and leave
    BMessage *msg=new BMessage('MVME');
    msg->AddInt32("day",cday);
    msg->AddInt32("month",cmonth);
    msg->AddInt32("year",cyear);
    Window()->PostMessage(msg);
   }
   else if(which_focused==3)
   {
    BMessage msg('TODA');
    MessageReceived(&msg);
   }
   break;
  }
  
  default: 
   BView::KeyDown(bytes, numBytes);
 }
}


////////////////////////////////////////////////////
void MonthWindowView::MessageReceived(BMessage *msg)
{
 switch(msg->what)
 {
  case 'YEA0': // year before
  {
   if(cyear<=first_year) break;
   cyear--;
   NewMonth=true;
   BString s("");
   if(cyear<10) s.Append("000");
   else if(cyear<100) s.Append("00");
   else if(cyear<1000) s.Append("0");
   s<<cyear;
   yearStringView->SetText(s.String());
   
   if(cmonth==2) if(cday==29) // one can do simplier: if cday==29, then make
                              // cday=28, because two leap years one after other can't be
   {
    if(cyear%4!=0) cday=28;
    else if(cyear%100==0 && cyear%400!=0) cday=28;
   }
   
   DrawMonth();
   break;
  }
  case 'YEA1': // year after
  {
   if(cyear>=last_year) break;
   cyear++;
   NewMonth=true;
   BString s("");
   if(cyear<10) s.Append("000");
   else if(cyear<100) s.Append("00");
   else if(cyear<1000) s.Append("0");
   s<<cyear;
   yearStringView->SetText(s.String());
   
   if(cmonth==2) if(cday==29)
   {
    if(cyear%4!=0) cday=28;
    else if(cyear%100==0 && cyear%400!=0) cday=28;
   }
   
   DrawMonth();
   break;
  }
  case 'MON0': // month before
  {
   if(cmonth>1) cmonth--;
   else if(cyear>first_year)
   {
    cmonth=12;
    cyear--;
    
    BString s("");
    if(cyear<10) s.Append("000");
    else if(cyear<100) s.Append("00");
    else if(cyear<1000) s.Append("0");
    s<<cyear;
    yearStringView->SetText(s.String());
   }
   else break; // nothing is changed
   
   NewMonth=true;
   monthStringView->SetText(monthNames[cmonth-1]);
   
   if(cmonth==2)
   {
    int tmpday=28;
    if(cyear%4==0) if((cyear%100!=0) || (cyear%400==0)) tmpday=29;
    if(cday>tmpday) cday=tmpday;
   }
   else if(cday>monthDays[cmonth-1]) cday=monthDays[cmonth-1];
   
   DrawMonth();
   break;
  }
  case 'MON1': // month after
  {
   if(cmonth<12) cmonth++;
   else if(cyear<last_year)
   {
    cmonth=1;
    cyear++;
    
    BString s("");
    if(cyear<10) s.Append("000");
    else if(cyear<100) s.Append("00");
    else if(cyear<1000) s.Append("0");
    s<<cyear;
    yearStringView->SetText(s.String());
   }
   else break; // nothing is changed
   
   NewMonth=true;
   monthStringView->SetText(monthNames[cmonth-1]);
   
   if(cmonth==2)
   {
    int tmpday=28;
    if(cyear%4==0) if((cyear%100!=0) || (cyear%400==0)) tmpday=29;
    if(cday>tmpday) cday=tmpday;
   }
   else if(cday>monthDays[cmonth-1]) cday=monthDays[cmonth-1];
   
   DrawMonth();
   break;
  }
  case 'TODA': // to place to today's date
  {
   if(tyear<first_year || tyear>last_year) break;
   
   NewMonth=true;
   
   cyear=tyear;
   cmonth=tmonth;
   cday=tday;
   
   BString s("");
   if(cyear<10) s.Append("000");
   else if(cyear<100) s.Append("00");
   else if(cyear<1000) s.Append("0");
   s<<cyear;
   yearStringView->SetText(s.String());
   monthStringView->SetText(monthNames[cmonth-1]);
   
   DrawMonth();
   break;
  }
  default:
   BView::MessageReceived(msg);
 }
}


/////////////////////////////////////////
void MonthWindowView::MouseDown(BPoint p)
{
 // It's necessary to define whether mouse cursor is on one of dates
 BRect r;
 
 int k=cwday1;
 float x=w_cell*k+1;
 float y=yearStringView->Frame().bottom+h_cell+h_cell;
 
 int qu_days=monthDays[cmonth-1]; // quantity of days in month
 
 if(cmonth==2) if(cyear%4==0) if((cyear%100!=0) || (cyear%400==0)) qu_days=29;
 
 bool flag=false; // whether mouse is pressed just on date
 int t=0;
 while(t<qu_days)
 {
  t++;
  r.Set(x,y-h_cell+5,x+w_cell-1,y+4);
  
  if(r.Contains(p))
  {
   flag=true;
   dayPressed=t;
   cursorPressed.Set(r.left,r.top,r.right,r.bottom);
   MousePressed=true;
   break; // exit while
  }
  
  x+=w_cell;
  k++;
  
  if(k==7)
  {
   k=0;
   y+=h_cell;
   x=1;
  }
 }
}


///////////////////////////////////////
void MonthWindowView::MouseUp(BPoint p)
{
 if(!MousePressed) return;
 
 if(cursorPressed.Contains(p))
 {
  // here it is needed to send dayPressed, cyear, cmonth
  // to window and leave
  BMessage *msg=new BMessage('MVME');
  msg->AddInt32("day",dayPressed);
  msg->AddInt32("month",cmonth);
  msg->AddInt32("year",cyear);
  Window()->PostMessage(msg);
 }
 else MousePressed=false;
}
