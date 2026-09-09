#ifndef LAYOUT_EDITOR_TEST_H
#define LAYOUT_EDITOR_TEST_H

#include <cppunit/extensions/HelperMacros.h>

/** No toolbar-click test: `hey` can't trigger this BButton (click handled
 * in MouseDown/MouseUp, not BControl's value-scripting hook). No
 * SendMessage()-dispatch test either: crashes a leaked PDocument's looper
 * post-teardown, see #117. Batch::Do()/Undo() called directly instead.
 */
class LayoutEditorTest : public CppUnit::TestFixture
{
public:
	void BuildLayoutCommandShapesOneUndoableBatch(void);
	void CenterOnOldBoundsShiftsToMatchOldCenter(void);
	void BatchAppliesAndUndoesAllSubcommands(void);

	CPPUNIT_TEST_SUITE(LayoutEditorTest);
	CPPUNIT_TEST(BuildLayoutCommandShapesOneUndoableBatch);
	CPPUNIT_TEST(CenterOnOldBoundsShiftsToMatchOldCenter);
	CPPUNIT_TEST(BatchAppliesAndUndoesAllSubcommands);
	CPPUNIT_TEST_SUITE_END();
};

#endif
