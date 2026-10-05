#ifndef MACRO_TEXT_TEST_H
#define MACRO_TEXT_TEST_H

#include <cppunit/extensions/HelperMacros.h>

/** MacroText.cpp (#55) - the guided text DSL a recorded macro's
 * "Macro::Commmand" list round-trips through (still used for Macro >
 * Save/Open - see MacroText.h), plus the BMessage-tree helpers
 * (BuildInsertPrototype/HighestReferencedId/AssignInsertId/
 * FormatFieldValue/ParseFieldValue) the tree editor (MacroOutlineView)
 * shares with it. Pure functions, exercised directly here the same way
 * GroupBoundaryTest/LayoutEditorTest exercise their own pure-geometry/
 * pure-logic targets - no running editor view needed.
 */
class MacroTextTest : public CppUnit::TestFixture
{
public:
	void RoundTripsInsertWithNodeRef(void);
	void RoundTripsMoveWithFloats(void);
	void RoundTripsGroupWithBoolAndNodeRef(void);
	void RoundTripsSelectWithRepeatedFields(void);
	void RoundTripsNestedSubCommand(void);
	void RoundTripsStringField(void);
	void RoundTripsNestedFieldBlock(void);
	void RoundTripsRecursiveNestedFieldBlock(void);
	void RoundTripsRepeatedNestedFieldBlocks(void);
	void RawEscapeHatchPreservesOpaqueType(void);
	void EachFieldRendersOnItsOwnLine(void);
	void MultipleTokensOnOneLineIsRejected(void);
	void UnknownCommandNameIsRejected(void);
	void UnknownFieldIsRejected(void);
	void TypeMismatchIsRejected(void);
	void RoundTripsBoundField(void);
	void BindingInsideFieldBlockIsRejected(void);
	void EmptyVariableNameIsRejected(void);
	void PropertyInfoAcceptsNodeSelectedForSelectionDrivenCommands(void);
	void GeneratedAddAttributeSnippetParses(void);
	void FindThenAddAttributeReachesEveryFoundNode(void);
	void EveryCommandExampleParses(void);
	void IncludedNodeKeepsItsMessageType(void);
	void TypeCodesReadAsNames(void);
	void InsertPrototypeParsesAndKeepsNodeShape(void);
	void DroppedPrototypesGetDistinctIds(void);
	void RepeatedInsertCreatesDistinctNodes(void);
	void MacroSurvivesDocumentSaveAndLoad(void);
	void PlayMacroReportsWhatHappened(void);
	void FormatAndParseFieldValueRoundTripEveryType(void);
	void CalculateWritesResultIntoValueContext(void);
	void CalculateSupportsEveryOperator(void);
	void CalculateExampleParses(void);
	void InsertResultVariableEnablesFollowUpReference(void);
	void CalculatedNodeIdBindsAsInt32NotFloat(void);
	void DeleteSelectedRowsRemovesOnlyChosenSiblings(void);
	void DeleteSelectedRowsSkipsDescendantsOfAnotherSelectedRow(void);
	void AddNamedFieldAddsCustomFieldToGenericBlock(void);
	void AddFieldMenuOffersRepeatableFieldAgain(void);
	void MoveCommandRowsMovesSeveralTogetherInOrder(void);
	void MoveCommandRowsOntoContainerAppendsAtEnd(void);
	void AddNodeReferenceWiresChipIdIntoTargetCommand(void);
	void AddNodeReferenceRefusesCommandWithNoNodeField(void);
	void ReplaceNodeReferenceFieldOverwritesExistingValue(void);
	void InterpolatesCounterIntoInsertedNodeNames(void);
	void InterpolatedNodeStaysReachableById(void);
	void UnknownPlaceholderIsReported(void);
	void ConnectionFollowsInterpolatedNodes(void);
	void PlaceholderAloneIsNotABinding(void);
	void InterpolatesCommandSettingsInNestedBlocks(void);
	void LayoutKeepsGraphInsideGrownCanvas(void);
	void SelectConnectedFollowsConnections(void);
	void GetValueSumsConnectedAttribute(void);
	void GetValueReportsMissingAttribute(void);
	void TextThatIsNoNumberIsReported(void);
	void GetValueReadsEditorAttribute(void);
	void DecimalCommaIsANumber(void);
	void ExpandAndCollapseAllSurviveRebuild(void);
	void MoveCommandReparentsIntoAnotherContainer(void);
	void MoveCommandPromotesToTopLevel(void);
	void MoveCommandRefusesDroppingIntoOwnSubtree(void);
	void MoveCommandReordersTopLevelSiblings(void);

