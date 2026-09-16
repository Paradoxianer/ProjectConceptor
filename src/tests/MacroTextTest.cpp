#include "MacroTextTest.h"

#include <app/Message.h>
#include <app/PropertyInfo.h>
#include <interface/Rect.h>
#include <support/List.h>
#include <support/String.h>

#include "BasePlugin.h"
#include "ChangeValue.h"
#include "Group.h"
#include "Insert.h"
#include "MacroText.h"
#include "Move.h"
#include "PCommand.h"
#include "PCommandManager.h"
#include "PDocument.h"
#include "ProjectConceptorDefs.h"
#include "Select.h"
#include "TestDocument.h"

CPPUNIT_TEST_SUITE_REGISTRATION(MacroTextTest);

namespace {

// Minimal BasePlugin wrappers to register real commands without a real
// plugin .so - same technique PCommandTest.cpp/LayoutEditorTest.cpp
// already use.
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

TEST_PLUGIN(TestInsertPlugin,Insert,"Insert")
TEST_PLUGIN(TestMovePlugin,Move,"Move")
TEST_PLUGIN(TestGroupPlugin,Group,"Group")
TEST_PLUGIN(TestSelectPlugin,Select,"Select")
TEST_PLUGIN(TestChangeValuePlugin,ChangeValue,"ChangeValue")

#undef TEST_PLUGIN

// A single string field - only real production command with a bare
// top-level string field is Find, which drags in FindWindow's UI
// dependency; this stand-in is lighter and exercises exactly the DSL
// path MacroText.cpp cares about (quoting/escaping).
class TestStringCommand : public PCommand {
public:
	virtual void	AttachedToManager(void) {}
	virtual void	DetachedFromManager(void) {}
	virtual char*	Name(void) { return (char *)"TestString"; }
	virtual const property_info* PropertyInfo(int32 *count) {
		static const property_info	props[] = {
			{ "TestString", { B_EXECUTE_PROPERTY, 0 }, { B_DIRECT_SPECIFIER, 0 },
				"test", 0, {0}, { { { {"text", B_STRING_TYPE} } } } },
		};
		*count	= 1;
		return props;
	}
};

class TestStringPlugin : public BasePlugin {
public:
	TestStringPlugin(void) : BasePlugin(0) {}
	virtual char*	GetName(void) { return (char *)"TestString"; }
	virtual char*	GetAutor(void) { return (char *)"test"; }
	virtual char*	GetVersionsString(void) { return (char *)"0"; }
	virtual char*	GetDescription(void) { return (char *)"test"; }
	virtual uint32	GetType(void) { return P_C_COMMANDO_PLUGIN_TYPE; }
	virtual void*	GetNewObject(void *value) { return new TestStringCommand(); }
};

// A field type with no readable syntax at all (not bool/int/float/string/
// point/rect/BMessage) - the only thing left in MacroText.cpp's default:
// raw:<type_code>:<base64> escape hatch once B_MESSAGE_TYPE gets its own
// "~fieldName" block syntax (see RawEscapeHatchPreservesOpaqueType below).
class TestRawCommand : public PCommand {
public:
	virtual void	AttachedToManager(void) {}
	virtual void	DetachedFromManager(void) {}
	virtual char*	Name(void) { return (char *)"TestRaw"; }
	virtual const property_info* PropertyInfo(int32 *count) {
		static const property_info	props[] = {
			{ "TestRaw", { B_EXECUTE_PROPERTY, 0 }, { B_DIRECT_SPECIFIER, 0 },
				"test", 0, {0}, { { { {"blob", (type_code)'TRBL'} } } } },
		};
		*count	= 1;
		return props;
	}
};

class TestRawPlugin : public BasePlugin {
public:
	TestRawPlugin(void) : BasePlugin(0) {}
	virtual char*	GetName(void) { return (char *)"TestRaw"; }
	virtual char*	GetAutor(void) { return (char *)"test"; }
	virtual char*	GetVersionsString(void) { return (char *)"0"; }
	virtual char*	GetDescription(void) { return (char *)"test"; }
	virtual uint32	GetType(void) { return P_C_COMMANDO_PLUGIN_TYPE; }
	virtual void*	GetNewObject(void *value) { return new TestRawCommand(); }
};

PDocument* NewRegisteredTestDocument(void)
{
	PDocument	*doc	= NewHeadlessTestDocument();
	doc->GetCommandManager()->RegisterPCommand(new TestInsertPlugin());
	doc->GetCommandManager()->RegisterPCommand(new TestMovePlugin());
	doc->GetCommandManager()->RegisterPCommand(new TestGroupPlugin());
	doc->GetCommandManager()->RegisterPCommand(new TestSelectPlugin());
	doc->GetCommandManager()->RegisterPCommand(new TestChangeValuePlugin());
	doc->GetCommandManager()->RegisterPCommand(new TestStringPlugin());
	doc->GetCommandManager()->RegisterPCommand(new TestRawPlugin());
	// Batch is already compiled into every real PCommandManager - not
	// registered via plugin loading in a headless test doc either, so it
	// needs the same manual registration as everything else here.
	return doc;
}

}


