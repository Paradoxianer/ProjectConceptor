#ifndef RENDERER_H
#define RENDERER_H

class GraphEditor;

class Renderer
{

public:
						Renderer(GraphEditor *parentEditor, BMessage *forContainer){editor = parentEditor; container = forContainer;};

	virtual	void		ValueChanged(void)										= 0;


	virtual	void		MouseDown(BPoint where, int32 buttons =0,
	                              int32 clicks =0,int32 modifiers =0)			= 0;
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

	/** Live-preview color while a picker is open, without touching the
	 * underlying node data - mirrors how Move/Resize preview a drag by
	 * only updating renderer geometry, committing a single real command
	 * at the end (see docs/notes.md). Non-pure with an empty default so
	 * only renderers that actually draw a fill color (ClassRenderer/
	 * ConnectionRenderer) need to override it - everything else (labels,
	 * attribute rows, ...) is unaffected. ClearPreviewFillColor() is
	 * called automatically from within each overriding renderer's own
	 * ValueChanged() once a real committed value arrives, so callers
	 * never need to explicitly clear it themselves.
	 */
	virtual	void		SetPreviewFillColor(rgb_color color) {};
	virtual	void		ClearPreviewFillColor(void) {};

	/** Advances a renderer's own in-flight position animation (e.g. after
	 * Auto-Layout) by dt seconds. Returns true while still animating -
	 * GraphEditor calls this every tick from its shared animation runner
	 * (see GraphEditor::StartAnimating()) and drops the renderer once it
	 * returns false. Non-pure/no-op default: only ClassRenderer overrides
	 * it for now.
	 */
	virtual	bool		AnimationStep(float dt) {return false;};

protected:
			BMessage	*container;
			GraphEditor	*editor;
};
#endif
