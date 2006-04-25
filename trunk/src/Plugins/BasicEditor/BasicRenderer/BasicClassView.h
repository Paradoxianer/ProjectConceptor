#ifndef BASIC_CLASS_VIEW_H
#define BASIC_CLASS_VIEW_H

#include <app/Message.h>
#include <app/Messenger.h>
#include <interface/View.h>
#include <interface/Box.h>
#include <interface/PictureButton.h>
#include <interface/TextView.h>
#include <interface/TextControl.h>

#include "BasicEditor.h"
#include "PDocument.h"
#include "PCommand.h"

#ifdef B_ZETA_VERSION
	#include <locale/Locale.h>
	#include <locale/LanguageNotifier.h>
#else
	#define _T(a) a
#endif 

const uint32	B_C_V_NAME_CHANGED	= 'bcNC';

class BasicClassView: public BBox
{

public:
						BasicClassView(BMessage *forMessage);
	virtual void		AttachedToWindow(); 
	virtual void		Draw(BRect updateRect);
	virtual	void		MouseDown(BPoint where);
	virtual	void		MouseUp(BPoint where);
	virtual	void		MouseMoved(BPoint pt, uint32 code, const BMessage *msg); 
	virtual	void		LanguageChanged();
	virtual	void		MessageReceived(BMessage *message);
	virtual	void		FrameMoved(BPoint new_position);
	virtual	void		FrameResized(float new_width, float new_height);
	virtual void		ValueChanged(void);

protected:
	virtual void		Init();
	BMessage			*viewMessage;
		
	float				xRadius,yRadius;
	rgb_color			fillColor,borderColor;
	BPoint				*startMouseDown;
	BPoint				*pStartMouseDown;
	BPoint				*oldPpt;

	int					connecting;
	bool				resizing;
	BTextControl		*objectName;

	BMessage			*dataMessage;	
	BMessage			*configMessage;
	BMessage			*patternMessage;
	BMessage			*fontMessage;

	PCommand			*moveCommand;
	PCommand			*resizeCommand;
	PCommand			*selectCommand;

	PDocument			*doc;
	BMessenger			*sentTo;
	
	BList				*outgoing;
	BList				*incoming;
private:
};
#endif