void MacroTextTest::RoundTripsInsertWithNodeRef(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();

	BMessage	insert;
	insert.AddString("Command::Name","Insert");
	insert.AddInt32("node",7);

	BList	commands;
	commands.AddItem(&insert);
	BString	text;
	SerializeCommands(&commands,&text);
	CPPUNIT_ASSERT(text.FindFirst("Insert node=@7") == 0);

	BList		parsed;
	BString		error;
	status_t	err	= ParseCommands(text,&parsed,doc->GetCommandManager(),&error);
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,err);
	CPPUNIT_ASSERT_EQUAL((int32)1,parsed.CountItems());

	BMessage	*result	= (BMessage*)parsed.ItemAt(0);
	const char	*name	= NULL;
	result->FindString("Command::Name",&name);
	CPPUNIT_ASSERT(strcmp(name,"Insert") == 0);
	int32	node	= -1;
	CPPUNIT_ASSERT_EQUAL(B_OK,result->FindInt32("node",&node));
	CPPUNIT_ASSERT_EQUAL((int32)7,node);
}


void MacroTextTest::RoundTripsMoveWithFloats(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();

	BMessage	move;
	move.AddString("Command::Name","Move");
	move.AddFloat("dx",10.5f);
	move.AddFloat("dy",-3.0f);

	BList	commands;
	commands.AddItem(&move);
	BString	text;
	SerializeCommands(&commands,&text);

	BList		parsed;
	BString		error;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,ParseCommands(text,&parsed,doc->GetCommandManager(),&error));

	BMessage	*result	= (BMessage*)parsed.ItemAt(0);
	float	dx	= 0, dy = 0;
	CPPUNIT_ASSERT_EQUAL(B_OK,result->FindFloat("dx",&dx));
	CPPUNIT_ASSERT_EQUAL(B_OK,result->FindFloat("dy",&dy));
	CPPUNIT_ASSERT((dx > 10.49f) && (dx < 10.51f));
	CPPUNIT_ASSERT((dy > -3.01f) && (dy < -2.99f));
}


void MacroTextTest::RoundTripsGroupWithBoolAndNodeRef(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();

	BMessage	group;
	group.AddString("Command::Name","Group");
	group.AddInt32("node",3);
	group.AddBool("deselect",true);

	BList	commands;
	commands.AddItem(&group);
	BString	text;
	SerializeCommands(&commands,&text);

	BList		parsed;
	BString		error;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,ParseCommands(text,&parsed,doc->GetCommandManager(),&error));

	BMessage	*result	= (BMessage*)parsed.ItemAt(0);
	int32	node	= -1;
	bool	deselect	= false;
	CPPUNIT_ASSERT_EQUAL(B_OK,result->FindInt32("node",&node));
	CPPUNIT_ASSERT_EQUAL(B_OK,result->FindBool("deselect",&deselect));
	CPPUNIT_ASSERT_EQUAL((int32)3,node);
	CPPUNIT_ASSERT(deselect);
}


