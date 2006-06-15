#ifndef RENDERER_H
#define RENDERER_H

class GraphEditor;

class Renderer
{

public:
						Renderer(GraphEditor *parentEditor,BMessage *forContainer){editor=parentEditor;container = forContainer;};

	virtual	void		ValueChanged(void)										= 0;


	virtual	void		MouseDown(BPoint where)									= 0;
	virtual	void		MouseMoved(BPoint pt, uint32 code, const BMessage *msg)	= 0;
	virtual	void		MouseUp(BPoint where) 									= 0;

	virtual	void		Draw(BView *drawOn, BRect updateRect)					= 0;
	
	virtual void		SetFrame(BRect newFrame)								= 0;
	virtual BRect		Frame(void)												= 0;
	virtual	void		MoveBy(float dx, float dy)								= 0;
	virtual	void		ResizeBy(float dx,float dy)								= 0;

	virtual bool		Selected(void)											= 0;
	virtual bool		Caught(BPoint where)									= 0;

	virtual	BMessage*	GetMessage(void){return container;};


protected:
			BMessage	*container;
			GraphEditor	*editor;
};
#endif
