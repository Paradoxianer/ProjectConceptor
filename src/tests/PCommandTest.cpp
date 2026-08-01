#include "PCommandTest.h"

#include <app/Message.h>

#include "ChangeValue.h"
#include "PDocument.h"
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
