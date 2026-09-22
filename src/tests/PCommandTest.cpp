#include "PCommandTest.h"

#include <app/Message.h>
#include <app/Messenger.h>
#include <interface/Rect.h>
#include <support/List.h>
#include <OS.h>

#include "BasePlugin.h"
#include "ChangeValue.h"
#include "Find.h"
#include "ForEach.h"
#include "Group.h"
#include "If.h"
#include "Insert.h"
#include "Move.h"
#include "Remember.h"
#include "Repeat.h"
#include "Select.h"
#include "Sleep.h"
#include "PCommandManager.h"
#include "PDocument.h"
#include "ProjectConceptorDefs.h"
#include "TestDocument.h"

CPPUNIT_TEST_SUITE_REGISTRATION(PCommandTest);

namespace {

// Minimal BasePlugin to register ChangeValue without a real plugin .so.
class TestChangeValuePlugin : public BasePlugin {
public:
	TestChangeValuePlugin(void) : BasePlugin(0) {}
	virtual char*	GetName(void) { return (char *)"ChangeValue"; }
	virtual char*	GetAutor(void) { return (char *)"test"; }
	virtual char*	GetVersionsString(void) { return (char *)"0"; }
	virtual char*	GetDescription(void) { return (char *)"test"; }
	virtual uint32	GetType(void) { return P_C_COMMANDO_PLUGIN_TYPE; }
	virtual void*	GetNewObject(void *value) { return new ChangeValue(); }
};

// Do()/Undo() unoverridden - exercises only the subPCommand loop.
class TestWrapperCommand : public PCommand {
public:
	virtual void	AttachedToManager(void) {}
	virtual void	DetachedFromManager(void) {}
	virtual char*	Name(void) { return (char *)"TestWrapper"; }
};

// Same minimal-BasePlugin technique as TestChangeValuePlugin above, for
// #135's own new commands (and Move/Select, needed as PlayMacro()-driven
// subPCommand children below - direct Do() calls elsewhere in this file
// never needed them registered, since they bypass GetPCommand() lookup
// entirely).
#define TEST_PLUGIN(ClassName, CommandClass, CommandName) \
	class ClassName : public BasePlugin { \
	public: \
		ClassName(void) : BasePlugin(0) {} \
		virtual char*	GetName(void) { return (char *)CommandName; } \
		virtual char*	GetAutor(void) { return (char *)"test"; } \
		virtual char*	GetVersionsString(void) { return (char *)"0"; } \
		virtual char*	GetDescription(void) { return (char *)"test"; } \
		virtual uint32	GetType(void) { return P_C_COMMANDO_PLUGIN_TYPE; } \
		virtual void*	GetNewObject(void *value) { return new CommandClass(); } \
	};

TEST_PLUGIN(TestMovePlugin,Move,"Move")
TEST_PLUGIN(TestSelectPlugin,Select,"Select")
TEST_PLUGIN(TestFindPlugin,Find,"Find")
TEST_PLUGIN(TestRepeatPlugin,Repeat,"Repeat")
TEST_PLUGIN(TestForEachPlugin,ForEach,"ForEach")
TEST_PLUGIN(TestIfPlugin,If,"If")
TEST_PLUGIN(TestRememberPlugin,Remember,"Remember")
TEST_PLUGIN(TestSleepPlugin,Sleep,"Sleep")

#undef TEST_PLUGIN

}

void PCommandTest::ChangeValueDoUndo(void)
{
	PDocument	*doc	= NewHeadlessTestDocument();

	BMessage	node;
	node.AddInt32("TestValue",1);

	BMessage	valueContainer;
	valueContainer.AddString("name","TestValue");
	valueContainer.AddInt32("type",(int32)B_INT32_TYPE);
	valueContainer.AddInt32("index",0);
	int32	newValue	= 99;
	valueContainer.AddData("newValue",B_INT32_TYPE,&newValue,sizeof(int32));

	BMessage	settings;
	settings.AddPointer("node",&node);
	settings.AddMessage("valueContainer",&valueContainer);

	ChangeValue	command;
	BMessage	*result	= command.Do(doc,&settings);
	CPPUNIT_ASSERT(result != NULL);

	int32	changed	= 0;
	CPPUNIT_ASSERT(node.FindInt32("TestValue",&changed) == B_OK);
	CPPUNIT_ASSERT_EQUAL((int32)99,changed);

	command.Undo(doc,result);

	int32	restored	= 0;
	CPPUNIT_ASSERT(node.FindInt32("TestValue",&restored) == B_OK);
	CPPUNIT_ASSERT_EQUAL((int32)1,restored);
}

void PCommandTest::ChangeValueOnSelectionDoUndo(void)
{
	// regression check for issue #61: ChangeValue applied to "all selected
	// nodes" (P_C_NODE_SELECTED - used by e.g. the Pen size control), which
	// the report claims gets called twice with the second Undo ending up
	// with the wrong node. Exercises Do()+Undo() across two selected nodes
	// at once through the real ChangeValue class and checks that each
	// node's own value is independently changed and restored.
	PDocument	*doc	= NewHeadlessTestDocument();

	BMessage	pattern1;
	pattern1.AddFloat("PenSize",1.0f);
	BMessage	node1(P_C_CLASS_TYPE);
	node1.AddMessage(P_C_NODE_PATTERN,&pattern1);

	BMessage	pattern2;
	pattern2.AddFloat("PenSize",2.0f);
	BMessage	node2(P_C_CLASS_TYPE);
	node2.AddMessage(P_C_NODE_PATTERN,&pattern2);

	doc->GetSelected()->AddItem(&node1);
	doc->GetSelected()->AddItem(&node2);

	BMessage	valueContainer;
	valueContainer.AddString("name","PenSize");
	valueContainer.AddString("subgroup",P_C_NODE_PATTERN);
	valueContainer.AddInt32("type",(int32)B_FLOAT_TYPE);
	valueContainer.AddFloat("newValue",5.0f);

	BMessage	settings;
	settings.AddBool(P_C_NODE_SELECTED,true);
	settings.AddMessage("valueContainer",&valueContainer);

	ChangeValue	command;
	BMessage	*result	= command.Do(doc,&settings);
	CPPUNIT_ASSERT(result != NULL);

	BMessage	changedPattern1;
	CPPUNIT_ASSERT(node1.FindMessage(P_C_NODE_PATTERN,&changedPattern1) == B_OK);
	float	penSize1	= 0;
	CPPUNIT_ASSERT(changedPattern1.FindFloat("PenSize",&penSize1) == B_OK);
	CPPUNIT_ASSERT_EQUAL(5.0f,penSize1);

	BMessage	changedPattern2;
	CPPUNIT_ASSERT(node2.FindMessage(P_C_NODE_PATTERN,&changedPattern2) == B_OK);
	float	penSize2	= 0;
	CPPUNIT_ASSERT(changedPattern2.FindFloat("PenSize",&penSize2) == B_OK);
	CPPUNIT_ASSERT_EQUAL(5.0f,penSize2);

	command.Undo(doc,result);

	BMessage	restoredPattern1;
	CPPUNIT_ASSERT(node1.FindMessage(P_C_NODE_PATTERN,&restoredPattern1) == B_OK);
	float	restored1	= 0;
	CPPUNIT_ASSERT(restoredPattern1.FindFloat("PenSize",&restored1) == B_OK);
	CPPUNIT_ASSERT_EQUAL(1.0f,restored1);

	BMessage	restoredPattern2;
	CPPUNIT_ASSERT(node2.FindMessage(P_C_NODE_PATTERN,&restoredPattern2) == B_OK);
	float	restored2	= 0;
	CPPUNIT_ASSERT(restoredPattern2.FindFloat("PenSize",&restored2) == B_OK);
	CPPUNIT_ASSERT_EQUAL(2.0f,restored2);
}

