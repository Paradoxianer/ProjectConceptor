#include "PCommandTest.h"

#include <app/Message.h>
#include <interface/Rect.h>
#include <support/List.h>

#include "ChangeValue.h"
#include "Group.h"
#include "Insert.h"
#include "PDocument.h"
#include "ProjectConceptorDefs.h"
#include "TestDocument.h"

CPPUNIT_TEST_SUITE_REGISTRATION(PCommandTest);

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
