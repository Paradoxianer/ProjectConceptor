#include "NodeShape.h"

#include <Catalog.h>
#include <View.h>
#include <math.h>
#include <string.h>

#include "ProjectConceptorDefs.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "GraphEditor"

// circle approximation by four cubic beziers
static const float	kKappa		= 0.5523f;
static const int32	kBezierSteps	= 12;

// accent: the edge (0..1) the node's color band runs along; none means
// along the top
struct BuiltInShape {
	const char	*name;
	const char	*label;
	bool		hasAccent;
	float		ax, ay, bx, by;
};

static const BuiltInShape	kBuiltIn[] = {
	{ "rounded",		B_TRANSLATE_MARK("Rounded rectangle"),	false, 0, 0, 0, 0 },
	{ "rectangle",		B_TRANSLATE_MARK("Rectangle"),			false, 0, 0, 0, 0 },
	{ "ellipse",		B_TRANSLATE_MARK("Ellipse"),			false, 0, 0, 0, 0 },
	{ "diamond",		B_TRANSLATE_MARK("Diamond"),			true, 0, 0.5, 0.5, 0 },
	{ "triangle",		B_TRANSLATE_MARK("Triangle"),			true, 0, 1, 0.5, 0 },
	{ "hexagon",		B_TRANSLATE_MARK("Hexagon"),			false, 0, 0, 0, 0 },
	{ "parallelogram",	B_TRANSLATE_MARK("Parallelogram"),		false, 0, 0, 0, 0 },
	{ "note",			B_TRANSLATE_MARK("Note"),				false, 0, 0, 0, 0 }
};


static void Polygon(BShape &shape, const BPoint *points, int32 count)
{
	shape.MoveTo(points[0]);
	for (int32 i = 1; i < count; i++)
		shape.LineTo(points[i]);
	shape.Close();
}


int32 NodeShape::CountBuiltIn(void)
{
	return sizeof(kBuiltIn) / sizeof(kBuiltIn[0]);
}


const char* NodeShape::BuiltInName(int32 index)
{
	return ((index >= 0) && (index < CountBuiltIn())) ? kBuiltIn[index].name : NULL;
}


const char* NodeShape::BuiltInLabel(int32 index)
{
	return ((index >= 0) && (index < CountBuiltIn())) ? kBuiltIn[index].label : NULL;
}


status_t NodeShape::BuildBuiltIn(const char *name, BMessage *archive)
{
	if ((name == NULL) || (archive == NULL))
		return B_BAD_VALUE;
	archive->MakeEmpty();
	BShape	shape;
	BRect	textRect(0, 0, 1, 1);
	if (strcmp(name, "rounded") == 0) {
		archive->AddString(P_C_SHAPE_NAME, name);
		return B_OK;
	} else if (strcmp(name, "rectangle") == 0) {
		const BPoint	points[] = { BPoint(0, 0), BPoint(1, 0), BPoint(1, 1), BPoint(0, 1) };
		Polygon(shape, points, 4);
	} else if (strcmp(name, "ellipse") == 0) {
		const float	k	= kKappa * 0.5f;
		shape.MoveTo(BPoint(0.5, 0));
		BPoint	q1[] = { BPoint(0.5 + k, 0), BPoint(1, 0.5 - k), BPoint(1, 0.5) };
		BPoint	q2[] = { BPoint(1, 0.5 + k), BPoint(0.5 + k, 1), BPoint(0.5, 1) };
		BPoint	q3[] = { BPoint(0.5 - k, 1), BPoint(0, 0.5 + k), BPoint(0, 0.5) };
		BPoint	q4[] = { BPoint(0, 0.5 - k), BPoint(0.5 - k, 0), BPoint(0.5, 0) };
		shape.BezierTo(q1);
		shape.BezierTo(q2);
		shape.BezierTo(q3);
		shape.BezierTo(q4);
		shape.Close();
		textRect.Set(0.15, 0.15, 0.85, 0.85);
	} else if (strcmp(name, "diamond") == 0) {
		const BPoint	points[] = { BPoint(0.5, 0), BPoint(1, 0.5), BPoint(0.5, 1), BPoint(0, 0.5) };
		Polygon(shape, points, 4);
		textRect.Set(0.25, 0.25, 0.75, 0.75);
	} else if (strcmp(name, "triangle") == 0) {
		const BPoint	points[] = { BPoint(0.5, 0), BPoint(1, 1), BPoint(0, 1) };
		Polygon(shape, points, 3);
		textRect.Set(0.25, 0.45, 0.75, 0.95);
	} else if (strcmp(name, "hexagon") == 0) {
		const BPoint	points[] = { BPoint(0.25, 0), BPoint(0.75, 0), BPoint(1, 0.5),
			BPoint(0.75, 1), BPoint(0.25, 1), BPoint(0, 0.5) };
		Polygon(shape, points, 6);
		textRect.Set(0.15, 0, 0.85, 1);
	} else if (strcmp(name, "parallelogram") == 0) {
		const BPoint	points[] = { BPoint(0.2, 0), BPoint(1, 0), BPoint(0.8, 1), BPoint(0, 1) };
		Polygon(shape, points, 4);
		textRect.Set(0.15, 0, 0.85, 1);
	} else if (strcmp(name, "note") == 0) {
		const BPoint	points[] = { BPoint(0, 0), BPoint(0.85, 0), BPoint(1, 0.15),
			BPoint(1, 1), BPoint(0, 1) };
		Polygon(shape, points, 5);
	} else
		return B_NAME_NOT_FOUND;

	status_t	err	= shape.Archive(archive);
	if (err != B_OK)
		return err;
	archive->AddString(P_C_SHAPE_NAME, name);
	archive->AddRect(P_C_SHAPE_TEXT_RECT, textRect);
	return B_OK;
}


