#include "ContextBarTest.h"

#include <vector>

#include "ContextBar.h"

CPPUNIT_TEST_SUITE_REGISTRATION(ContextBarTest);

static const BRect	kView(0, 0, 800, 600);


static void TwoButtons(ContextBar &bar, std::vector<float> &widths)
{
	bar.AddButton("one", new BMessage('one '));
	bar.AddButton("two", new BMessage('two '));
	widths.push_back(30);
	widths.push_back(50);
}


void ContextBarTest::SitsCenteredAboveTheSelection(void)
{
	ContextBar			bar;
	std::vector<float>	widths;
	TwoButtons(bar, widths);
	bar.Layout(BRect(300, 300, 500, 400), kView, widths, 12);
	BRect	frame	= bar.Frame();
	CPPUNIT_ASSERT(frame.bottom < 300);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(400, (frame.left + frame.right) / 2, 0.01);
	CPPUNIT_ASSERT(bar.ButtonFrame(0).right <= bar.ButtonFrame(1).left);
}


void ContextBarTest::GoesBelowWithoutRoomAbove(void)
{
	ContextBar			bar;
	std::vector<float>	widths;
	TwoButtons(bar, widths);
	bar.Layout(BRect(300, 5, 500, 100), kView, widths, 12);
	CPPUNIT_ASSERT(bar.Frame().top > 100);
}


void ContextBarTest::StaysInsideTheView(void)
{
	ContextBar			bar;
	std::vector<float>	widths;
	TwoButtons(bar, widths);
	bar.Layout(BRect(760, 300, 900, 400), kView, widths, 12);
	CPPUNIT_ASSERT(bar.Frame().right <= kView.right);
	bar.Layout(BRect(-100, 300, 20, 400), kView, widths, 12);
	CPPUNIT_ASSERT(bar.Frame().left >= kView.left);
}


void ContextBarTest::HitTestFindsTheButton(void)
{
	ContextBar			bar;
	std::vector<float>	widths;
	TwoButtons(bar, widths);
	bar.Layout(BRect(300, 300, 500, 400), kView, widths, 12);
	BRect	second	= bar.ButtonFrame(1);
	BPoint	center((second.left + second.right) / 2, (second.top + second.bottom) / 2);
	CPPUNIT_ASSERT_EQUAL((int32)1, bar.ButtonAt(center));
	CPPUNIT_ASSERT(bar.MessageAt(1)->what == 'two ');
	CPPUNIT_ASSERT_EQUAL((int32)-1, bar.ButtonAt(BPoint(400, 350)));
}


void ContextBarTest::HiddenBarIsNeverHit(void)
{
	ContextBar			bar;
	std::vector<float>	widths;
	TwoButtons(bar, widths);
	bar.Layout(BRect(300, 300, 500, 400), kView, widths, 12);
	BRect	first	= bar.ButtonFrame(0);
	bar.Hide();
	CPPUNIT_ASSERT_EQUAL((int32)-1, bar.ButtonAt(first.LeftTop() + BPoint(2, 2)));
}
