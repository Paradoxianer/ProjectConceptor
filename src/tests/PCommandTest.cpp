#include "PCommandTest.h"

#include <app/Message.h>
#include <interface/Rect.h>
#include <support/List.h>

#include "BasePlugin.h"
#include "ChangeValue.h"
#include "Group.h"
#include "Insert.h"
#include "PCommandManager.h"
#include "PDocument.h"
#include "ProjectConceptorDefs.h"
#include "TestDocument.h"

CPPUNIT_TEST_SUITE_REGISTRATION(PCommandTest);

namespace {

// Minimal BasePlugin so a real ChangeValue instance can be registered
// with a PCommandManager under a chosen name, without pulling in the
// dynamic-loading machinery real plugin .so's use - GetNewObject() is
// the only part PCommandManager::RegisterPCommand() actually calls.
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

// A bare wrapper command - Do()/Undo() are inherited straight from
// PCommand, unoverridden, so calling them here exercises exactly the
// PCommand::subPCommand loop being tested and nothing else.
class TestWrapperCommand : public PCommand {
public:
	virtual void	AttachedToManager(void) {}
	virtual void	DetachedFromManager(void) {}
	virtual char*	Name(void) { return (char *)"TestWrapper"; }
};

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
	// regression test for issue #116: PCommand::Do()'s subPCommand
	// write-back used ReplaceMessage()'s unindexed 2-arg overload, which
	// always targets slot 0 regardless of loop index - so a wrapper
	// carrying more than one PCommand::subPCommand only ever got undo
	// info written into the LAST slot. Undo() then silently restored
	// only that last subcommand and left every earlier one applied.
	// Two ChangeValue subcommands here, each targeting a different node -
	// without the fix, node1 (processed first, slot 0) never gets its
	// undo info and Undo() is a no-op for it.
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