void MacroTextTest::RoundTripsSelectWithRepeatedFields(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();

	BMessage	select;
	select.AddString("Command::Name","Select");
	select.AddInt32("node",1);
	select.AddInt32("node",2);
	select.AddRect("frame",BRect(0,0,10,10));
	select.AddBool("selectAll",false);

	BList	commands;
	commands.AddItem(&select);
	BString	text;
	SerializeCommands(&commands,&text);

	BList		parsed;
	BString		error;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,ParseCommands(text,&parsed,doc->GetCommandManager(),&error));

	BMessage	*result	= (BMessage*)parsed.ItemAt(0);
	int32	node0	= -1, node1 = -1;
	CPPUNIT_ASSERT_EQUAL(B_OK,result->FindInt32("node",0,&node0));
	CPPUNIT_ASSERT_EQUAL(B_OK,result->FindInt32("node",1,&node1));
	CPPUNIT_ASSERT_EQUAL((int32)1,node0);
	CPPUNIT_ASSERT_EQUAL((int32)2,node1);
	BRect	frame;
	CPPUNIT_ASSERT_EQUAL(B_OK,result->FindRect("frame",&frame));
	CPPUNIT_ASSERT(frame == BRect(0,0,10,10));
}


void MacroTextTest::RoundTripsNestedSubCommand(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();

	BMessage	batch;
	batch.AddString("Command::Name","Batch");
	BMessage	child1;
	child1.AddString("Command::Name","Move");
	child1.AddFloat("dx",1.0f);
	child1.AddFloat("dy",2.0f);
	BMessage	child2;
	child2.AddString("Command::Name","Insert");
	child2.AddInt32("node",9);
	batch.AddMessage("PCommand::subPCommand",&child1);
	batch.AddMessage("PCommand::subPCommand",&child2);

	BList	commands;
	commands.AddItem(&batch);
	BString	text;
	SerializeCommands(&commands,&text);
	CPPUNIT_ASSERT(text.FindFirst("  Move") >= 0);
	CPPUNIT_ASSERT(text.FindFirst("  Insert") >= 0);

	BList		parsed;
	BString		error;
	status_t	err	= ParseCommands(text,&parsed,doc->GetCommandManager(),&error);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(error.String(),(status_t)B_OK,err);
	CPPUNIT_ASSERT_EQUAL((int32)1,parsed.CountItems());

	BMessage	*result	= (BMessage*)parsed.ItemAt(0);
	BMessage	sub1,sub2;
	CPPUNIT_ASSERT_EQUAL(B_OK,result->FindMessage("PCommand::subPCommand",0,&sub1));
	CPPUNIT_ASSERT_EQUAL(B_OK,result->FindMessage("PCommand::subPCommand",1,&sub2));
	const char	*sub1Name	= NULL;
	const char	*sub2Name	= NULL;
	sub1.FindString("Command::Name",&sub1Name);
	sub2.FindString("Command::Name",&sub2Name);
	CPPUNIT_ASSERT(strcmp(sub1Name,"Move") == 0);
	CPPUNIT_ASSERT(strcmp(sub2Name,"Insert") == 0);
	int32	insertedNode	= -1;
	CPPUNIT_ASSERT_EQUAL(B_OK,sub2.FindInt32("node",&insertedNode));
	CPPUNIT_ASSERT_EQUAL((int32)9,insertedNode);
}


void MacroTextTest::RoundTripsStringField(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();

	BMessage	cmd;
	cmd.AddString("Command::Name","TestString");
	cmd.AddString("text","say \"hi\" \\ bye");

	BList	commands;
	commands.AddItem(&cmd);
	BString	text;
	SerializeCommands(&commands,&text);

	BList		parsed;
	BString		error;
	status_t	err	= ParseCommands(text,&parsed,doc->GetCommandManager(),&error);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(error.String(),(status_t)B_OK,err);

	BMessage	*result	= (BMessage*)parsed.ItemAt(0);
	const char	*value	= NULL;
	CPPUNIT_ASSERT_EQUAL(B_OK,result->FindString("text",&value));
	CPPUNIT_ASSERT(strcmp(value,"say \"hi\" \\ bye") == 0);
}