class ShapeCollector : public BShapeIterator {
public:
	ShapeCollector(std::vector<NodeShape::Op> &ops) : fOps(ops) {}

	virtual status_t IterateMoveTo(BPoint *point)
	{
		NodeShape::Op	op	= { NodeShape::OP_MOVE, { *point } };
		fOps.push_back(op);
		return B_OK;
	}

	virtual status_t IterateLineTo(int32 lineCount, BPoint *linePoints)
	{
		for (int32 i = 0; i < lineCount; i++) {
			NodeShape::Op	op	= { NodeShape::OP_LINE, { linePoints[i] } };
			fOps.push_back(op);
		}
		return B_OK;
	}

	virtual status_t IterateBezierTo(int32 bezierCount, BPoint *bezierPoints)
	{
		for (int32 i = 0; i < bezierCount; i++) {
			NodeShape::Op	op	= { NodeShape::OP_BEZIER,
				{ bezierPoints[i * 3], bezierPoints[i * 3 + 1], bezierPoints[i * 3 + 2] } };
			fOps.push_back(op);
		}
		return B_OK;
	}

	virtual status_t IterateClose(void)
	{
		NodeShape::Op	op	= { NodeShape::OP_CLOSE, {} };
		fOps.push_back(op);
		return B_OK;
	}

private:
	std::vector<NodeShape::Op>	&fOps;
};


NodeShape::NodeShape(void)
	:
	fTextRect(0, 0, 1, 1),
	fHasAccent(false)
{
}


void NodeShape::SetTo(const BMessage *archive)
{
	fOps.clear();
	fPolygons.clear();
	fScaled.Clear();
	fTextRect.Set(0, 0, 1, 1);
	fName	= "rounded";
	fHasAccent	= false;
	if (archive == NULL)
		return;
	const char	*name	= NULL;
	if (archive->FindString(P_C_SHAPE_NAME, &name) == B_OK)
		fName	= name;
	for (int32 i = 0; i < CountBuiltIn(); i++) {
		if ((fName == kBuiltIn[i].name) && kBuiltIn[i].hasAccent) {
			fHasAccent	= true;
			fAccentFrom	= BPoint(kBuiltIn[i].ax, kBuiltIn[i].ay);
			fAccentTo	= BPoint(kBuiltIn[i].bx, kBuiltIn[i].by);
		}
	}
	BMessage	copy(*archive);
	BShape		shape(&copy);
	ShapeCollector	collector(fOps);
	collector.Iterate(&shape);
	BRect	textRect;
	if ((archive->FindRect(P_C_SHAPE_TEXT_RECT, &textRect) == B_OK) && textRect.IsValid())
		fTextRect = textRect;
	Layout(fFrame);
}


