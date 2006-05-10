// InputRequestView.cpp
// Autor: Christian Lörchner

#include "InputRequestView.h"

InputRequestView::InputRequestView(BRect frame, const char* btn0_label, const char* btn1_label, const char* btn2_label)
	:BView(frame, "InputRequestView", B_FOLLOW_ALL, B_WILL_DRAW)
{
  float 	temp;
  
  //calculate the necessary width of our view
  //first we have to know the width of every button
  temp = font.StringWidth(btn0_label);
  btn0_width = (temp + BTN_TXT_SPACE < BTN_MIN_WIDTH)? BTN_MIN_WIDTH : temp + BTN_TXT_SPACE;
  if (btn1_label != NULL)
  {
    temp = font.StringWidth(btn1_label);
    btn1_width = (temp + BTN_TXT_SPACE < BTN_MIN_WIDTH)? BTN_MIN_WIDTH : temp + BTN_TXT_SPACE;
  }
  else
    btn1_width = 0;
  if (btn2_label != NULL)
  {
    temp = font.StringWidth(btn2_label);
    btn2_width = (temp + BTN_TXT_SPACE < BTN_MIN_WIDTH)? BTN_MIN_WIDTH : temp + BTN_TXT_SPACE;
  }
  else
    btn2_width = 0;
    
  height = frame.Height();
  temp = 15 + btn2_width + BTN_SPACE + btn1_width + BTN_SPACE + btn0_width + BTN_SPACE + 50;
  width = (temp > frame.Width())? temp : frame.Width();

  ResizeTo(width, height);

  //install our grafical elements
  SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
  
  label = new BStringView(BRect(60, 10, width - 15 - 150 - 10, 30), "label", NULL);
  AddChild(label);
  
  text = new BTextControl(BRect(width - 15 - 150, 10, width - 15, 30), "text", NULL, NULL, new BMessage(TEXT_MSG));
  text->SetDivider(0);
  AddChild(text);
  
  BRect		rect0;
  BRect		rect1;
  BRect		rect2;
  
  // only show needed buttons and stretch them if necessary
  // btn0 always is the leftmost button
  if (btn1_label == NULL)
  {
    rect0 = BRect(width - 15 - btn0_width, 40, width - 15, 60);
    button0 = new BButton(rect0, "button0", btn0_label, new BMessage(BTN0_MSG));
    AddChild(button0);
  }
  else if (btn2_label == NULL)
  {
    rect1 = BRect(width - 15 - btn1_width, 40, width - 15, 60);
    rect0 = BRect(width - 15 - btn1_width - BTN_SPACE - btn0_width, 40, width - 15 - btn1_width - BTN_SPACE, 60);
    
    button0 = new BButton(rect0, "button0", btn0_label, new BMessage(BTN0_MSG));
    AddChild(button0);
    button1 = new BButton(rect1, "button1", btn1_label, new BMessage(BTN1_MSG));
    AddChild(button1);
  }
  else
  {
    rect2 = BRect(width - 15 - btn2_width, 40, width - 15, 60);
    rect1 = BRect(width - 15 - btn2_width - BTN_SPACE - btn1_width, 40, width - 15 - btn2_width - BTN_SPACE, 60);
    rect0 = BRect(width - 15 - btn2_width - BTN_SPACE - btn1_width - BTN_SPACE - btn0_width, 40, width - 15 - btn2_width - BTN_SPACE - btn1_width - BTN_SPACE, 60);
    
    button0 = new BButton(rect0, "button0", btn0_label, new BMessage(BTN0_MSG));
    AddChild(button0);
    button1 = new BButton(rect1, "button1", btn1_label, new BMessage(BTN1_MSG));
    AddChild(button1);
    button2 = new BButton(rect2, "button2", btn2_label, new BMessage(BTN2_MSG));
    AddChild(button2);
  }

}

float InputRequestView::Width()
{
  return width;
}

void InputRequestView::Draw(BRect rect)
{
  //draw our nice gray rect on the left side of the window
  SetHighColor(183, 183, 183, 0);
  FillRect(BRect(0, 0, 30, height), B_SOLID_HIGH);
  
  //TODO:
  //draw a question mark
}