void MacroTextTest::RoundTripsNestedFieldBlock(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();

	BMessage	valueContainer;
	valueContainer.AddString("name","Node::name");
	valueContainer.AddInt32("type",(int32)B_STRING_TYPE);
	valueContainer.AddString("newValue","Untitled");

	BMessage	changeValue;
	changeValue.AddString("Command::Name","ChangeValue");
	changeValue.AddInt32("node",5);
	changeValue.AddMessage("valueContainer",&valueContainer);

	BList	commands;
	commands.AddItem(&changeValue);
	BString	text;
	SerializeCommands(&commands,&text);
	// the whole point of the "~fieldName" block syntax: this must now be
	// readable, not fall into the opaque raw:<type>:<base64> escape hatch
	CPPUNIT_ASSERT(text.FindFirst("~valueContainer") >= 0);
	CPPUNIT_ASSERT(text.FindFirst("Node::name") >= 0);
	CPPUNIT_ASSERT(text.FindFirst("raw:") < 0);

	BList		parsed;
	BString		error;
	status_t	err	= ParseCommands(text,&parsed,doc->GetCommandManager(),&error);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(error.String(),(status_t)B_OK,err);

	BMessage	*result	= (BMessage*)parsed.ItemAt(0);
	BMessage	restoredContainer;
	CPPUNIT_ASSERT_EQUAL(B_OK,result->FindMessage("valueContainer",&restoredContainer));
	const char	*restoredName	= NULL;
	const char	*restoredValue	= NULL;
	int32		restoredType	= -1;
	restoredContainer.FindString("name",&restoredName);
	restoredContainer.FindString("newValue",&restoredValue);
	restoredContainer.FindInt32("type",&restoredType);
	CPPUNIT_ASSERT(strcmp(restoredName,"Node::name") == 0);
	CPPUNIT_ASSERT(strcmp(restoredValue,"Untitled") == 0);
	CPPUNIT_ASSERT_EQUAL((int32)B_STRING_TYPE,restoredType);
}


void MacroTextTest::RoundTripsRecursiveNestedFieldBlock(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();

	// two levels deep: Insert's "included_node" (Indexer-embedded full
	// node content) containing its own nested "font" sub-message -
	// proves SerializeFieldBlock()/the "~" parser branch actually recurse,
	// not just handle one level.
	BMessage	font;
	font.AddString("family","Swis721 BT");
	font.AddFloat("size",12.0f);

	BMessage	includedNode;
	includedNode.AddString("name","New Node");
	includedNode.AddRect("frame",BRect(0,0,80,40));
	includedNode.AddMessage("font",&font);

	BMessage	insert;
	insert.AddString("Command::Name","Insert");
	insert.AddInt32("node",11);
	insert.AddMessage("included_node",&includedNode);

	BList	commands;
	commands.AddItem(&insert);
	BString	text;
	SerializeCommands(&commands,&text);
	CPPUNIT_ASSERT(text.FindFirst("~included_node") >= 0);
	CPPUNIT_ASSERT(text.FindFirst("~font") >= 0);
	CPPUNIT_ASSERT(text.FindFirst("raw:") < 0);

	BList		parsed;
	BString		error;
	status_t	err	= ParseCommands(text,&parsed,doc->GetCommandManager(),&error);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(error.String(),(status_t)B_OK,err);

	BMessage	*result	= (BMessage*)parsed.ItemAt(0);
	BMessage	restoredIncluded;
	CPPUNIT_ASSERT_EQUAL(B_OK,result->FindMessage("included_node",&restoredIncluded));
	BRect	restoredFrame;
	CPPUNIT_ASSERT_EQUAL(B_OK,restoredIncluded.FindRect("frame",&restoredFrame));
	CPPUNIT_ASSERT(restoredFrame == BRect(0,0,80,40));
	BMessage	restoredFont;
	CPPUNIT_ASSERT_EQUAL(B_OK,restoredIncluded.FindMessage("font",&restoredFont));
	float	restoredSize	= 0;
	CPPUNIT_ASSERT_EQUAL(B_OK,restoredFont.FindFloat("size",&restoredSize));
	CPPUNIT_ASSERT((restoredSize > 11.99f) && (restoredSize < 12.01f));
}


