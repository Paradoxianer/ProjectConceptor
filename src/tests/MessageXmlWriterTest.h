#ifndef MESSAGE_XML_WRITER_TEST_H
#define MESSAGE_XML_WRITER_TEST_H

#include <cppunit/extensions/HelperMacros.h>

/** Isolates the SIGFPE from issue #114 - debug_server caught it inside
 * MessageXmlWriter::ProcessMessage(), 5/5 reproduced at the exact same
 * offset (a deterministic arithmetic fault, not corruption-dependent
 * scheduling), but the exact trigger was never confirmed. Each test here
 * tries one specific candidate field in isolation through the public
 * WriteTo() API (an in-memory BMallocIO destination, no filesystem
 * needed) - whichever one actually faults confirms the real cause.
 */
class MessageXmlWriterTest : public CppUnit::TestFixture
{
public:
	void ZeroLengthRawFieldDoesNotCrash(void);
	void NaNFloatFieldDoesNotCrash(void);
	void InfinityDoubleFieldDoesNotCrash(void);
	void OrdinaryMessageRoundtrips(void);
	void RawFieldSizesZeroToThreeHundredDoNotCrash(void);
	void RealisticNestedNodeMessageDoesNotCrash(void);

	CPPUNIT_TEST_SUITE(MessageXmlWriterTest);
	CPPUNIT_TEST(ZeroLengthRawFieldDoesNotCrash);
	CPPUNIT_TEST(NaNFloatFieldDoesNotCrash);
	CPPUNIT_TEST(InfinityDoubleFieldDoesNotCrash);
	CPPUNIT_TEST(OrdinaryMessageRoundtrips);
	CPPUNIT_TEST(RawFieldSizesZeroToThreeHundredDoNotCrash);
	CPPUNIT_TEST(RealisticNestedNodeMessageDoesNotCrash);
	CPPUNIT_TEST_SUITE_END();
};

#endif
