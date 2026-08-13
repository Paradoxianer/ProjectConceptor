#ifndef PCOMMAND_TEST_H
#define PCOMMAND_TEST_H

#include <cppunit/extensions/HelperMacros.h>

/** PCommand's contract is that Do() followed by Undo() restores the exact
 * prior state - this checks that holds for ChangeValue, without needing a
 * running app (PCommand::Do/Undo only need doc->GetChangedNodes()/
 * GetSelected()/SetModified(), all safe on a headless PDocument).
 */
class PCommandTest : public CppUnit::TestFixture
{
public:
	void ChangeValueDoUndo(void);
	void GroupThenInsertChildRegistersInParentList(void);

	CPPUNIT_TEST_SUITE(PCommandTest);
	CPPUNIT_TEST(ChangeValueDoUndo);
	CPPUNIT_TEST(GroupThenInsertChildRegistersInParentList);
	CPPUNIT_TEST_SUITE_END();
};

#endif