void MacroTextTest::RoundTripsRepeatedNestedFieldBlocks(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();

	// Indexer attaches one "included_node" per newly-referenced node, so a
	// single Insert command can carry several - same field name repeated,
	// must round-trip in order.
	BMessage	includedA;
	includedA.AddString("name","A");
	BMessage	includedB;
	includedB.AddString("name","B");

	BMessage	insert;
	insert.AddString("Command::Name","Insert");
	insert.AddInt32("node",1);
	insert.AddInt32("node",2);
	insert.AddMessage("included_node",&includedA);
	insert.AddMessage("included_node",&includedB);

	BList	commands;
	commands.AddItem(&insert);
	BString	text;
	SerializeCommands(&commands,&text);

	BList		parsed;
	BString		error;
	status_t	err	= ParseCommands(text,&parsed,doc->GetCommandManager(),&error);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(error.String(),(status_t)B_OK,err);

	BMessage	*result	= (BMessage*)parsed.ItemAt(0);
	BMessage	restoredA,restoredB;
	CPPUNIT_ASSERT_EQUAL(B_OK,result->FindMessage("included_node",0,&restoredA));
	CPPUNIT_ASSERT_EQUAL(B_OK,result->FindMessage("included_node",1,&restoredB));
	const char	*nameA	= NULL;
	const char	*nameB	= NULL;
	restoredA.FindString("name",&nameA);
	restoredB.FindString("name",&nameB);
	CPPUNIT_ASSERT(strcmp(nameA,"A") == 0);
	CPPUNIT_ASSERT(strcmp(nameB,"B") == 0);
}


void MacroTextTest::RawEscapeHatchPreservesOpaqueType(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();

	const char	*payload	= "opaque bytes";
	BMessage	cmd;
	cmd.AddString("Command::Name","TestRaw");
	cmd.AddData("blob",(type_code)'TRBL',payload,strlen(payload)+1);

	BList	commands;
	commands.AddItem(&cmd);
	BString	text;
	SerializeCommands(&commands,&text);
	CPPUNIT_ASSERT(text.FindFirst("raw:") >= 0);

	BList		parsed;
	BString		error;
	status_t	err	= ParseCommands(text,&parsed,doc->GetCommandManager(),&error);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(error.String(),(status_t)B_OK,err);

	BMessage	*result	= (BMessage*)parsed.ItemAt(0);
	const void	*data	= NULL;
	ssize_t		size	= 0;
	CPPUNIT_ASSERT_EQUAL(B_OK,result->FindData("blob",(type_code)'TRBL',&data,&size));
	CPPUNIT_ASSERT(strcmp((const char*)data,payload) == 0);
}


void MacroTextTest::UnknownCommandNameIsRejected(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();

	BList		parsed;
	BString		error;
	status_t	err	= ParseCommands(BString("Frobnicate node=@1\n"),&parsed,
		doc->GetCommandManager(),&error);
	CPPUNIT_ASSERT_EQUAL((status_t)B_NAME_NOT_FOUND,err);
	CPPUNIT_ASSERT_EQUAL((int32)0,parsed.CountItems());
	CPPUNIT_ASSERT(error.FindFirst("Frobnicate") >= 0);
}


void MacroTextTest::UnknownFieldIsRejected(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();

	BList		parsed;
	BString		error;
	status_t	err	= ParseCommands(BString("Move bogus=1\n"),&parsed,
		doc->GetCommandManager(),&error);
	CPPUNIT_ASSERT_EQUAL((status_t)B_BAD_VALUE,err);
	CPPUNIT_ASSERT_EQUAL((int32)0,parsed.CountItems());
	CPPUNIT_ASSERT(error.FindFirst("bogus") >= 0);
}


void MacroTextTest::TypeMismatchIsRejected(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();

	BList		parsed;
	BString		error;
	status_t	err	= ParseCommands(BString("Move dx=\"nope\" dy=1.0\n"),&parsed,
		doc->GetCommandManager(),&error);
	CPPUNIT_ASSERT_EQUAL((status_t)B_BAD_VALUE,err);
	CPPUNIT_ASSERT_EQUAL((int32)0,parsed.CountItems());
}