void NodeShape::Layout(BRect frame)
{
	fFrame	= frame;
	fScaled.Clear();
	fPolygons.clear();
	if (!HasPath() || !frame.IsValid())
		return;
	const float	width	= frame.Width();
	const float	height	= frame.Height();
	BPoint	current(0, 0);
	BPoint	start(0, 0);
	for (size_t i = 0; i < fOps.size(); i++) {
		const Op	&op	= fOps[i];
		BPoint	scaled[3];
		for (int32 j = 0; j < 3; j++)
			scaled[j] = BPoint(op.points[j].x * width, op.points[j].y * height);
		switch (op.kind) {
			case OP_MOVE:
				fScaled.MoveTo(scaled[0]);
				fPolygons.push_back(std::vector<BPoint>());
				fPolygons.back().push_back(scaled[0] + frame.LeftTop());
				current	= scaled[0];
				start	= scaled[0];
				break;
			case OP_LINE:
				fScaled.LineTo(scaled[0]);
				if (fPolygons.empty())
					fPolygons.push_back(std::vector<BPoint>());
				fPolygons.back().push_back(scaled[0] + frame.LeftTop());
				current	= scaled[0];
				break;
			case OP_BEZIER: {
				fScaled.BezierTo(scaled);
				if (fPolygons.empty())
					fPolygons.push_back(std::vector<BPoint>());
				for (int32 step = 1; step <= kBezierSteps; step++) {
					float	t	= (float)step / kBezierSteps;
					float	u	= 1 - t;
					BPoint	p(
						u * u * u * current.x + 3 * u * u * t * scaled[0].x
							+ 3 * u * t * t * scaled[1].x + t * t * t * scaled[2].x,
						u * u * u * current.y + 3 * u * u * t * scaled[0].y
							+ 3 * u * t * t * scaled[1].y + t * t * t * scaled[2].y);
					fPolygons.back().push_back(p + frame.LeftTop());
				}
				current	= scaled[2];
				break;
			}
			case OP_CLOSE:
				fScaled.Close();
				current	= start;
				break;
		}
	}
}


bool NodeShape::Contains(BPoint where) const
{
	if (!HasPath())
		return fFrame.Contains(where);
	// even-odd rule over every subpath
	bool	inside	= false;
	for (size_t p = 0; p < fPolygons.size(); p++) {
		const std::vector<BPoint>	&polygon	= fPolygons[p];
		size_t	count	= polygon.size();
		for (size_t i = 0, j = count - 1; i < count; j = i++) {
			const BPoint	&a	= polygon[i];
			const BPoint	&b	= polygon[j];
			if (((a.y > where.y) != (b.y > where.y))
				&& (where.x < (b.x - a.x) * (where.y - a.y) / (b.y - a.y) + a.x))
				inside = !inside;
		}
	}
	return inside;
}


BPoint NodeShape::Anchor(BPoint toward) const
{
	if (!HasPath())
		return toward;
	BPoint	center((fFrame.left + fFrame.right) / 2, (fFrame.top + fFrame.bottom) / 2);
	BPoint	d	= toward - center;
	float	best	= -1;
	for (size_t p = 0; p < fPolygons.size(); p++) {
		const std::vector<BPoint>	&polygon	= fPolygons[p];
		size_t	count	= polygon.size();
		for (size_t i = 0, j = count - 1; i < count; j = i++) {
			BPoint	a		= polygon[j];
			BPoint	e		= polygon[i] - a;
			float	denom	= d.x * e.y - d.y * e.x;
			if ((denom > -1e-6) && (denom < 1e-6))
				continue;
			BPoint	w		= a - center;
			float	u		= (w.x * e.y - w.y * e.x) / denom;
			float	v		= (w.x * d.y - w.y * d.x) / denom;
			if ((u > 0) && (v >= 0) && (v <= 1) && ((best < 0) || (u < best)))
				best = u;
		}
	}
	if (best < 0)
		return toward;
	return BPoint(center.x + best * d.x, center.y + best * d.y);
}


void NodeShape::Fill(BView *view, BPoint offset) const
{
	for (size_t p = 0; p < fPolygons.size(); p++) {
		std::vector<BPoint>	points(fPolygons[p]);
		for (size_t i = 0; i < points.size(); i++)
			points[i] += offset;
		if (points.size() >= 3)
			view->FillPolygon(&points[0], points.size());
	}
}


