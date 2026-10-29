#include "StatusTextsTest.h"

#include "StatusTexts.h"

CPPUNIT_TEST_SUITE_REGISTRATION(StatusTextsTest);


void StatusTextsTest::NothingSelected(void)
{
	CPPUNIT_ASSERT(StatusTexts::Selection(0, 0) == "No selection");
}


void StatusTextsTest::CountsUseSingularAndPlural(void)
{
	CPPUNIT_ASSERT(StatusTexts::Selection(1, 0) == "1 node");
	CPPUNIT_ASSERT(StatusTexts::Selection(3, 0) == "3 nodes");
	CPPUNIT_ASSERT(StatusTexts::Selection(0, 1) == "1 connection");
	CPPUNIT_ASSERT(StatusTexts::Selection(2, 5) == "2 nodes, 5 connections");
}


void StatusTextsTest::PointerIsRounded(void)
{
	CPPUNIT_ASSERT(StatusTexts::Pointer(BPoint(-3.6, 7.5)) == "Pointer -4, 8");
}
