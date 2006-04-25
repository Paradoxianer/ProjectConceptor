#ifndef BASIC_CONNECTION_VIEW_H
#define BASIC_CONNECTION_VIEW_H

#include <app/Message.h>
#include <app/Messenger.h>
#include <interface/View.h>
#include <interface/TextView.h>
#include <interface/TextControl.h>

const uint32	B_C_NAME_CHANGED	= 'bcNC';

#include "BasicEditor.h"

class BasicConnectionView: public BView
{

public:
						BasicConnectionView(BMessage *forMessage);
	virtual void		AttachedToWindow(); 
	virtual void		Draw(BRect updateRect);
	virtual	void		MouseDown(BPoint where);
	virtual	void		MouseUp(BPoint where);
	virtual	void		MouseMoved(BPoint pt, uint32 code, const BMessage *msg); 
	virtual	void		LanguageChanged();
	virtual	void		MessageReceived(BMessage *message);
	virtual	void		FrameMoved(BPoint new_position);
	virtual	void		FrameResized(float new_width, float new_height);

protected:
	virtual void		Init();
	virtual void		ValueChanged(BMessage *changeMessage);
	BTextControl		*connectionName;
	
	BMessage			*viewMessage;
	BMessage			*from;
	BMessage			*to;
	BPoint				*fromPoint;
	BPoint				*toPoint;
	bool				mirrorX,mirrorY;
private:
};
#endif