void PCommandTest::ChangeValueOnConnectionPattern(void)
{
	// regression test: connections used to never get a P_C_NODE_PATTERN
	// sub-message at all, so the Pen size/Fill color toolbar controls
	// (which go through ChangeValue targeting "PenSize"/"FillColor" inside
	// that sub-message, same as for a class node) silently did nothing for
	// a selected connection - DoChangeValue()'s FindData/ReplaceData both
	// failed quietly against a message that was never there.
	PDocument	*doc	= NewHeadlessTestDocument();

	BMessage	pattern;
	pattern.AddFloat("PenSize",2.0f);
	pattern.AddInt32("FillColor",0xff43439a);
	BMessage	connection(P_C_CONNECTION_TYPE);
	connection.AddMessage(P_C_NODE_PATTERN,&pattern);

	doc->GetSelected()->AddItem(&connection);

	BMessage	valueContainer;
	valueContainer.AddString("name","PenSize");
	valueContainer.AddString("subgroup",P_C_NODE_PATTERN);
	valueContainer.AddInt32("type",(int32)B_FLOAT_TYPE);
	valueContainer.AddFloat("newValue",5.0f);

	BMessage	settings;
	settings.AddBool(P_C_NODE_SELECTED,true);
	settings.AddMessage("valueContainer",&valueContainer);

	ChangeValue	command;
	BMessage	*result	= command.Do(doc,&settings);
	CPPUNIT_ASSERT(result != NULL);

	BMessage	changedPattern;
	CPPUNIT_ASSERT(connection.FindMessage(P_C_NODE_PATTERN,&changedPattern) == B_OK);
	float	changedPenSize	= 0;
	CPPUNIT_ASSERT(changedPattern.FindFloat("PenSize",&changedPenSize) == B_OK);
	CPPUNIT_ASSERT_EQUAL(5.0f,changedPenSize);
}

void PCommandTest::GroupThenInsertChildRegistersInParentList(void)
{
	// regression test for issue #36: double-clicking a group node inserts a
	// new child the same way GroupRenderer::MouseDown does - build an Insert
	// command and set P_C_NODE_PARENT on the *node being inserted*, not on
	// the command's wrapper message. Insert::Do() used to read
	// P_C_NODE_PARENT off the wrapper instead, which nothing ever set, so
	// the new node silently never made it into the group's own node list
	// and ended up a top-level sibling instead of a child.
	PDocument	*doc	= NewHeadlessTestDocument();

	BMessage	child1(P_C_CLASS_TYPE);
	child1.AddRect(P_C_NODE_FRAME,BRect(0,0,50,50));
	BMessage	child2(P_C_CLASS_TYPE);
	child2.AddRect(P_C_NODE_FRAME,BRect(100,0,150,50));
	doc->GetAllNodes()->AddItem(&child1);
	doc->GetAllNodes()->AddItem(&child2);
	doc->GetSelected()->AddItem(&child1);
	doc->GetSelected()->AddItem(&child2);

	BMessage	groupNode(P_C_GROUP_TYPE);
	BMessage	groupSettings;
	groupSettings.AddPointer("node",&groupNode);

	Group	groupCommand;
	groupCommand.Do(doc,&groupSettings);

	BList	*groupAllNodes	= NULL;
	CPPUNIT_ASSERT(groupNode.FindPointer(P_C_NODE_ALLNODES,(void **)&groupAllNodes) == B_OK);
	CPPUNIT_ASSERT(groupAllNodes->HasItem(&child1));

	BMessage	newChild(P_C_CLASS_TYPE);
	newChild.AddPointer(P_C_NODE_PARENT,&groupNode);

	BMessage	insertSettings;
	insertSettings.AddPointer("node",&newChild);

	Insert	insertCommand;
	insertCommand.Do(doc,&insertSettings);

	CPPUNIT_ASSERT(doc->GetAllNodes()->HasItem(&newChild));
	CPPUNIT_ASSERT(groupAllNodes->HasItem(&newChild));
}

void PCommandTest::GroupUndoThenRedoKeepsChildren(void)
{
	// regression test for a bug found while live-testing #38's fixes:
	// Group::Undo() removed a child from the group's own P_C_NODE_ALLNODES
	// list but never cleared P_C_NODE_PARENT on the child itself. Redo runs
	// Group::Do() again on the same two nodes - which only (re-)groups a
	// node whose P_C_NODE_PARENT isn't already set (see its guard) - so the
	// stale leftover parent pointer made every child look "already grouped"
	// and Do() silently skipped re-adding any of them.
	PDocument	*doc	= NewHeadlessTestDocument();

	BMessage	child1(P_C_CLASS_TYPE);
	child1.AddRect(P_C_NODE_FRAME,BRect(0,0,50,50));
	BMessage	child2(P_C_CLASS_TYPE);
	child2.AddRect(P_C_NODE_FRAME,BRect(100,0,150,50));
	doc->GetAllNodes()->AddItem(&child1);
	doc->GetAllNodes()->AddItem(&child2);
	doc->GetSelected()->AddItem(&child1);
	doc->GetSelected()->AddItem(&child2);

	BMessage	groupNode(P_C_GROUP_TYPE);
	BMessage	groupSettings;
	groupSettings.AddPointer("node",&groupNode);

	Group		groupCommand;
	BMessage	*result	= groupCommand.Do(doc,&groupSettings);
	CPPUNIT_ASSERT(result != NULL);

	groupCommand.Undo(doc,result);

	void	*parent	= NULL;
	CPPUNIT_ASSERT(child1.FindPointer(P_C_NODE_PARENT,&parent) != B_OK);
	CPPUNIT_ASSERT(child2.FindPointer(P_C_NODE_PARENT,&parent) != B_OK);

	// redo: same settings message, same current selection - matches what
	// PCommandManager::Redo() actually replays
	groupCommand.Do(doc,&groupSettings);

	BList	*groupAllNodes	= NULL;
	CPPUNIT_ASSERT(groupNode.FindPointer(P_C_NODE_ALLNODES,(void **)&groupAllNodes) == B_OK);
	CPPUNIT_ASSERT(groupAllNodes->HasItem(&child1));
	CPPUNIT_ASSERT(groupAllNodes->HasItem(&child2));
}

