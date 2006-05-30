#ifndef ITEM_RENDERER_H
#define ITEM_RENDERER_H

class GraphEditor;

class ItemRenderer
{

public:

	virtual	void		MouseDown(BPoint where)									= 0;
	virtual	void		MouseUp(BPoint where) 									= 0;

	virtual	void		Draw(BView *drawOn, BRect updateRect)					= 0;

	virtual void		SetFrame(BRect newRect)									= 0;
	virtual BRect		Frame(void)												= 0;
	virtual	void		MoveBy(float dx, float dy)								= 0;
	virtual	void		ResizeBy(float dx,float dy)								= 0;


protected:
};
#endif
