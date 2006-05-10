// InputRequest.h
// Autor: Christian Lörchner

#ifndef _INPUTREQUEST_VIEW_H
#define _INPUTREQUEST_VIEW_H

#include <View.h>

#include <Font.h>

#include <interface/Button.h>
#include <interface/StringView.h>
#include <interface/TextControl.h>

class InputRequestView : public BView
{
  public:
    BStringView		*label;
    BTextControl	*text;
  	
  	BButton		*button0;
  	BButton		*button1;
  	BButton		*button2;
  	
  	#define TEXT_MSG	'text'
  	
	#define BTN0_MSG	'btn0'
	#define BTN1_MSG	'btn1'
	#define BTN2_MSG	'btn2'

    				InputRequestView(BRect frame, const char* btn0_label, const char* btn1_label, const char* btn2_label);
	virtual	float	Width(); //auxilliary function for use in our window
  private:
    virtual void	Draw(BRect rect);
    
    float	height;
    float	width;
    
    BFont	font;
    float 	btn0_width;
    float 	btn1_width;
    float 	btn2_width;  	
    
    #define	BTN_TXT_SPACE	20 // right plus left side of the button label space
  	#define BTN_SPACE		10 // space between buttons
  	#define	BTN_MIN_WIDTH	80 // minimum width of a button
  				
};

#endif
