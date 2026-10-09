#ifndef NODE_SHAPE_TEST_H
#define NODE_SHAPE_TEST_H

#include <cppunit/extensions/HelperMacros.h>

/** NodeShape (GraphEditor) is the geometry behind Node::Shape: outline,
 * hit test, where connections dock, where text goes.
 */
class NodeShapeTest : public CppUnit::TestFixture
{
public:
	void RoundedHasNoPathAndUsesTheFrame(void);
	void EveryBuiltInSurvivesArchiving(void);
	void DiamondContainsCenterButNotCorner(void);
	void DiamondAnchorsAtItsTips(void);
	void TriangleAnchorsOnTheSlantedEdge(void);
	void EllipseAnchorsOnTheCurve(void);
	void TextFrameFollowsTheShape(void);
	void UnknownNameIsRefused(void);
	void RectangleBandRunsAlongTheTop(void);
	void DiamondBandRunsAlongTheUpperLeftEdge(void);
	void RoundedOutlineFillsTheFrame(void);

	CPPUNIT_TEST_SUITE(NodeShapeTest);
	CPPUNIT_TEST(RoundedHasNoPathAndUsesTheFrame);
	CPPUNIT_TEST(EveryBuiltInSurvivesArchiving);
	CPPUNIT_TEST(DiamondContainsCenterButNotCorner);
	CPPUNIT_TEST(DiamondAnchorsAtItsTips);
	CPPUNIT_TEST(TriangleAnchorsOnTheSlantedEdge);
	CPPUNIT_TEST(EllipseAnchorsOnTheCurve);
	CPPUNIT_TEST(TextFrameFollowsTheShape);
	CPPUNIT_TEST(UnknownNameIsRefused);
	CPPUNIT_TEST(RectangleBandRunsAlongTheTop);
	CPPUNIT_TEST(DiamondBandRunsAlongTheUpperLeftEdge);
	CPPUNIT_TEST(RoundedOutlineFillsTheFrame);
	CPPUNIT_TEST_SUITE_END();
};

#endif
