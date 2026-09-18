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
	void ChangeValueOnSelectionDoUndo(void);
	void ChangeValueOnConnectionPattern(void);
	void GroupThenInsertChildRegistersInParentList(void);
	void GroupUndoThenRedoKeepsChildren(void);
	void WrapperUndoRestoresAllSubcommands(void);
	void MoveGroupWithSelectedChildrenMovesOnce(void);
	void ExecuteViaRealMessageDispatchSurvivesProcessExit(void);
	void DirectManipulationOnSelectionNormalizedForRecording(void);
	void DirectManipulationOffSelectionKeepsExplicitNodeForRecording(void);
	void FindDefaultScopeSearchesOnlyNodes(void);
	void FindScopeBothIncludesConnections(void);
	void FindSetOperationAddUnionsWithSelection(void);
	void FindSetOperationSubtractRemovesMatches(void);
	void FindSetOperationIntersectKeepsOnlyMatches(void);
	void FindDoUndoRestoresExactPriorSelection(void);

	CPPUNIT_TEST_SUITE(PCommandTest);
	CPPUNIT_TEST(ChangeValueDoUndo);
	CPPUNIT_TEST(ChangeValueOnSelectionDoUndo);
	CPPUNIT_TEST(ChangeValueOnConnectionPattern);
	CPPUNIT_TEST(GroupThenInsertChildRegistersInParentList);
	CPPUNIT_TEST(GroupUndoThenRedoKeepsChildren);
	CPPUNIT_TEST(WrapperUndoRestoresAllSubcommands);
	CPPUNIT_TEST(MoveGroupWithSelectedChildrenMovesOnce);
	CPPUNIT_TEST(ExecuteViaRealMessageDispatchSurvivesProcessExit);
	CPPUNIT_TEST(DirectManipulationOnSelectionNormalizedForRecording);
	CPPUNIT_TEST(DirectManipulationOffSelectionKeepsExplicitNodeForRecording);
	CPPUNIT_TEST(FindDefaultScopeSearchesOnlyNodes);
	CPPUNIT_TEST(FindScopeBothIncludesConnections);
	CPPUNIT_TEST(FindSetOperationAddUnionsWithSelection);
	CPPUNIT_TEST(FindSetOperationSubtractRemovesMatches);
	CPPUNIT_TEST(FindSetOperationIntersectKeepsOnlyMatches);
	CPPUNIT_TEST(FindDoUndoRestoresExactPriorSelection);
	CPPUNIT_TEST_SUITE_END();
};

#endif
