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