	CPPUNIT_TEST_SUITE(MacroTextTest);
	CPPUNIT_TEST(RoundTripsInsertWithNodeRef);
	CPPUNIT_TEST(RoundTripsMoveWithFloats);
	CPPUNIT_TEST(RoundTripsGroupWithBoolAndNodeRef);
	CPPUNIT_TEST(RoundTripsSelectWithRepeatedFields);
	CPPUNIT_TEST(RoundTripsNestedSubCommand);
	CPPUNIT_TEST(RoundTripsStringField);
	CPPUNIT_TEST(RoundTripsNestedFieldBlock);
	CPPUNIT_TEST(RoundTripsRecursiveNestedFieldBlock);
	CPPUNIT_TEST(RoundTripsRepeatedNestedFieldBlocks);
	CPPUNIT_TEST(RawEscapeHatchPreservesOpaqueType);
	CPPUNIT_TEST(EachFieldRendersOnItsOwnLine);
	CPPUNIT_TEST(MultipleTokensOnOneLineIsRejected);
	CPPUNIT_TEST(UnknownCommandNameIsRejected);
	CPPUNIT_TEST(UnknownFieldIsRejected);
	CPPUNIT_TEST(TypeMismatchIsRejected);
	CPPUNIT_TEST(RoundTripsBoundField);
	CPPUNIT_TEST(BindingInsideFieldBlockIsRejected);
	CPPUNIT_TEST(EmptyVariableNameIsRejected);
	CPPUNIT_TEST(PropertyInfoAcceptsNodeSelectedForSelectionDrivenCommands);
	CPPUNIT_TEST(GeneratedAddAttributeSnippetParses);
	CPPUNIT_TEST(FindThenAddAttributeReachesEveryFoundNode);
	CPPUNIT_TEST(EveryCommandExampleParses);
	CPPUNIT_TEST(IncludedNodeKeepsItsMessageType);
	CPPUNIT_TEST(TypeCodesReadAsNames);
	CPPUNIT_TEST(InsertPrototypeParsesAndKeepsNodeShape);
	CPPUNIT_TEST(DroppedPrototypesGetDistinctIds);
	CPPUNIT_TEST(RepeatedInsertCreatesDistinctNodes);
	CPPUNIT_TEST(MacroSurvivesDocumentSaveAndLoad);
	CPPUNIT_TEST(PlayMacroReportsWhatHappened);
	CPPUNIT_TEST(FormatAndParseFieldValueRoundTripEveryType);
	CPPUNIT_TEST(CalculateWritesResultIntoValueContext);
	CPPUNIT_TEST(CalculateSupportsEveryOperator);
	CPPUNIT_TEST(CalculateExampleParses);
	CPPUNIT_TEST(InsertResultVariableEnablesFollowUpReference);
	CPPUNIT_TEST(CalculatedNodeIdBindsAsInt32NotFloat);
	CPPUNIT_TEST(DeleteSelectedRowsRemovesOnlyChosenSiblings);
	CPPUNIT_TEST(DeleteSelectedRowsSkipsDescendantsOfAnotherSelectedRow);
	CPPUNIT_TEST(AddNamedFieldAddsCustomFieldToGenericBlock);
	CPPUNIT_TEST(AddFieldMenuOffersRepeatableFieldAgain);
	CPPUNIT_TEST(MoveCommandRowsMovesSeveralTogetherInOrder);
	CPPUNIT_TEST(MoveCommandRowsOntoContainerAppendsAtEnd);
	CPPUNIT_TEST(AddNodeReferenceWiresChipIdIntoTargetCommand);
	CPPUNIT_TEST(AddNodeReferenceRefusesCommandWithNoNodeField);
	CPPUNIT_TEST(ReplaceNodeReferenceFieldOverwritesExistingValue);
	CPPUNIT_TEST(InterpolatesCounterIntoInsertedNodeNames);
	CPPUNIT_TEST(InterpolatedNodeStaysReachableById);
	CPPUNIT_TEST(UnknownPlaceholderIsReported);
	CPPUNIT_TEST(ConnectionFollowsInterpolatedNodes);
	CPPUNIT_TEST(PlaceholderAloneIsNotABinding);
	CPPUNIT_TEST(InterpolatesCommandSettingsInNestedBlocks);
	CPPUNIT_TEST(LayoutKeepsGraphInsideGrownCanvas);
	CPPUNIT_TEST(SelectConnectedFollowsConnections);
	CPPUNIT_TEST(GetValueSumsConnectedAttribute);
	CPPUNIT_TEST(GetValueReportsMissingAttribute);
	CPPUNIT_TEST(TextThatIsNoNumberIsReported);
	CPPUNIT_TEST(GetValueReadsEditorAttribute);
	CPPUNIT_TEST(DecimalCommaIsANumber);
	CPPUNIT_TEST(ExpandAndCollapseAllSurviveRebuild);
	CPPUNIT_TEST(MoveCommandReparentsIntoAnotherContainer);
	CPPUNIT_TEST(MoveCommandPromotesToTopLevel);
	CPPUNIT_TEST(MoveCommandRefusesDroppingIntoOwnSubtree);
	CPPUNIT_TEST(MoveCommandReordersTopLevelSiblings);
	CPPUNIT_TEST_SUITE_END();
};

#endif
