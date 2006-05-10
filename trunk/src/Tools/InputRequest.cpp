// InputRequest.cpp
// Autor: Christian Lörchner

#include "InputRequest.h"

InputRequest::InputRequest(const char* title, const char* label, const char* text, const char* btn0_label)
	:BWindow(BRect(0, 0, IR_WINDOW_WIDTH, IR_WINDOW_HEIGHT), title,
	 B_MODAL_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{
  Init(label, text, btn0_label, NULL, NULL);
}

InputRequest::InputRequest(const char* title, const char* label, const char* text, const char* btn0_label, const char* btn1_label)
	:BWindow(BRect(0, 0, IR_WINDOW_WIDTH, IR_WINDOW_HEIGHT), title,
	 B_MODAL_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{
  Init(label, text, btn0_label, btn1_label, NULL);
}

InputRequest::InputRequest(const char* title, const char* label, const char* text, const char* btn0_label, const char* btn1_label, const char* btn2_label)
	:BWindow(BRect(0, 0, IR_WINDOW_WIDTH, IR_WINDOW_HEIGHT), title,
	 B_MODAL_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{
  Init(label, text, btn0_label, btn1_label, btn2_label);
}

void InputRequest::Init(const char* label, const char* text, const char* btn0_label, const char* btn1_label, const char* btn2_label)
{
  BRect frame = Bounds();
  irView = new InputRequestView(frame, btn0_label, btn1_label, btn2_label);
  
  // now move and resize the window with the calculated width of our view
  MoveTo(BPoint(screen.GetX()/2-irView->Width()/2, screen.GetY()/2-IR_WINDOW_HEIGHT/2-125)); 
  ResizeTo(irView->Width(), IR_WINDOW_HEIGHT);
  
  AddChild(irView);

  SetLabel(label);
  SetText(text);
  
  //init
  button_index = -1;
}

void InputRequest::MessageReceived(BMessage *msg)
{
  switch(msg->what)
  {
	case BTN0_MSG:
	{
	  button_index = 0;
	  break;
	}
	case BTN1_MSG:
	{
	  button_index = 1;
	  break;
	}
	case BTN2_MSG:
	{
	  button_index = 2;
	  break;
	}
	default:
	  BWindow::MessageReceived(msg);
  }
}

void InputRequest::SetLabel(const char* label)
{
  irView->label->SetText(label);  
}

const char* InputRequest::Label()
{
  return irView->label->Text();
}

void InputRequest::SetText(const char* text)
{
  irView->text->SetText(text);
}

const char* InputRequest::Text()
{
  return irView->text->Text();
}

int32 InputRequest::Go(char** input)
{
  button_index = -1;
  Show();
  while (button_index == -1) //wait for a click. i think this should be made in a better way with BMessages ;)
  {
    snooze(100000);
  }
  
  char* input_value = (char*) calloc(strlen(irView->text->Text()) + 1, 1);
  strcpy(input_value, irView->text->Text());
  
  *input = input_value;
  Hide();
  return button_index;
}

