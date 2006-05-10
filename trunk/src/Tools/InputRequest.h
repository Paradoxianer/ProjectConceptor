// InputRequest.h
// Autor: Christian Lörchner

#ifndef _INPUTREQUEST_H
#define _INPUTREQUEST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Window.h>  
#include "InputRequestView.h"
#include "ScreenInfo.h"

class InputRequest : public BWindow
{
  public:
						InputRequest(const char* title, const char* label, const char* text, const char* btn0_label);
						InputRequest(const char* title, const char* label, const char* text, const char* btn0_label, const char* btn1_label);
						InputRequest(const char* title, const char* label, const char* text, const char* btn0_label, const char* btn1_label, const char* btn2_label);
	virtual	int32		Go(char** input); //be carful! new memory (for input) is alloceted by this funktion. you have to free memory by yourself

  private:
	virtual	void		Init(const char* label, const char* text, const char* btn0_label, const char* btn1_label, const char* btn2_label);
	virtual	void		MessageReceived(BMessage *msg);
	virtual	void		SetLabel(const char* label);
	virtual	const char*	Label();
	virtual	void		SetText(const char* text);
	virtual	const char*	Text();
	
	#define	IR_WINDOW_WIDTH		315 // recommend min. value! the view does not take this in every case (button width)
	#define	IR_WINDOW_HEIGHT	75

	ScreenInfo			screen;

	InputRequestView	*irView;
	int32				button_index;
};

#endif