void PCommandTest::WrapperUndoRestoresAllSubcommands(void)
{
	// #116: subPCommand write-back used to always hit slot 0, so only
	// the last of several subcommands got undo info. Two ChangeValues
	// here - without the fix, node1 (slot 0) never undoes.
	PDocument	*doc	= NewHeadlessTestDocument();
	doc->GetCommandManager()->RegisterPCommand(new TestChangeValuePlugin());

	BMessage	node1(P_C_CLASS_TYPE);
	node1.AddInt32("TestValue",1);
	BMessage	node2(P_C_CLASS_TYPE);
	node2.AddInt32("TestValue",2);

	BMessage	valueContainer1;
	valueContainer1.AddString("name","TestValue");
	valueContainer1.AddInt32("type",(int32)B_INT32_TYPE);
	valueContainer1.AddInt32("index",0);
	int32	newValue1	= 100;
	valueContainer1.AddData("newValue",B_INT32_TYPE,&newValue1,sizeof(int32));

	BMessage	sub1;
	sub1.AddString("Command::Name","ChangeValue");
	sub1.AddPointer("node",&node1);
	sub1.AddMessage("valueContainer",&valueContainer1);

	BMessage	valueContainer2;
	valueContainer2.AddString("name","TestValue");
	valueContainer2.AddInt32("type",(int32)B_INT32_TYPE);
	valueContainer2.AddInt32("index",0);
	int32	newValue2	= 200;
	valueContainer2.AddData("newValue",B_INT32_TYPE,&newValue2,sizeof(int32));

	BMessage	sub2;
	sub2.AddString("Command::Name","ChangeValue");
	sub2.AddPointer("node",&node2);
	sub2.AddMessage("valueContainer",&valueContainer2);

	BMessage	settings;
	settings.AddMessage("PCommand::subPCommand",&sub1);
	settings.AddMessage("PCommand::subPCommand",&sub2);

	TestWrapperCommand	wrapper;
	wrapper.SetManager(doc->GetCommandManager());
	BMessage	*result	= wrapper.Do(doc,&settings);
	CPPUNIT_ASSERT(result != NULL);

	int32	changed1	= 0;
	CPPUNIT_ASSERT(node1.FindInt32("TestValue",&changed1) == B_OK);
	CPPUNIT_ASSERT_EQUAL((int32)100,changed1);
	int32	changed2	= 0;
	CPPUNIT_ASSERT(node2.FindInt32("TestValue",&changed2) == B_OK);
	CPPUNIT_ASSERT_EQUAL((int32)200,changed2);

	wrapper.Undo(doc,result);

	int32	restored1	= 0;
	CPPUNIT_ASSERT(node1.FindInt32("TestValue",&restored1) == B_OK);
	CPPUNIT_ASSERT_EQUAL((int32)1,restored1);
	int32	restored2	= 0;
	CPPUNIT_ASSERT(node2.FindInt32("TestValue",&restored2) == B_OK);
	CPPUNIT_ASSERT_EQUAL((int32)2,restored2);
}


void PCommandTest::MoveGroupWithSelectedChildrenMovesOnce(void)
{
	// Move::MoveNode() carries a group's children along by recursing through
	// P_C_NODE_ALLNODES. Select all puts the group *and* its children in the
	// selection, so moving every selected node from the top offset each
	// child twice - and the group's box, refitted to its children, ended up
	// at twice the drag distance.
	PDocument	*doc	= NewHeadlessTestDocument();

	BMessage	child1(P_C_CLASS_TYPE);
	child1.AddRect(P_C_NODE_FRAME,BRect(0,0,50,50));
	BMessage	child2(P_C_CLASS_TYPE);
	child2.AddRect(P_C_NODE_FRAME,BRect(100,0,150,50));
	doc->GetAllNodes()->AddItem(&child1);
	doc->GetAllNodes()->AddItem(&child2);
	doc->GetSelected()->AddItem(&child1);
	doc->GetSelected()->AddItem(&child2);

	BMessage	groupNode(P_C_GROUP_TYPE);
	BMessage	groupSettings;
	groupSettings.AddPointer("node",&groupNode);
	Group	groupCommand;
	groupCommand.Do(doc,&groupSettings);

	// go through the real Select all, so this covers the selection state it
	// actually produces rather than a hand-built approximation
	doc->GetSelected()->MakeEmpty();
	doc->GetAllNodes()->AddItem(&groupNode);

	BMessage	selectSettings;
	selectSettings.AddBool("selectAll",true);
	Select	selectCommand;
	selectCommand.Do(doc,&selectSettings);

	CPPUNIT_ASSERT(doc->GetSelected()->HasItem(&groupNode));
	CPPUNIT_ASSERT(doc->GetSelected()->HasItem(&child1));
	bool	flag	= false;
	CPPUNIT_ASSERT(child1.FindBool(P_C_NODE_SELECTED,&flag) == B_OK);
	CPPUNIT_ASSERT(flag);

	BMessage	moveSettings;
	moveSettings.AddFloat("dx",10.0);
	moveSettings.AddFloat("dy",5.0);

	Move	moveCommand;
	moveCommand.Do(doc,&moveSettings);

	BRect	moved;
	CPPUNIT_ASSERT(child1.FindRect(P_C_NODE_FRAME,&moved) == B_OK);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0,moved.left,0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0,moved.top,0.001);
	CPPUNIT_ASSERT(child2.FindRect(P_C_NODE_FRAME,&moved) == B_OK);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(110.0,moved.left,0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0,moved.top,0.001);
}

