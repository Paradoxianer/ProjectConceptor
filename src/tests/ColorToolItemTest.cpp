#include "ColorToolItemTest.h"

#include <string.h>

#include "ColorToolItem.h"

CPPUNIT_TEST_SUITE_REGISTRATION(ColorToolItemTest);

static const rgb_color	kRed	= { 200, 40, 40, 255 };
static const rgb_color	kGrey	= { 120, 120, 120, 255 };
static const rgb_color	kBlack	= { 0, 0, 0, 255 };


// picking a target is the picker window's job; tests set it directly
class TestColorToolItem : public ColorToolItem {
public:
	TestColorToolItem() : ColorToolItem("color", kGrey, new BMessage('test')) {}
	void Choose(int32 index) { currentTarget = index; }
};


static void AddNodeTargets(ColorToolItem &item)
{
	item.ClearTargets();
	item.AddTarget("Fill", "FillColor", kRed, kGrey);
	item.AddTarget("Border", "BorderColor", kGrey, kGrey);
	item.AddTarget("Text", "HighColor", kBlack, kBlack);
}


void ColorToolItemTest::WithoutTargetsTheFillIsColored(void)
{
	TestColorToolItem	item;
	CPPUNIT_ASSERT(strcmp(item.TargetField(), "FillColor") == 0);
}


void ColorToolItemTest::ChosenTargetSurvivesARebuild(void)
{
	TestColorToolItem	item;
	AddNodeTargets(item);
	item.Choose(1);
	CPPUNIT_ASSERT(strcmp(item.TargetField(), "BorderColor") == 0);
	// a new selection rebuilds the targets; border stays chosen
	AddNodeTargets(item);
	CPPUNIT_ASSERT(strcmp(item.TargetField(), "BorderColor") == 0);
	// a connection has only its line: back to the first target
	item.ClearTargets();
	item.AddTarget("Line", "FillColor", kRed, kRed);
	CPPUNIT_ASSERT(strcmp(item.TargetField(), "FillColor") == 0);
}


void ColorToolItemTest::ColorForFindsTheField(void)
{
	TestColorToolItem	item;
	AddNodeTargets(item);
	rgb_color	fill	= item.ColorFor("FillColor");
	CPPUNIT_ASSERT(fill.red == kRed.red && fill.green == kRed.green);
	rgb_color	text	= item.ColorFor("HighColor");
	CPPUNIT_ASSERT(text.red == 0 && text.blue == 0);
}
