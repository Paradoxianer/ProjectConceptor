#ifndef NODE_SHAPE_H
#define NODE_SHAPE_H

#include <Message.h>
#include <Rect.h>
#include <Shape.h>
#include <String.h>

class BView;

#include <vector>

/** A node's outline. Node::Shape holds an archived BShape whose points are
 * in 0..1 of the node's frame, plus P_C_SHAPE_NAME and an optional
 * P_C_SHAPE_TEXT_RECT (also 0..1) for where name and attributes go. A
 * Node::Shape without a path is the classic rounded rectangle.
 */
class NodeShape
{
public:
							NodeShape(void);

	/** Names of the built-in shapes, in toolbar order. */
	static	int32			CountBuiltIn(void);
	static	const char*		BuiltInName(int32 index);
	/** English label, for B_TRANSLATE by the caller. */
	static	const char*		BuiltInLabel(int32 index);
	/** Fills archive with the built-in shape called name. */
	static	status_t		BuildBuiltIn(const char *name, BMessage *archive);

			/** Reads a Node::Shape message; anything without a path
			 * becomes the classic rounded rectangle. */
			void			SetTo(const BMessage *archive);
			bool			HasPath(void) const {return !fOps.empty();};
			const char*		Name(void) const {return fName.String();};

			/** Scales the shape to frame. Shape() is then relative to
			 * frame.LeftTop() (BView draws shapes at the pen location),
			 * Contains()/Anchor() work in frame's coordinates. */
			void			Layout(BRect frame);
			BShape*			Shape(void) {return &fScaled;};
			/** Fills/strokes the outline. Uses polygons rather than the
			 * BShape: FillShape()/StrokeShape() ignore BView::SetScale(). */
			void			Fill(BView *view, BPoint offset = BPoint(0, 0)) const;
			void			Stroke(BView *view, BPoint offset = BPoint(0, 0)) const;
			bool			Contains(BPoint where) const;
			/** Where a ray from the frame's center toward toward leaves the
			 * outline; toward itself if the shape has no path. */
			BPoint			Anchor(BPoint toward) const;
			/** The part of frame meant for name and attributes. */
			BRect			TextFrame(BRect frame) const;
			/** The laid-out outline as one polygon; without a path the
			 * rounded rectangle with cornerRadius. */
			void			Outline(float cornerRadius,
								std::vector<BPoint> *points) const;

			/** The part of outline within thickness of the shape's accent
			 * edge - the top, or for shapes without a top edge (diamond,
			 * triangle) the upper left one: the node's color band. */
			void			AccentBand(const std::vector<BPoint> &outline,
								float thickness, std::vector<BPoint> *band) const;

private:
	enum op_kind {
		OP_MOVE,
		OP_LINE,
		OP_BEZIER,
		OP_CLOSE
	};
	struct Op {
		op_kind				kind;
		BPoint				points[3];
	};
	friend class ShapeCollector;

			std::vector<Op>		fOps;
			BString				fName;
			BRect				fTextRect;
			bool				fHasAccent;
			BPoint				fAccentFrom;
			BPoint				fAccentTo;
			BRect				fFrame;
			BShape				fScaled;
			// one closed polygon per subpath, beziers flattened, in frame
			// coordinates
			std::vector<std::vector<BPoint> >	fPolygons;
};

#endif