void PCommandTest::ExecuteViaRealMessageDispatchSurvivesProcessExit(void)
{
	// #117: every other test in this suite calls a PCommand's Do()/Undo()
	// directly - this is the one path that goes through a real BMessenger
	// send to the document's own looper thread, same as every editor in
	// the app does it (see e.g. GraphEditor's sentTo->SendMessage() calls).
	// A headless PDocument (NewHeadlessTestDocument()) is "fine to just
	// leak" per its own doc comment, but its BLooper::Run() thread is real
	// and was never told to stop - if it is still mid-dispatch of a
	// message queued here when the test process itself exits, it crashes
	// into memory that is already being torn down. The crash used to show
	// up as a *separate* debug_server report a few seconds after CppUnit
	// had already printed "OK" and this process returned - snooze()ing
	// here to let this specific send finish is not the fix (later tests'
	// leaked documents, and this one after the snooze, are still running
	// loopers when main() returns) - the actual fix is in TestMain.cpp.
	PDocument	*doc	= NewHeadlessTestDocument();
	doc->GetCommandManager()->RegisterPCommand(new TestChangeValuePlugin());

	BMessage	*node	= new BMessage(P_C_CLASS_TYPE);
	node->AddInt32("TestValue",1);
	doc->GetAllNodes()->AddItem(node);

	BMessage	*valueContainer	= new BMessage();
	valueContainer->AddString("name","TestValue");
	valueContainer->AddInt32("type",(int32)B_INT32_TYPE);
	valueContainer->AddInt32("index",0);
	int32	newValue	= 42;
	valueContainer->AddData("newValue",B_INT32_TYPE,&newValue,sizeof(int32));

	BMessage	settings(P_C_EXECUTE_COMMAND);
	settings.AddString("Command::Name","ChangeValue");
	settings.AddPointer("node",node);
	settings.AddMessage("valueContainer",valueContainer);

	BMessenger	target(doc);
	CPPUNIT_ASSERT(target.IsValid());
	CPPUNIT_ASSERT(target.SendMessage(&settings) == B_OK);

	// give the looper thread a chance to actually dispatch it before this
	// test method (and eventually the whole process) moves on
	snooze(200000);

	int32	changed	= 0;
	CPPUNIT_ASSERT(node->FindInt32("TestValue",&changed) == B_OK);
	CPPUNIT_ASSERT_EQUAL((int32)42,changed);
}


void PCommandTest::DirectManipulationOnSelectionNormalizedForRecording(void)
{
	// #132: ChangeValue sent with an explicit "node" pointer (the shape
	// ClassRenderer's inline rename/attribute editing and NavigatorEditor's
	// field editor actually send - see their own "node" pointer comments)
	// gets recorded the SAME portable way as the toolbar's own
	// Node::selected=true form, when that pointer is exactly what was
	// selected right before the command ran - PCommandManager::Execute()'s
	// NormalizeToSelection() is what does this, right before the macro
	// recording step.
	PDocument	*doc	= NewHeadlessTestDocument();
	doc->GetCommandManager()->RegisterPCommand(new TestChangeValuePlugin());

	BMessage	*node	= new BMessage(P_C_CLASS_TYPE);
	node->AddInt32("TestValue",1);
	doc->GetAllNodes()->AddItem(node);
	doc->GetSelected()->AddItem(node);

	BMessage	*valueContainer	= new BMessage();
	valueContainer->AddString("name","TestValue");
	valueContainer->AddInt32("type",(int32)B_INT32_TYPE);
	valueContainer->AddInt32("index",0);
	int32	newValue	= 42;
	valueContainer->AddData("newValue",B_INT32_TYPE,&newValue,sizeof(int32));

	BMessage	settings(P_C_EXECUTE_COMMAND);
	settings.AddString("Command::Name","ChangeValue");
	settings.AddPointer("node",node);
	settings.AddMessage("valueContainer",valueContainer);

	doc->GetCommandManager()->StartMacro();
	CPPUNIT_ASSERT(doc->GetCommandManager()->Execute(&settings) == B_OK);

	BMessage	*recording	= doc->GetCommandManager()->GetRecording();
	CPPUNIT_ASSERT(recording != NULL);
	BMessage	recorded;
	int32		lastIndex	= 0;
	while (recording->FindMessage("Macro::Commmand",lastIndex,&recorded) == B_OK)
		lastIndex++;
	CPPUNIT_ASSERT(lastIndex > 0);
	CPPUNIT_ASSERT(recording->FindMessage("Macro::Commmand",lastIndex-1,&recorded) == B_OK);

	bool	selectedFlag	= false;
	CPPUNIT_ASSERT(recorded.FindBool(P_C_NODE_SELECTED,&selectedFlag) == B_OK);
	CPPUNIT_ASSERT(selectedFlag);
	void	*ignoredPointer	= NULL;
	CPPUNIT_ASSERT(recorded.FindPointer("node",&ignoredPointer) != B_OK);
	int32	ignoredId	= 0;
	CPPUNIT_ASSERT(recorded.FindInt32("node",&ignoredId) != B_OK);
}


void PCommandTest::DirectManipulationOffSelectionKeepsExplicitNodeForRecording(void)
{
	// the other half of #132's contract: an explicit "node" pointer that
	// does NOT match the current selection is a genuinely document-local
	// target (e.g. ClassRenderer::AdjustParents()'s own ChangeValue
	// subcommand, computed per-node and never equal to the top-level
	// selection) - NormalizeToSelection() must leave it alone, or replay
	// would silently apply the edit to whatever happens to be selected
	// instead of the node it actually targeted.
	PDocument	*doc	= NewHeadlessTestDocument();
	doc->GetCommandManager()->RegisterPCommand(new TestChangeValuePlugin());

	BMessage	*selectedNode	= new BMessage(P_C_CLASS_TYPE);
	selectedNode->AddInt32("TestValue",0);
	doc->GetAllNodes()->AddItem(selectedNode);
	doc->GetSelected()->AddItem(selectedNode);

	BMessage	*targetNode	= new BMessage(P_C_CLASS_TYPE);
	targetNode->AddInt32("TestValue",1);
	doc->GetAllNodes()->AddItem(targetNode);
	// deliberately NOT added to doc->GetSelected() - settings below targets
	// it directly instead, same as a computed per-node edit would.

	BMessage	*valueContainer	= new BMessage();
	valueContainer->AddString("name","TestValue");
	valueContainer->AddInt32("type",(int32)B_INT32_TYPE);
	valueContainer->AddInt32("index",0);
	int32	newValue	= 42;
	valueContainer->AddData("newValue",B_INT32_TYPE,&newValue,sizeof(int32));

	BMessage	settings(P_C_EXECUTE_COMMAND);
	settings.AddString("Command::Name","ChangeValue");
	settings.AddPointer("node",targetNode);
	settings.AddMessage("valueContainer",valueContainer);

	doc->GetCommandManager()->StartMacro();
	CPPUNIT_ASSERT(doc->GetCommandManager()->Execute(&settings) == B_OK);

	BMessage	*recording	= doc->GetCommandManager()->GetRecording();
	CPPUNIT_ASSERT(recording != NULL);
	BMessage	recorded;
	int32		lastIndex	= 0;
	while (recording->FindMessage("Macro::Commmand",lastIndex,&recorded) == B_OK)
		lastIndex++;
	CPPUNIT_ASSERT(lastIndex > 0);
	CPPUNIT_ASSERT(recording->FindMessage("Macro::Commmand",lastIndex-1,&recorded) == B_OK);

	bool	selectedFlag	= false;
	CPPUNIT_ASSERT((recorded.FindBool(P_C_NODE_SELECTED,&selectedFlag) != B_OK) || !selectedFlag);
	// the pre-existing (not #132's own) pointer->id conversion still ran -
	// either an int32 id (already-seen node) or an embedded "included_node"
	// (first time this Indexer has seen it) is present either way.
	int32	nodeId		= 0;
	bool	hasIncluded	= recorded.HasMessage("included_node");
	CPPUNIT_ASSERT((recorded.FindInt32("node",&nodeId) == B_OK) || hasIncluded);
}