void NodeShape::Stroke(BView *view, BPoint offset) const
{
	for (size_t p = 0; p < fPolygons.size(); p++) {
		std::vector<BPoint>	points(fPolygons[p]);
		for (size_t i = 0; i < points.size(); i++)
			points[i] += offset;
		if (points.size() >= 2)
			view->StrokePolygon(&points[0], points.size(), true);
	}
}


BRect NodeShape::TextFrame(BRect frame) const
{
	if (!HasPath())
		return frame;
	return BRect(frame.left + fTextRect.left * frame.Width(),
		frame.top + fTextRect.top * frame.Height(),
		frame.left + fTextRect.right * frame.Width(),
		frame.top + fTextRect.bottom * frame.Height());
}


void NodeShape::Outline(float cornerRadius, std::vector<BPoint> *points) const
{
	points->clear();
	if (HasPath()) {
		if (!fPolygons.empty())
			*points = fPolygons[0];
		return;
	}
	if (!fFrame.IsValid())
		return;
	float	radius	= cornerRadius;
	if (radius > fFrame.Width() / 2)
		radius = fFrame.Width() / 2;
	if (radius > fFrame.Height() / 2)
		radius = fFrame.Height() / 2;
	const int32	steps	= 8;
	const BPoint	centers[] = {
		BPoint(fFrame.right - radius, fFrame.top + radius),
		BPoint(fFrame.right - radius, fFrame.bottom - radius),
		BPoint(fFrame.left + radius, fFrame.bottom - radius),
		BPoint(fFrame.left + radius, fFrame.top + radius) };
	// clockwise on screen, starting at the top right corner's arc
	for (int32 corner = 0; corner < 4; corner++) {
		float	start	= -M_PI / 2 + corner * M_PI / 2;
		for (int32 i = 0; i <= steps; i++) {
			float	angle	= start + (M_PI / 2) * i / steps;
			points->push_back(BPoint(centers[corner].x + radius * cosf(angle),
				centers[corner].y + radius * sinf(angle)));
		}
	}
}


// keeps the part of polygon where (p - a) . normal + offset <= 0
static void ClipHalfPlane(const std::vector<BPoint> &polygon, BPoint a,
	BPoint normal, float offset, std::vector<BPoint> *out)
{
	out->clear();
	size_t	count	= polygon.size();
	for (size_t i = 0; i < count; i++) {
		const BPoint	&p	= polygon[i];
		const BPoint	&q	= polygon[(i + 1) % count];
		float	dp	= (p.x - a.x) * normal.x + (p.y - a.y) * normal.y + offset;
		float	dq	= (q.x - a.x) * normal.x + (q.y - a.y) * normal.y + offset;
		if (dp <= 0)
			out->push_back(p);
		if ((dp < 0) != (dq < 0) && (dp != dq)) {
			float	t	= dp / (dp - dq);
			out->push_back(BPoint(p.x + t * (q.x - p.x), p.y + t * (q.y - p.y)));
		}
	}
}


void NodeShape::AccentBand(const std::vector<BPoint> &outline, float thickness,
	std::vector<BPoint> *band) const
{
	band->clear();
	if (outline.size() < 3)
		return;
	// a point on the edge and the direction into the shape
	BPoint	onEdge;
	BPoint	inward(0, 1);
	if (fHasAccent) {
		BPoint	a(fFrame.left + fAccentFrom.x * fFrame.Width(),
			fFrame.top + fAccentFrom.y * fFrame.Height());
		BPoint	b(fFrame.left + fAccentTo.x * fFrame.Width(),
			fFrame.top + fAccentTo.y * fFrame.Height());
		BPoint	edge	= b - a;
		float	length	= sqrtf(edge.x * edge.x + edge.y * edge.y);
		if (length < 1e-4)
			return;
		inward	= BPoint(-edge.y / length, edge.x / length);
		BPoint	center((fFrame.left + fFrame.right) / 2, (fFrame.top + fFrame.bottom) / 2);
		if ((center.x - a.x) * inward.x + (center.y - a.y) * inward.y < 0)
			inward = BPoint(-inward.x, -inward.y);
		onEdge	= a;
	} else {
		onEdge	= outline[0];
		for (size_t i = 1; i < outline.size(); i++) {
			if (outline[i].y < onEdge.y)
				onEdge = outline[i];
		}
	}
	ClipHalfPlane(outline, onEdge, inward, -thickness, band);
}
