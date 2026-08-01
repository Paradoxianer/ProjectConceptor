#include "IndexerTest.h"

#include <app/Message.h>
#include <support/String.h>

#include "Indexer.h"
#include "PDocument.h"
#include "ProjectConceptorDefs.h"
#include "TestDocument.h"

CPPUNIT_TEST_SUITE_REGISTRATION(IndexerTest);

void IndexerTest::NodeRoundtrip(void)
{
	PDocument	*doc	= NewHeadlessTestDocument();

	BMessage	node(P_C_CLASS_TYPE);
	BMessage	data;
	data.AddString(P_C_NODE_NAME,"Test Node");
	node.AddMessage(P_C_NODE_DATA,&data);

	Indexer		saveIndexer(doc);
	BMessage	*indexed	= saveIndexer.IndexNode(&node);
	CPPUNIT_ASSERT(indexed != NULL);
	int32	id	= -1;
	CPPUNIT_ASSERT(indexed->FindInt32("this",&id) == B_OK);
	CPPUNIT_ASSERT(id > 0);

	// on-disk shape never keeps a live "this" beyond save - the loader
	// pulls it back out via RegisterDeIndexNode() before resolving anything
	Indexer		loadIndexer(doc);
	BMessage	*registered	= loadIndexer.RegisterDeIndexNode(indexed);
	CPPUNIT_ASSERT(registered->FindInt32("this",&id) != B_OK);

	BMessage	*result	= loadIndexer.DeIndexNode(registered);
	BMessage	resultData;
	CPPUNIT_ASSERT(result->FindMessage(P_C_NODE_DATA,&resultData) == B_OK);
	BString	name;
	CPPUNIT_ASSERT(resultData.FindString(P_C_NODE_NAME,&name) == B_OK);
	CPPUNIT_ASSERT(name == "Test Node");
}

void IndexerTest::GroupedNodeRoundtrip(void)
{
	// regression test for the bug in issue #68: IndexNode() never
	// converted P_C_NODE_PARENT, so DeIndexNode() (which does expect an
	// id there) silently found nothing and every grouped node lost its
	// parent on reload
	PDocument	*doc	= NewHeadlessTestDocument();

	BMessage	group(P_C_GROUP_TYPE);
	BMessage	child(P_C_CLASS_TYPE);
	child.AddPointer(P_C_NODE_PARENT,&group);

	Indexer		saveIndexer(doc);
	BMessage	*indexedGroup	= saveIndexer.IndexNode(&group);
	BMessage	*indexedChild	= saveIndexer.IndexNode(&child);
	int32	parentId	= -1;
	CPPUNIT_ASSERT(indexedChild->FindInt32(P_C_NODE_PARENT,&parentId) == B_OK);
	int32	groupId	= -1;
	CPPUNIT_ASSERT(indexedGroup->FindInt32("this",&groupId) == B_OK);
	CPPUNIT_ASSERT_EQUAL(groupId,parentId);

	Indexer		loadIndexer(doc);
	loadIndexer.RegisterDeIndexNode(indexedGroup);
	loadIndexer.RegisterDeIndexNode(indexedChild);
	BMessage	*resultChild	= loadIndexer.DeIndexNode(indexedChild);

	void	*resolvedParent	= NULL;
	CPPUNIT_ASSERT(resultChild->FindPointer(P_C_NODE_PARENT,&resolvedParent) == B_OK);
	CPPUNIT_ASSERT_EQUAL((void*)indexedGroup,resolvedParent);
}

void IndexerTest::ConnectionRoundtrip(void)
{
	// regression test for the bug in issue #68: allConnectionsList used to
	// store the address of a transient copy instead of the original
	// connection's identity - this checks the resolved pointers after
	// reload are the exact same objects that were registered, not copies
	PDocument	*doc	= NewHeadlessTestDocument();

	BMessage	from(P_C_CLASS_TYPE);
	BMessage	to(P_C_CLASS_TYPE);
	BMessage	connection(P_C_CONNECTION_TYPE);
	connection.AddPointer(P_C_NODE_CONNECTION_FROM,&from);
	connection.AddPointer(P_C_NODE_CONNECTION_TO,&to);

	Indexer		saveIndexer(doc);
	BMessage	*indexedFrom		= saveIndexer.IndexNode(&from);
	BMessage	*indexedTo			= saveIndexer.IndexNode(&to);
	BMessage	*indexedConnection	= saveIndexer.IndexConnection(&connection,false);
	int32	fromId	= -1;
	int32	toId	= -1;
	CPPUNIT_ASSERT(indexedConnection->FindInt32(P_C_NODE_CONNECTION_FROM,&fromId) == B_OK);
	CPPUNIT_ASSERT(indexedConnection->FindInt32(P_C_NODE_CONNECTION_TO,&toId) == B_OK);
	CPPUNIT_ASSERT(fromId != toId);

	// mirrors PDocLoader's real sequence: every node gets registered first,
	// connections are resolved against the now-complete sorter afterward
	Indexer		loadIndexer(doc);
	loadIndexer.RegisterDeIndexNode(indexedFrom);
	loadIndexer.RegisterDeIndexNode(indexedTo);
	BMessage	*result	= loadIndexer.DeIndexConnection(indexedConnection);

	void	*resolvedFrom	= NULL;
	void	*resolvedTo		= NULL;
	CPPUNIT_ASSERT(result->FindPointer(P_C_NODE_CONNECTION_FROM,&resolvedFrom) == B_OK);
	CPPUNIT_ASSERT(result->FindPointer(P_C_NODE_CONNECTION_TO,&resolvedTo) == B_OK);
	CPPUNIT_ASSERT_EQUAL((void*)indexedFrom,resolvedFrom);
	CPPUNIT_ASSERT_EQUAL((void*)indexedTo,resolvedTo);
}

void IndexerTest::ManyNodesDoNotLeakEditorInstances(void)
{
	// regression test for issue #71: IndexNode()/IndexConnection() used to
	// construct a fresh, never-released PEditor per editor plugin on every
	// single call. With real editor plugins loaded (this test binary's
	// Plugins/ symlinks to the actual built plugins, see docs/notes.md)
	// that exhausted the process's file descriptors around ~250 nodes -
	// this document is well past that. Indexer::GetCachedEditors() now
	// builds each editor once per Indexer instance and reuses it; if that
	// regresses, this test either fails outright or the process crashes/
	// hangs building the 250th-ish editor instance, same as the original
	// bug did for the stress-fixture generator.
	PDocument	*doc	= NewHeadlessTestDocument();
	Indexer		indexer(doc);

	const int32	count	= 300;
	BMessage	*previous	= NULL;
	for (int32 i = 0; i < count; i++) {
		BMessage	*node	= new BMessage(P_C_CLASS_TYPE);
		BMessage	*indexed	= indexer.IndexNode(node);
		CPPUNIT_ASSERT(indexed != NULL);
		if (previous != NULL) {
			BMessage	*connection	= new BMessage(P_C_CONNECTION_TYPE);
			connection->AddPointer(P_C_NODE_CONNECTION_FROM,previous);
			connection->AddPointer(P_C_NODE_CONNECTION_TO,node);
			CPPUNIT_ASSERT(indexer.IndexConnection(connection,false) != NULL);
		}
		previous = node;
	}
}