void PCommandTest::FindDefaultScopeSearchesOnlyNodes(void)
{
	// #133: Find::FindNodes() used to walk doc->GetAllNodes() only, no way
	// to reach connections at all - the default (no "scope" field) has to
	// keep behaving exactly that way, or every existing macro/shortcut
	// using Find without ever knowing "scope" exists would start also
	// matching connections it never used to.
	PDocument	*doc	= NewHeadlessTestDocument();

	BMessage	*node	= new BMessage(P_C_CLASS_TYPE);
	node->AddBool(P_C_NODE_SELECTED,false);
	node->AddString("Node::name","apple");
	doc->GetAllNodes()->AddItem(node);

	BMessage	*connection	= new BMessage(P_C_CONNECTION_TYPE);
	connection->AddBool(P_C_NODE_SELECTED,false);
	connection->AddString("Node::name","apple");
	doc->GetAllConnections()->AddItem(connection);

	BMessage	settings;
	settings.AddString("searchString","apple");

	Find	command;
	BMessage	*result	= command.Do(doc,&settings);
	CPPUNIT_ASSERT(result != NULL);

	CPPUNIT_ASSERT(doc->GetSelected()->HasItem(node));
	CPPUNIT_ASSERT(!doc->GetSelected()->HasItem(connection));
}


void PCommandTest::FindScopeBothIncludesConnections(void)
{
	PDocument	*doc	= NewHeadlessTestDocument();

	BMessage	*node	= new BMessage(P_C_CLASS_TYPE);
	node->AddBool(P_C_NODE_SELECTED,false);
	node->AddString("Node::name","apple");
	doc->GetAllNodes()->AddItem(node);

	BMessage	*connection	= new BMessage(P_C_CONNECTION_TYPE);
	connection->AddBool(P_C_NODE_SELECTED,false);
	connection->AddString("Node::name","apple");
	doc->GetAllConnections()->AddItem(connection);

	BMessage	settings;
	settings.AddString("searchString","apple");
	settings.AddString("scope","both");

	Find	command;
	BMessage	*result	= command.Do(doc,&settings);
	CPPUNIT_ASSERT(result != NULL);

	CPPUNIT_ASSERT(doc->GetSelected()->HasItem(node));
	CPPUNIT_ASSERT(doc->GetSelected()->HasItem(connection));
}


void PCommandTest::FindSetOperationAddUnionsWithSelection(void)
{
	// #133: without an "add" (or subtract/intersect) mode, a second Find
	// always throws away whatever the first one selected - "everything
	// matching X, plus everything matching Y" wasn't expressible at all.
	PDocument	*doc	= NewHeadlessTestDocument();

	BMessage	*nodeA	= new BMessage(P_C_CLASS_TYPE);
	nodeA->AddBool(P_C_NODE_SELECTED,true);
	nodeA->AddString("Node::name","alpha");
	doc->GetAllNodes()->AddItem(nodeA);
	doc->GetSelected()->AddItem(nodeA);

	BMessage	*nodeB	= new BMessage(P_C_CLASS_TYPE);
	nodeB->AddBool(P_C_NODE_SELECTED,false);
	nodeB->AddString("Node::name","beta");
	doc->GetAllNodes()->AddItem(nodeB);

	BMessage	settings;
	settings.AddString("searchString","beta");
	settings.AddString("setOperation","add");

	Find	command;
	BMessage	*result	= command.Do(doc,&settings);
	CPPUNIT_ASSERT(result != NULL);

	CPPUNIT_ASSERT_EQUAL((int32)2,doc->GetSelected()->CountItems());
	CPPUNIT_ASSERT(doc->GetSelected()->HasItem(nodeA));
	CPPUNIT_ASSERT(doc->GetSelected()->HasItem(nodeB));
}


void PCommandTest::FindSetOperationSubtractRemovesMatches(void)
{
	PDocument	*doc	= NewHeadlessTestDocument();

	BMessage	*nodeA	= new BMessage(P_C_CLASS_TYPE);
	nodeA->AddBool(P_C_NODE_SELECTED,true);
	nodeA->AddString("Node::name","alpha");
	doc->GetAllNodes()->AddItem(nodeA);
	doc->GetSelected()->AddItem(nodeA);

	BMessage	*nodeB	= new BMessage(P_C_CLASS_TYPE);
	nodeB->AddBool(P_C_NODE_SELECTED,true);
	nodeB->AddString("Node::name","beta");
	doc->GetAllNodes()->AddItem(nodeB);
	doc->GetSelected()->AddItem(nodeB);

	BMessage	settings;
	settings.AddString("searchString","beta");
	settings.AddString("setOperation","subtract");

	Find	command;
	BMessage	*result	= command.Do(doc,&settings);
	CPPUNIT_ASSERT(result != NULL);

	CPPUNIT_ASSERT_EQUAL((int32)1,doc->GetSelected()->CountItems());
	CPPUNIT_ASSERT(doc->GetSelected()->HasItem(nodeA));
	CPPUNIT_ASSERT(!doc->GetSelected()->HasItem(nodeB));
}


void PCommandTest::FindSetOperationIntersectKeepsOnlyMatches(void)
{
	PDocument	*doc	= NewHeadlessTestDocument();

	BMessage	*nodeA	= new BMessage(P_C_CLASS_TYPE);
	nodeA->AddBool(P_C_NODE_SELECTED,true);
	nodeA->AddString("Node::name","shared");
	doc->GetAllNodes()->AddItem(nodeA);
	doc->GetSelected()->AddItem(nodeA);

	BMessage	*nodeB	= new BMessage(P_C_CLASS_TYPE);
	nodeB->AddBool(P_C_NODE_SELECTED,true);
	nodeB->AddString("Node::name","onlyselected");
	doc->GetAllNodes()->AddItem(nodeB);
	doc->GetSelected()->AddItem(nodeB);

	BMessage	*nodeC	= new BMessage(P_C_CLASS_TYPE);
	nodeC->AddBool(P_C_NODE_SELECTED,false);
	nodeC->AddString("Node::name","shared");
	doc->GetAllNodes()->AddItem(nodeC);

	BMessage	settings;
	settings.AddString("searchString","shared");
	settings.AddString("setOperation","intersect");

	Find	command;
	BMessage	*result	= command.Do(doc,&settings);
	CPPUNIT_ASSERT(result != NULL);

	// only nodeA matches BOTH "was selected" and "matches the search" -
	// nodeB was selected but doesn't match, nodeC matches but wasn't
	// selected, neither belongs in an intersection of the two
	CPPUNIT_ASSERT_EQUAL((int32)1,doc->GetSelected()->CountItems());
	CPPUNIT_ASSERT(doc->GetSelected()->HasItem(nodeA));
}


