#ifndef BASIC_GROUP_VIEW_H
#define BASIC_GROUP_VIEW_H

#include <interface/Box.h>
#include "BasicEditor.h"

class BasicGroupView : public BBox
{

public:
						BasicGroupView(BMessage *message);
	virtual void		AttachedToWindow(); 
	virtual void		Draw(BRect updateRect);
	virtual void		DrawAfterChildren(BRect updateRect);

	virtual	void		MouseDown(BPoint where);
	virtual	void		MouseUp(BPoint where);
	virtual	void		MouseMoved(BPoint pt, uint32 code, const BMessage *msg); 
//	virtual	void		LanguageChanged();
	virtual	void		MessageReceived(BMessage *message);
	virtual	void		FrameMoved(BPoint new_position);
	virtual	void		FrameResized(float new_width, float new_height);
protected:
	virtual void		Init();

protected:
			BRect			*selectRect;
			bool			key_hold;
			BPoint			*startMouseDown;
			bool			connecting;	
			BRect			*fromRect;
			BPoint			*toPoint;
			BMessage		*objectMessage;

private:
};
#endif
