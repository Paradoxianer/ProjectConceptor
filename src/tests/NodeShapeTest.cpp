#include "NodeShapeTest.h"

#include <DataIO.h>
#include <math.h>
#include <string.h>

#include "NodeShape.h"
#include "ProjectConceptorDefs.h"

static const BRect	kFrame(100, 100, 300, 200);

static NodeShape BuiltIn(const char *name)
{
	BMessage	archive;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK, NodeShape::BuildBuiltIn(name, &archive));
	NodeShape	shape;
	shape.SetTo(&archive);
	shape.Layout(kFrame);
	return shape;
}

static void AssertNear(BPoint expected, BPoint actual)
{
	CPPUNIT_ASSERT_DOUBLES_EQUAL(expected.x, actual.x, 0.5);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(expected.y, actual.y, 0.5);
}


void NodeShapeTest::RoundedHasNoPathAndUsesTheFrame(void)
{
	NodeShape	shape	= BuiltIn("rounded");
	CPPUNIT_ASSERT(!shape.HasPath());
	CPPUNIT_ASSERT(shape.Contains(BPoint(101, 101)));
	AssertNear(BPoint(100, 150), shape.Anchor(BPoint(100, 150)));
	CPPUNIT_ASSERT(shape.TextFrame(kFrame) == kFrame);
}


void NodeShapeTest::EveryBuiltInSurvivesArchiving(void)
{
	for (int32 i = 0; i < NodeShape::CountBuiltIn(); i++) {
		BMessage	archive;
		CPPUNIT_ASSERT_EQUAL((status_t)B_OK,
			NodeShape::BuildBuiltIn(NodeShape::BuiltInName(i), &archive));
		// as it travels in a document
		BMallocIO	flat;
		archive.Flatten(&flat);
		flat.Seek(0, SEEK_SET);
		BMessage	restored;
		CPPUNIT_ASSERT_EQUAL((status_t)B_OK, restored.Unflatten(&flat));
		const char	*name	= NULL;
		CPPUNIT_ASSERT_EQUAL((status_t)B_OK, restored.FindString(P_C_SHAPE_NAME, &name));
		CPPUNIT_ASSERT(strcmp(name, NodeShape::BuiltInName(i)) == 0);
		NodeShape	shape;
		shape.SetTo(&restored);
		shape.Layout(kFrame);
		CPPUNIT_ASSERT(shape.Contains(BPoint(200, 150)));
		CPPUNIT_ASSERT_EQUAL(strcmp(name, "rounded") != 0, shape.HasPath());
	}
}


void NodeShapeTest::DiamondContainsCenterButNotCorner(void)
{
	NodeShape	shape	= BuiltIn("diamond");
	CPPUNIT_ASSERT(shape.Contains(BPoint(200, 150)));
	CPPUNIT_ASSERT(!shape.Contains(BPoint(105, 105)));
	CPPUNIT_ASSERT(!shape.Contains(BPoint(295, 195)));
}


void NodeShapeTest::DiamondAnchorsAtItsTips(void)
{
	NodeShape	shape	= BuiltIn("diamond");
	AssertNear(BPoint(100, 150), shape.Anchor(BPoint(100, 150)));
	AssertNear(BPoint(300, 150), shape.Anchor(BPoint(300, 150)));
	AssertNear(BPoint(200, 100), shape.Anchor(BPoint(200, 100)));
	AssertNear(BPoint(200, 200), shape.Anchor(BPoint(200, 200)));
}


void NodeShapeTest::TriangleAnchorsOnTheSlantedEdge(void)
{
	NodeShape	shape	= BuiltIn("triangle");
	// left edge runs from (100,200) to (200,100); at y=150 that's x=150
	AssertNear(BPoint(150, 150), shape.Anchor(BPoint(100, 150)));
	AssertNear(BPoint(200, 200), shape.Anchor(BPoint(200, 200)));
}


void NodeShapeTest::EllipseAnchorsOnTheCurve(void)
{
	NodeShape	shape	= BuiltIn("ellipse");
	AssertNear(BPoint(300, 150), shape.Anchor(BPoint(300, 150)));
	BPoint	diagonal	= shape.Anchor(BPoint(300, 200));
	// on the ellipse (x-200)^2/100^2 + (y-150)^2/50^2 = 1, within the
	// flattening error
	float	value	= powf((diagonal.x - 200) / 100, 2) + powf((diagonal.y - 150) / 50, 2);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, value, 0.02);
}


void NodeShapeTest::TextFrameFollowsTheShape(void)
{
	NodeShape	shape	= BuiltIn("diamond");
	BRect		text	= shape.TextFrame(kFrame);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(150, text.left, 0.01);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(125, text.top, 0.01);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(250, text.right, 0.01);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(175, text.bottom, 0.01);
}


void NodeShapeTest::UnknownNameIsRefused(void)
{
	BMessage	archive;
	CPPUNIT_ASSERT(NodeShape::BuildBuiltIn("no such shape", &archive) != B_OK);
}