void PCommandTest::FindDoUndoRestoresExactPriorSelection(void)
{
	// regression check for the dead Undo() this rewrite replaced - it read
	// "node" pointers off the command's own top-level settings message,
	// which Do() never actually stored there (a local variable holding
	// them was silently discarded by an unrelated reassignment a few lines
	// later), so Undo() restored nothing at all. Do() now stores the prior
	// selection in a proper "Find::Undo" submessage, matching every other
	// command in this codebase.
	PDocument	*doc	= NewHeadlessTestDocument();

	BMessage	*nodeA	= new BMessage(P_C_CLASS_TYPE);
	nodeA->AddBool(P_C_NODE_SELECTED,true);
	nodeA->AddString("Node::name","alpha");
	doc->GetAllNodes()->AddItem(nodeA);
	doc->GetSelected()->AddItem(nodeA);

	BMessage	*nodeB	= new BMessage(P_C_CLASS_TYPE);
	nodeB->AddBool(P_C_NODE_SELECTED,false);
	nodeB->AddString("Node::name","beta");
	doc->GetAllNodes()->AddItem(nodeB);

	BMessage	settings;
	settings.AddString("searchString","beta");

	Find	command;
	BMessage	*result	= command.Do(doc,&settings);
	CPPUNIT_ASSERT(result != NULL);
	// sanity: the find itself worked as expected before checking undo
	CPPUNIT_ASSERT(doc->GetSelected()->HasItem(nodeB));
	CPPUNIT_ASSERT(!doc->GetSelected()->HasItem(nodeA));

	command.Undo(doc,result);

	CPPUNIT_ASSERT_EQUAL((int32)1,doc->GetSelected()->CountItems());
	CPPUNIT_ASSERT(doc->GetSelected()->HasItem(nodeA));
	CPPUNIT_ASSERT(!doc->GetSelected()->HasItem(nodeB));
	bool	nodeASelected	= false;
	CPPUNIT_ASSERT(nodeA->FindBool(P_C_NODE_SELECTED,&nodeASelected) == B_OK);
	CPPUNIT_ASSERT(nodeASelected);
	bool	nodeBSelected	= true;
	CPPUNIT_ASSERT(nodeB->FindBool(P_C_NODE_SELECTED,&nodeBSelected) == B_OK);
	CPPUNIT_ASSERT(!nodeBSelected);
}


void PCommandTest::RepeatRunsChildNTimesWithCounterBinding(void)
{
	// #135 end-to-end: PlayMacro() owning the value context, Repeat
	// publishing its 0-based iteration counter into it each pass, and
	// PCommandManager::ResolveBindings() (invoked via PCommand::
	// RunSubCommandsOnce(), since Move here runs as Repeat's own
	// subPCommand child, not as a top-level Execute() call) picking that
	// counter back up for a child field bound to "$i" - all without going
	// through the MacroEditor DSL text at all, a macro built directly as
	// BMessages the way IndexMacroCommand()/DeIndexCommand() actually
	// shape one.
	PDocument	*doc	= NewHeadlessTestDocument();
	doc->GetCommandManager()->RegisterPCommand(new TestRepeatPlugin());
	doc->GetCommandManager()->RegisterPCommand(new TestMovePlugin());

	BMessage	*node	= new BMessage(P_C_CLASS_TYPE);
	node->AddRect(P_C_NODE_FRAME,BRect(0,0,50,50));
	doc->GetAllNodes()->AddItem(node);
	doc->GetSelected()->AddItem(node);

	BMessage	moveTemplate;
	moveTemplate.AddString("Command::Name","Move");
	moveTemplate.AddFloat("dy",0.0f);
	BMessage	moveBindings;
	moveBindings.AddString("dx","i");
	moveTemplate.AddMessage("PCommand::bindings",&moveBindings);

	BMessage	repeatCmd;
	repeatCmd.AddString("Command::Name","Repeat");
	repeatCmd.AddInt32("count",3);
	repeatCmd.AddString("counterVariable","i");
	repeatCmd.AddMessage("PCommand::subPCommand",&moveTemplate);

	BMessage	macro(P_C_MACRO_TYPE);
	macro.AddMessage("Macro::Commmand",&repeatCmd);

	doc->GetCommandManager()->PlayMacro(&macro);

	// three passes, dx bound to the counter each time: 0 + 1 + 2 = 3
	BRect	frame;
	CPPUNIT_ASSERT(node->FindRect(P_C_NODE_FRAME,&frame) == B_OK);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0,frame.left,0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0,frame.top,0.001);

	// the value context is scoped to the PlayMacro() call itself - nothing
	// should still be reachable/observable once it returns
	CPPUNIT_ASSERT(doc->GetCommandManager()->GetValueContext() == NULL);
}


void PCommandTest::RepeatUndoReversesAllIterations(void)
{
	PDocument	*doc	= NewHeadlessTestDocument();
	doc->GetCommandManager()->RegisterPCommand(new TestRepeatPlugin());
	doc->GetCommandManager()->RegisterPCommand(new TestMovePlugin());

	BMessage	*node	= new BMessage(P_C_CLASS_TYPE);
	node->AddRect(P_C_NODE_FRAME,BRect(0,0,50,50));
	doc->GetAllNodes()->AddItem(node);
	doc->GetSelected()->AddItem(node);

	BMessage	moveTemplate;
	moveTemplate.AddString("Command::Name","Move");
	moveTemplate.AddFloat("dy",0.0f);
	BMessage	moveBindings;
	moveBindings.AddString("dx","i");
	moveTemplate.AddMessage("PCommand::bindings",&moveBindings);

	BMessage	repeatCmd;
	repeatCmd.AddString("Command::Name","Repeat");
	repeatCmd.AddInt32("count",3);
	repeatCmd.AddString("counterVariable","i");
	repeatCmd.AddMessage("PCommand::subPCommand",&moveTemplate);

	BMessage	macro(P_C_MACRO_TYPE);
	macro.AddMessage("Macro::Commmand",&repeatCmd);

	doc->GetCommandManager()->PlayMacro(&macro);
	BRect	moved;
	CPPUNIT_ASSERT(node->FindRect(P_C_NODE_FRAME,&moved) == B_OK);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0,moved.left,0.001);

	BMessage	*undoEntry	= (BMessage*)doc->GetCommandManager()->GetUndoList()->LastItem();
	CPPUNIT_ASSERT(undoEntry != NULL);
	doc->GetCommandManager()->Undo(undoEntry);

	// all three Move iterations rolled back - exactly the original frame
	BRect	restored;
	CPPUNIT_ASSERT(node->FindRect(P_C_NODE_FRAME,&restored) == B_OK);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0,restored.left,0.001);
	CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0,restored.top,0.001);
}


