#ifndef CONNECTION_RENDERER_H
#define CONNECTION_RENDERER_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <app/Message.h>
#include <app/Messenger.h>
#include <interface/View.h>
#include <math.h>

const uint32	B_C_NAME_CHANGED	= 'bcNC';

#include "GraphEditor.h"
#include "Renderer.h"


const double M_PI_3_4	= M_PI_2+M_PI_4;

class ConnectionRenderer: public Renderer
{

public:
						ConnectionRenderer(GraphEditor *parentEditor,BMessage *forContainer);
	virtual void		Draw(BView *drawOn, BRect updateRect);
	virtual	void		MouseDown(BPoint where);
	virtual	void		MouseUp(BPoint where);
	virtual	void		MouseMoved(BPoint pt, uint32 code, const BMessage *msg); 
	virtual	void		LanguageChanged();
	virtual	void		MessageReceived(BMessage *message);
	virtual	void		ValueChanged(void);

	//**implement this
	virtual void		SetFrame(BRect newFrame){};
	virtual BRect		Frame(void);
	//**implement this
	virtual	void		MoveBy(float dx, float dy){};
	//**implement this
	virtual	void		ResizeBy(float dx,float dy){};

	virtual bool		Selected(void){};
	virtual bool		Caught(BPoint where);

protected:
	virtual void		Init();
	bool				selected;
	BPoint				first,second,third;
	rgb_color			fillColor;
	BMessage			*from;
	BMessage			*to;
	BPoint				fromPoint;
	BPoint				toPoint;
	bool				mirrorX,mirrorY;
	double				alpha;
	double				ax,mx;
	double				ay,my;
//	PCommand			*selectCommand;
	PDocument			*doc;
	BMessenger			*sentTo;
	

private:
};
#endif
