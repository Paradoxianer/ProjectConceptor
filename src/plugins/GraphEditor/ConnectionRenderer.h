#ifndef CONNECTION_RENDERER_H
#define CONNECTION_RENDERER_H
/*
 * @author Paradoxon powered by Jesus Christ
 *
 *
 *
 */
#include <app/Message.h>
#include <app/Messenger.h>
#include <interface/View.h>
#include <math.h>
#include <interface/Shape.h>

#include <vector>


const uint32	B_C_NAME_CHANGED	= 'bcNC';

#include "GraphEditor.h"
#include "Renderer.h"


const double	M_PI_3_4	= M_PI_2+M_PI_4;
const float		BEND_LENGTH	= 0.25;


class ClassRenderer;

class ConnectionRenderer: public Renderer
{

public:
						ConnectionRenderer(GraphEditor *parentEditor, BMessage *forContainer);
	virtual void		Draw(BView *drawOn, BRect updateRect);
	virtual	void		MouseDown(BPoint where, int32 buttons =0,
	                              int32 clicks =0,int32 modifiers =0);
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

	virtual bool		Selected(void){return selected;};
	virtual bool		Caught(BPoint where);
	// from/to are resolved once in ValueChanged() and cached until the next
	// broadcast that touches this specific connection - if an endpoint's
	// renderer is deleted in between (its node removed from the document,
	// e.g. via Undo/Delete) without this connection itself being marked
	// changed, the cached pointer goes dangling. GraphEditor::RemoveRenderer()
	// is the single place a renderer ever gets deleted, so it calls this on
	// every connection to null out any reference to what it's about to
	// delete, rather than leaving CalcLine()/Draw() to dereference freed
	// memory on the next redraw.
	virtual	void		InvalidateEndpoint(Renderer *removed);

	virtual	void		SetPreviewColor(const char *field, rgb_color color);
	virtual	void		ClearPreviewColor(void);
	virtual	BCursorID	CursorAt(BPoint where);

	/** index of the bend handle clicked last, -1 for none */
			int32		SelectedBend(void) const {return selectedBend;};
			BPoint		SelectedBendPoint(void) const;
	/** whether the bends are fixed by hand rather than computed */
			bool		HasFixedRoute(void) const {return !bends.empty();};
	/** commands removing the selected bend, or all fixed ones */
			void		DeleteSelectedBend(void);
			void		AutoRoute(void);

protected:
	/** fillColor, or the live preview color while a picker is open and
	 * hasn't committed yet - see the class comment on Renderer's own
	 * SetPreviewColor() for why this exists. Draw*() below always
	 * goes through this instead of reading fillColor directly.
	 */
	rgb_color			EffectiveFillColor(void) {
		return hasPreviewFillColor ? previewFillColor : fillColor;
	}
	/** what the line is drawn in: the standard connection color means
	 * "not set" and becomes the style's; selection uses the accent */
			rgb_color	LineColor(const GraphStyle &style);
			float		LineWidth(const GraphStyle &style);
			void		DrawArrows(BView *drawOn);
			void		Init();
			void		CalcLine();
	/** fromPoint/third on the sides of from/to facing the first/last bend;
	 * the outward directions of those sides */
			void		CalcFixedEnds(const std::vector<BPoint> &route,
							BPoint *fromOutward, BPoint *toOutward);
	/** legs and handles for the computed route */
			void		CalcAutoLegs(void);
	/** legs through route, in the connection's shape */
			void		CalcFixedLegs(const std::vector<BPoint> &route,
							BPoint fromOutward, BPoint toOutward);
			void		DrawRoute(BView *drawOn);
			void		DrawHandles(BView *drawOn);
	/** index into handles/legs under where, -1 for none */
			int32		HandleAt(BPoint where);
			int32		PlusAt(BPoint where);
	/** halfway along leg i; false if the leg is too short for a "+" */
			bool		PlusPoint(int32 i, BPoint *point);
			void		SendBends(const std::vector<BPoint> &points);

			BPoint		PointOnBezier(float t);
			float		Distance(BPoint one, BPoint two);
			float		DistanceToSegment(BPoint p, BPoint segStart, BPoint segEnd);

	bool				selected;
	/** arrow head at the target end: first/second are its base corners,
	 * third its tip on the target's own edge. */
	BPoint				first,second,third;
	/** same triangle mirrored onto the source end, drawn only when
	 * P_C_NODE_CONNECTION_ARROWS asks for it. */
	BPoint				fromFirst,fromSecond,fromThird;
	rgb_color			fillColor;
	bool				hasPreviewFillColor;
	rgb_color			previewFillColor;
	float				penSize;
	ClassRenderer		*from;
	ClassRenderer		*to;
	BPoint				fromPoint;
	BPoint				toPoint;
	/** control points of the computed route: the corners of an angular
	 * line, the bezier controls of a rounded one */
	BPoint				firstBend;
	BPoint				secondBend;
	/** P_C_NODE_CONNECTION_BENDS */
	std::vector<BPoint>	bends;
	/** the bends while a handle is dragged, drawn instead of bends once
	 * the drag changed something */
	std::vector<BPoint>	dragPoints;
	BPoint				dragStart;
	/** the drawn line, one polyline from handle to handle */
	std::vector<std::vector<BPoint> >	legs;
	/** where the bend handles sit: the fixed bends, or the computed
	 * route's bends that dragging one would fix */
	std::vector<BPoint>	handles;
	int32				selectedBend;
	/** the handle being dragged, -1 for none */
	int32				dragBend;
	bool				dragMoved;
	bool				dragAdded;
	bool				mirrorX,mirrorY;
	double				alpha;
//	PCommand			*selectCommand;
	PDocument			*doc;
	BMessenger			*sentTo;
	uint				connectionType;
	/** P_C_NODE_CONNECTION_ARROWS: bit 0 target, bit 1 source. */
	int8				arrows;


private:
};
#endif