void PCommandTest::ForEachRunsOncePerSelectedNodeWithNodeBinding(void)
{
	// #135: unlike ChangeValue's own Node::selected=true form (all
	// selected nodes at once, in one Do() call), ForEach's whole point is
	// running its child once *per* node, each with that one node's own
	// pointer bound in - here checked via ChangeValue's "node" field,
	// setting each node's own TestValue independently based on which
	// node this particular pass is on (a plain Node::selected=true
	// ChangeValue could never tell them apart to do that).
	PDocument	*doc	= NewHeadlessTestDocument();
	doc->GetCommandManager()->RegisterPCommand(new TestForEachPlugin());
	doc->GetCommandManager()->RegisterPCommand(new TestChangeValuePlugin());

	BMessage	*nodeA	= new BMessage(P_C_CLASS_TYPE);
	nodeA->AddInt32("TestValue",0);
	doc->GetAllNodes()->AddItem(nodeA);
	doc->GetSelected()->AddItem(nodeA);

	BMessage	*nodeB	= new BMessage(P_C_CLASS_TYPE);
	nodeB->AddInt32("TestValue",0);
	doc->GetAllNodes()->AddItem(nodeB);
	doc->GetSelected()->AddItem(nodeB);

	BMessage	*valueContainer	= new BMessage();
	valueContainer->AddString("name","TestValue");
	valueContainer->AddInt32("type",(int32)B_INT32_TYPE);
	valueContainer->AddInt32("index",0);
	int32	newValue	= 99;
	valueContainer->AddData("newValue",B_INT32_TYPE,&newValue,sizeof(int32));

	BMessage	changeValueTemplate;
	changeValueTemplate.AddString("Command::Name","ChangeValue");
	changeValueTemplate.AddMessage("valueContainer",valueContainer);
	BMessage	nodeBindings;
	nodeBindings.AddString("node","current");
	changeValueTemplate.AddMessage("PCommand::bindings",&nodeBindings);

	BMessage	forEachCmd;
	forEachCmd.AddString("Command::Name","ForEach");
	forEachCmd.AddString("nodeVariable","current");
	forEachCmd.AddMessage("PCommand::subPCommand",&changeValueTemplate);

	BMessage	macro(P_C_MACRO_TYPE);
	macro.AddMessage("Macro::Commmand",&forEachCmd);

	doc->GetCommandManager()->PlayMacro(&macro);

	int32	valueA	= 0, valueB = 0;
	CPPUNIT_ASSERT(nodeA->FindInt32("TestValue",&valueA) == B_OK);
	CPPUNIT_ASSERT(nodeB->FindInt32("TestValue",&valueB) == B_OK);
	CPPUNIT_ASSERT_EQUAL((int32)99,valueA);
	CPPUNIT_ASSERT_EQUAL((int32)99,valueB);
}


void PCommandTest::IfRunsChildOnlyWhenPredicateMatches(void)
{
	PDocument	*doc	= NewHeadlessTestDocument();
	doc->GetCommandManager()->RegisterPCommand(new TestIfPlugin());
	doc->GetCommandManager()->RegisterPCommand(new TestChangeValuePlugin());

	BMessage	*node	= new BMessage(P_C_CLASS_TYPE);
	node->AddInt32("TestValue",0);
	node->AddString("Node::name","findme");
	doc->GetAllNodes()->AddItem(node);
	doc->GetSelected()->AddItem(node);

	BMessage	*valueContainer	= new BMessage();
	valueContainer->AddString("name","TestValue");
	valueContainer->AddInt32("type",(int32)B_INT32_TYPE);
	valueContainer->AddInt32("index",0);
	int32	newValue	= 42;
	valueContainer->AddData("newValue",B_INT32_TYPE,&newValue,sizeof(int32));

	BMessage	changeValueTemplate;
	changeValueTemplate.AddString("Command::Name","ChangeValue");
	changeValueTemplate.AddBool(P_C_NODE_SELECTED,true);
	changeValueTemplate.AddMessage("valueContainer",valueContainer);

	// matching predicate - child runs
	BMessage	ifMatch;
	ifMatch.AddString("Command::Name","If");
	ifMatch.AddString("searchString","findme");
	ifMatch.AddMessage("PCommand::subPCommand",&changeValueTemplate);

	BMessage	macroMatch(P_C_MACRO_TYPE);
	macroMatch.AddMessage("Macro::Commmand",&ifMatch);
	doc->GetCommandManager()->PlayMacro(&macroMatch);

	int32	afterMatch	= 0;
	CPPUNIT_ASSERT(node->FindInt32("TestValue",&afterMatch) == B_OK);
	CPPUNIT_ASSERT_EQUAL((int32)42,afterMatch);

	// non-matching predicate - child does NOT run, value stays as it was
	BMessage	changeValueTemplate2;
	changeValueTemplate2.AddString("Command::Name","ChangeValue");
	changeValueTemplate2.AddBool(P_C_NODE_SELECTED,true);
	BMessage	*valueContainer2	= new BMessage();
	valueContainer2->AddString("name","TestValue");
	valueContainer2->AddInt32("type",(int32)B_INT32_TYPE);
	valueContainer2->AddInt32("index",0);
	int32	newValue2	= 7;
	valueContainer2->AddData("newValue",B_INT32_TYPE,&newValue2,sizeof(int32));
	changeValueTemplate2.AddMessage("valueContainer",valueContainer2);

	BMessage	ifNoMatch;
	ifNoMatch.AddString("Command::Name","If");
	ifNoMatch.AddString("searchString","nothing-matches-this");
	ifNoMatch.AddMessage("PCommand::subPCommand",&changeValueTemplate2);

	BMessage	macroNoMatch(P_C_MACRO_TYPE);
	macroNoMatch.AddMessage("Macro::Commmand",&ifNoMatch);
	doc->GetCommandManager()->PlayMacro(&macroNoMatch);

	int32	afterNoMatch	= 0;
	CPPUNIT_ASSERT(node->FindInt32("TestValue",&afterNoMatch) == B_OK);
	CPPUNIT_ASSERT_EQUAL((int32)42,afterNoMatch);
}


