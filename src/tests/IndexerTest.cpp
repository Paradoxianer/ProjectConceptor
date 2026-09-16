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

void IndexerTest::MacroCommandIncludedNodeRoundtrip(void)
{
	// regression test: DeIndexCommand() used to call DeIndexNode() on each
	// "included_node" without RegisterDeIndexNode() first, so the node's
	// own id never entered `sorter`. The command's own "node" id (added by
	// IndexCommand()) could then never resolve back to a pointer, and
	// PlayMacro() silently executed a command with no "node" field at all
	// - the recorded macro's Insert did nothing, with no error surfaced.
	// See PCommandManager::PlayMacro()/Indexer::DeIndexCommand().
	PDocument	*doc	= NewHeadlessTestDocument();

	BMessage	*node	= new BMessage(P_C_CLASS_TYPE);
	BMessage	data;
	data.AddString(P_C_NODE_NAME,"Test Node");
	node->AddMessage(P_C_NODE_DATA,&data);

	BMessage	command;
	command.AddString("Command::Name","Insert");
	command.AddPointer("node",node);

	Indexer		saveIndexer(doc);
	BMessage	*indexedCommand	= saveIndexer.IndexCommand(&command,true);
	int32	nodeId	= -1;
	CPPUNIT_ASSERT(indexedCommand->FindInt32("node",&nodeId) == B_OK);
	BMessage	includedNode;
	CPPUNIT_ASSERT(indexedCommand->FindMessage("included_node",&includedNode) == B_OK);

	// mirrors PCommandManager::PlayMacro(): one fresh Indexer, DeIndexCommand()
	// called directly on the stored/indexed command
	Indexer		playIndexer(doc);
	BMessage	*result	= playIndexer.DeIndexCommand(indexedCommand);

	void	*resolvedNode	= NULL;
	CPPUNIT_ASSERT(result->FindPointer("node",&resolvedNode) == B_OK);
	CPPUNIT_ASSERT(resolvedNode != NULL);
	BMessage	resultData;
	CPPUNIT_ASSERT(((BMessage*)resolvedNode)->FindMessage(P_C_NODE_DATA,&resultData) == B_OK);
	BString	name;
	CPPUNIT_ASSERT(resultData.FindString(P_C_NODE_NAME,&name) == B_OK);
	CPPUNIT_ASSERT(name == "Test Node");
}

void IndexerTest::MacroCommandIncludedConnectionRoundtrip(void)
{
	// regression test: a command's "node" field holds both plain nodes
	// and connections (e.g. Insert::Do() inserting a drawn connection
	// together with its two new endpoint nodes) - IndexCommand() used to
	// route every "node" pointer through IndexNode() regardless of type,
	// which has no idea how to convert a connection's own
	// P_C_NODE_CONNECTION_FROM/TO pointer fields, leaving them as live,
	// unconvertable pointers all the way into the stored macro text. Two
	// further bugs in the same family sat behind it: IndexConnection()'s
	// "endpoint already indexed elsewhere" branch did nothing at all
	// (same live-pointer leak, just for nodes listed before their
	// connection instead of after), and DeIndexConnection()'s embedded-
	// node case resolved a node without registering its own id into
	// `sorter` first. All three combined were confirmed as a live crash
	// on macro replay: ConnectionRenderer::Init() dereferencing a never-
	// resolved P_C_NODE_CONNECTION_FROM/TO.
	PDocument	*doc	= NewHeadlessTestDocument();

	BMessage	*from	= new BMessage(P_C_CLASS_TYPE);
	BMessage	*to		= new BMessage(P_C_CLASS_TYPE);
	BMessage	fromData;
	fromData.AddString(P_C_NODE_NAME,"From Node");
	from->AddMessage(P_C_NODE_DATA,&fromData);
	BMessage	toData;
	toData.AddString(P_C_NODE_NAME,"To Node");
	to->AddMessage(P_C_NODE_DATA,&toData);

	BMessage	*connection	= new BMessage(P_C_CONNECTION_TYPE);
	connection->AddPointer(P_C_NODE_CONNECTION_FROM,from);
	connection->AddPointer(P_C_NODE_CONNECTION_TO,to);

	// nodes listed before the connection - the common real-world order
	// (draw the nodes, then connect them) - exercises IndexConnection()'s
	// "already included" branch, not just its "not yet included" one.
	BMessage	command;
	command.AddString("Command::Name","Insert");
	command.AddPointer("node",from);
	command.AddPointer("node",to);
	command.AddPointer("node",connection);

	Indexer		saveIndexer(doc);
	BMessage	*indexedCommand	= saveIndexer.IndexCommand(&command,true);

	Indexer		playIndexer(doc);
	BMessage	*result	= playIndexer.DeIndexCommand(indexedCommand);

	BMessage	*candidate			= NULL;
	BMessage	*resolvedConnection	= NULL;
	int32		connectionCount		= 0;
	for (int32 i = 0; result->FindPointer("node",i,(void**)&candidate) == B_OK; i++) {
		if (candidate->what == P_C_CONNECTION_TYPE) {
			connectionCount++;
			resolvedConnection	= candidate;
		}
	}
	CPPUNIT_ASSERT_EQUAL((int32)1,connectionCount);
	CPPUNIT_ASSERT(resolvedConnection != NULL);

	void	*resolvedFrom	= NULL;
	void	*resolvedTo		= NULL;
	CPPUNIT_ASSERT(resolvedConnection->FindPointer(P_C_NODE_CONNECTION_FROM,&resolvedFrom) == B_OK);
	CPPUNIT_ASSERT(resolvedConnection->FindPointer(P_C_NODE_CONNECTION_TO,&resolvedTo) == B_OK);
	CPPUNIT_ASSERT(resolvedFrom != NULL);
	CPPUNIT_ASSERT(resolvedTo != NULL);

	BString	fromName;
	BMessage	resolvedFromData;
	((BMessage*)resolvedFrom)->FindMessage(P_C_NODE_DATA,&resolvedFromData);
	resolvedFromData.FindString(P_C_NODE_NAME,&fromName);
	CPPUNIT_ASSERT(fromName == "From Node");
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
