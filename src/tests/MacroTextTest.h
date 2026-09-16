#ifndef MACRO_TEXT_TEST_H
#define MACRO_TEXT_TEST_H

#include <cppunit/extensions/HelperMacros.h>

/** MacroText.cpp (#55) - the guided text DSL a recorded macro's
 * "Macro::Commmand" list round-trips through in the MacroEditor. Pure
 * functions, exercised directly here the same way GroupBoundaryTest/
 * LayoutEditorTest exercise their own pure-geometry/pure-logic targets -
 * no running MacroEditor view needed.
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
	void UnknownCommandNameIsRejected(void);
	void UnknownFieldIsRejected(void);
	void TypeMismatchIsRejected(void);

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
	CPPUNIT_TEST(UnknownCommandNameIsRejected);
	CPPUNIT_TEST(UnknownFieldIsRejected);
	CPPUNIT_TEST(TypeMismatchIsRejected);
	CPPUNIT_TEST_SUITE_END();
};

#endif
