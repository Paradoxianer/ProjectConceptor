#ifndef INDEXER_TEST_H
#define INDEXER_TEST_H

#include <cppunit/extensions/HelperMacros.h>

/** Indexer's Index/DeIndex pairs are the exact roundtrip that produced
 * most of the bugs listed in issue #68 (off-by-one, dropped P_C_NODE_PARENT
 * conversion, wrong object identity in allConnectionsList) - these tests
 * exercise that roundtrip directly, without a running app.
 */
class IndexerTest : public CppUnit::TestFixture
{
public:
	void NodeRoundtrip(void);
	void GroupedNodeRoundtrip(void);
	void ConnectionRoundtrip(void);

	CPPUNIT_TEST_SUITE(IndexerTest);
	CPPUNIT_TEST(NodeRoundtrip);
	CPPUNIT_TEST(GroupedNodeRoundtrip);
	CPPUNIT_TEST(ConnectionRoundtrip);
	CPPUNIT_TEST_SUITE_END();
};

#endif