void PCommandTest::RememberThenBoundSelectRestoresSelection(void)
{
	// #135: Remember has no "restore" mode of its own by design (see the
	// class comment on Remember.h) - restoring is just a plain Select
	// with its own "node" field bound to the remembered variable. This
	// checks that combination actually works end to end: bindings
	// resolution copying every one of a variable's repeated pointer
	// entries into Select's own repeated "node" field correctly, not
	// just a single value.
	PDocument	*doc	= NewHeadlessTestDocument();
	doc->GetCommandManager()->RegisterPCommand(new TestRememberPlugin());
	doc->GetCommandManager()->RegisterPCommand(new TestSelectPlugin());

	BMessage	*nodeA	= new BMessage(P_C_CLASS_TYPE);
	nodeA->AddBool(P_C_NODE_SELECTED,true);
	doc->GetAllNodes()->AddItem(nodeA);
	doc->GetSelected()->AddItem(nodeA);

	BMessage	*nodeB	= new BMessage(P_C_CLASS_TYPE);
	nodeB->AddBool(P_C_NODE_SELECTED,true);
	doc->GetAllNodes()->AddItem(nodeB);
	doc->GetSelected()->AddItem(nodeB);

	BMessage	*nodeC	= new BMessage(P_C_CLASS_TYPE);
	nodeC->AddBool(P_C_NODE_SELECTED,false);
	doc->GetAllNodes()->AddItem(nodeC);

	BMessage	rememberCmd;
	rememberCmd.AddString("Command::Name","Remember");
	rememberCmd.AddString("variable","saved");

	// deselect everything, select only nodeC instead - proves the later
	// bound Select genuinely restores {nodeA,nodeB}, not just "whatever
	// was already selected"
	BMessage	deselectCmd;
	deselectCmd.AddString("Command::Name","Select");
	deselectCmd.AddBool("deselect",true);
	deselectCmd.AddPointer("node",nodeC);

	BMessage	restoreCmd;
	restoreCmd.AddString("Command::Name","Select");
	restoreCmd.AddBool("deselect",true);
	BMessage	restoreBindings;
	restoreBindings.AddString("node","saved");
	restoreCmd.AddMessage("PCommand::bindings",&restoreBindings);

	BMessage	macro(P_C_MACRO_TYPE);
	macro.AddMessage("Macro::Commmand",&rememberCmd);
	macro.AddMessage("Macro::Commmand",&deselectCmd);
	macro.AddMessage("Macro::Commmand",&restoreCmd);

	doc->GetCommandManager()->PlayMacro(&macro);

	CPPUNIT_ASSERT_EQUAL((int32)2,doc->GetSelected()->CountItems());
	CPPUNIT_ASSERT(doc->GetSelected()->HasItem(nodeA));
	CPPUNIT_ASSERT(doc->GetSelected()->HasItem(nodeB));
	CPPUNIT_ASSERT(!doc->GetSelected()->HasItem(nodeC));
}


void PCommandTest::SleepReturnsCleanlyAndKeepsDocumentLockUsable(void)
{
	// Sleep::Do() releases the document lock for the actual wait
	// (doc->Unlock() then doc->Lock() again around snooze() - see its own
	// class comment for why: PCommandManager::Execute() holds that lock
	// for its whole Do() call, which would otherwise keep the exact
	// redraw a pause exists to make visible from happening during it).
	// BLooper::Lock()/Unlock() are reentrant per-thread via an internal
	// count (confirmed against Looper.cpp itself, not assumed) - this
	// checks that dance doesn't leave that count corrupted: the document
	// must still lock/unlock normally, and Execute() itself must still
	// return B_OK, once Sleep::Do() has released and reacquired it once
	// from inside an already-locked Execute() call.
	PDocument	*doc	= NewHeadlessTestDocument();
	doc->GetCommandManager()->RegisterPCommand(new TestSleepPlugin());

	BMessage	settings(P_C_EXECUTE_COMMAND);
	settings.AddString("Command::Name","Sleep");
	settings.AddInt32("milliseconds",5);

	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,doc->GetCommandManager()->Execute(&settings));

	CPPUNIT_ASSERT(doc->Lock());
	doc->Unlock();
}


void PCommandTest::SleepWithNoMillisecondsFieldIsANoOp(void)
{
	// no silent fallback (project convention) - a missing "milliseconds"
	// means nothing to wait for, not an invented default duration
	PDocument	*doc	= NewHeadlessTestDocument();
	doc->GetCommandManager()->RegisterPCommand(new TestSleepPlugin());

	BMessage	settings(P_C_EXECUTE_COMMAND);
	settings.AddString("Command::Name","Sleep");

	bigtime_t	start	= system_time();
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,doc->GetCommandManager()->Execute(&settings));
	CPPUNIT_ASSERT((system_time()-start) < 50000);
}


void PCommandTest::SleepUndoDoesNothing(void)
{
	PDocument	*doc	= NewHeadlessTestDocument();
	BMessage	settings;
	settings.AddInt32("milliseconds",5);

	Sleep	command;
	command.Undo(doc,&settings);	// must not crash - nothing to undo
}


void PCommandTest::MenuSearchFindForwardsShadowFlagToRecording(void)
{
	// user report: typing a search live while recording (FindWindow's
	// 'live' case, which sends "shadow"=true - see FindWindow.cpp)
	// recorded one macro entry per keystroke instead of being filtered
	// out entirely. Root cause: PDocument::MessageReceived()'s own
	// MENU_SEARCH_FIND handler built a brand new settings BMessage but
	// never copied the incoming message's own "shadow" field onto it -
	// the line that would have was commented out - so
	// PCommandManager::Execute()'s own "shadow means don't record" check
	// never saw it, regardless of what FindWindow actually sent.
	PDocument	*doc	= NewHeadlessTestDocument();
	doc->GetCommandManager()->RegisterPCommand(new TestFindPlugin());

	BMessage	*node	= new BMessage(P_C_CLASS_TYPE);
	node->AddString("Node::name","findme");
	doc->GetAllNodes()->AddItem(node);

	doc->GetCommandManager()->StartMacro();

	BMessage	liveSearch(MENU_SEARCH_FIND);
	liveSearch.AddString("searchString","findme");
	liveSearch.AddBool("shadow",true);
	BMessenger	sender(doc);
	CPPUNIT_ASSERT(sender.SendMessage(&liveSearch) == B_OK);
	snooze(200000);

	BMessage	*recording	= doc->GetCommandManager()->GetRecording();
	CPPUNIT_ASSERT(recording != NULL);
	CPPUNIT_ASSERT(!recording->HasMessage("Macro::Commmand"));

	BMessage	realSearch(MENU_SEARCH_FIND);
	realSearch.AddString("searchString","findme");
	CPPUNIT_ASSERT(sender.SendMessage(&realSearch) == B_OK);
	snooze(200000);

	CPPUNIT_ASSERT(recording->HasMessage("Macro::Commmand"));
}
