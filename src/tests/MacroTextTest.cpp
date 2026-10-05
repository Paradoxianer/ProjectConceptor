#include "MacroTextTest.h"

#include <app/Message.h>
#include <app/PropertyInfo.h>
#include <interface/Rect.h>
#include <support/List.h>
#include <support/String.h>
#include <utility>

#include "AddAttribute.h"
#include "Ask.h"
#include "Batch.h"
#include "ForEach.h"
#include "If.h"
#include "Layout.h"
#include "Remember.h"
#include "Sleep.h"
#include "Calculate.h"
#include "GetValue.h"
#include "SelectConnected.h"
#include "BasePlugin.h"
#include "ChangeValue.h"
#include "Find.h"
#include "Group.h"
#include "Insert.h"
#include <interface/Font.h>
#include <interface/GraphicsDefs.h>
#include <interface/TextView.h>
#include <map>

#include <interface/Window.h>

// the move/insert internals are private on purpose; the test drives them
// directly (same technique this file always used for MacroTextView before it)
#define private public
#include "MacroOutlineView.h"
#undef private
#include "MacroEditor.h"
#include "MacroText.h"
#include <support/DataIO.h>
#include <algorithm>
#include <string>
#include <vector>
#include "PDocLoader.h"
#include "Move.h"
#include "PCommand.h"
#include "PCommandManager.h"
#include "PDocument.h"
#include "ProjectConceptorDefs.h"
#include "RemoveAttribute.h"
#include "Repeat.h"
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
TEST_PLUGIN(TestFindPlugin,Find,"Find")
TEST_PLUGIN(TestRepeatPlugin,Repeat,"Repeat")
TEST_PLUGIN(TestAskPlugin,Ask,"Ask")
TEST_PLUGIN(TestBatchPlugin,Batch,"Batch")
TEST_PLUGIN(TestForEachPlugin,ForEach,"ForEach")
TEST_PLUGIN(TestIfPlugin,If,"If")
TEST_PLUGIN(TestLayoutPlugin,Layout,"Layout")
TEST_PLUGIN(TestRememberPlugin,Remember,"Remember")
TEST_PLUGIN(TestSleepPlugin,Sleep,"Sleep")
TEST_PLUGIN(TestCalculatePlugin,Calculate,"Calculate")
TEST_PLUGIN(TestGetValuePlugin,GetValue,"GetValue")
TEST_PLUGIN(TestSelectConnectedPlugin,SelectConnected,"SelectConnected")
TEST_PLUGIN(TestAddAttributePlugin,AddAttribute,"AddAttribute")
TEST_PLUGIN(TestRemoveAttributePlugin,RemoveAttribute,"RemoveAttribute")

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
	doc->GetCommandManager()->RegisterPCommand(new TestFindPlugin());
	doc->GetCommandManager()->RegisterPCommand(new TestRepeatPlugin());
	doc->GetCommandManager()->RegisterPCommand(new TestAskPlugin());
	doc->GetCommandManager()->RegisterPCommand(new TestBatchPlugin());
	doc->GetCommandManager()->RegisterPCommand(new TestForEachPlugin());
	doc->GetCommandManager()->RegisterPCommand(new TestIfPlugin());
	doc->GetCommandManager()->RegisterPCommand(new TestLayoutPlugin());
	doc->GetCommandManager()->RegisterPCommand(new TestRememberPlugin());
	doc->GetCommandManager()->RegisterPCommand(new TestSleepPlugin());
	doc->GetCommandManager()->RegisterPCommand(new TestCalculatePlugin());
	doc->GetCommandManager()->RegisterPCommand(new TestGetValuePlugin());
	doc->GetCommandManager()->RegisterPCommand(new TestSelectConnectedPlugin());
	doc->GetCommandManager()->RegisterPCommand(new TestAddAttributePlugin());
	doc->GetCommandManager()->RegisterPCommand(new TestRemoveAttributePlugin());
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
	// every BMessage entry is its own line - the command name line, then
	// its "node" field indented on the following line
	CPPUNIT_ASSERT(text.FindFirst("Insert\n") == 0);
	CPPUNIT_ASSERT(text.FindFirst("  node=@7") >= 0);

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


void MacroTextTest::EachFieldRendersOnItsOwnLine(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();

	BMessage	select;
	select.AddString("Command::Name","Select");
	select.AddInt32("node",3);
	select.AddRect("frame",BRect(0,0,10,10));
	select.AddBool("deselect",true);

	BList	commands;
	commands.AddItem(&select);
	BString	text;
	SerializeCommands(&commands,&text);

	// the whole point of #55's readability fix: "Select" carries only the
	// command name, none of its own field values packed onto that same
	// line - every field is its own indented line below it
	CPPUNIT_ASSERT(text.FindFirst("Select\n") == 0);
	CPPUNIT_ASSERT(text.FindFirst("Select ") < 0);
	CPPUNIT_ASSERT(text.FindFirst("  node=@3\n") >= 0);
	CPPUNIT_ASSERT(text.FindFirst("  deselect=true\n") >= 0);
	CPPUNIT_ASSERT(text.FindFirst("  frame=[") >= 0);
}


void MacroTextTest::MultipleTokensOnOneLineIsRejected(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();

	// the old inline "Move dx=1.0" shape is no longer valid syntax - every
	// BMessage entry needs its own line now
	BList		parsed;
	BString		error;
	status_t	err	= ParseCommands(BString("Move dx=1.0\n"),&parsed,
		doc->GetCommandManager(),&error);
	CPPUNIT_ASSERT_EQUAL((status_t)B_BAD_VALUE,err);
	CPPUNIT_ASSERT_EQUAL((int32)0,parsed.CountItems());
}


void MacroTextTest::UnknownCommandNameIsRejected(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();

	BList		parsed;
	BString		error;
	status_t	err	= ParseCommands(BString("Frobnicate\n"),&parsed,
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
	status_t	err	= ParseCommands(BString("Move\n  bogus=1\n"),&parsed,
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
	status_t	err	= ParseCommands(BString("Move\n  dx=\"nope\"\n  dy=1.0\n"),&parsed,
		doc->GetCommandManager(),&error);
	CPPUNIT_ASSERT_EQUAL((status_t)B_BAD_VALUE,err);
	CPPUNIT_ASSERT_EQUAL((int32)0,parsed.CountItems());
}


void MacroTextTest::RoundTripsBoundField(void)
{
	// #135: "$variableName" marks a field as bound instead of literal -
	// recorded as a "PCommand::bindings" entry, never a literal value on
	// the field itself (PCommandManager::ResolveBindings() fills that in
	// fresh at replay time), and rendered back out the same way it was
	// written.
	PDocument	*doc	= NewRegisteredTestDocument();

	BList		parsed;
	BString		error;
	status_t	err	= ParseCommands(BString("Move\n  dx=$offset\n  dy=3.0\n"),
		&parsed,doc->GetCommandManager(),&error);
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,err);
	CPPUNIT_ASSERT_EQUAL((int32)1,parsed.CountItems());

	BMessage	*result	= (BMessage*)parsed.ItemAt(0);
	float	dx	= -1;
	CPPUNIT_ASSERT(result->FindFloat("dx",&dx) != B_OK);
	float	dy	= 0;
	CPPUNIT_ASSERT_EQUAL(B_OK,result->FindFloat("dy",&dy));
	CPPUNIT_ASSERT((dy > 2.99f) && (dy < 3.01f));

	BMessage	bindings;
	CPPUNIT_ASSERT_EQUAL(B_OK,result->FindMessage("PCommand::bindings",&bindings));
	const char	*variableName	= NULL;
	CPPUNIT_ASSERT_EQUAL(B_OK,bindings.FindString("dx",&variableName));
	CPPUNIT_ASSERT(BString(variableName) == "offset");

	BString	text;
	SerializeCommands(&parsed,&text);
	CPPUNIT_ASSERT(text.FindFirst("dx=$offset") >= 0);
	CPPUNIT_ASSERT(text.FindFirst("dx=\"$offset\"") < 0);
}


void MacroTextTest::BindingInsideFieldBlockIsRejected(void)
{
	// ResolveBindings() only ever reads a command's own top-level
	// "PCommand::bindings" - one written inside a "~fieldName" block would
	// silently never resolve at replay time, so the parser rejects it
	// outright instead (see ParseCommands()'s own comment on this).
	PDocument	*doc	= NewRegisteredTestDocument();

	BList		parsed;
	BString		error;
	status_t	err	= ParseCommands(
		BString("ChangeValue\n  ~valueContainer\n    name=$fieldName\n"),
		&parsed,doc->GetCommandManager(),&error);
	CPPUNIT_ASSERT_EQUAL((status_t)B_BAD_VALUE,err);
	CPPUNIT_ASSERT_EQUAL((int32)0,parsed.CountItems());
	CPPUNIT_ASSERT(error.FindFirst("field block") >= 0);
}


void MacroTextTest::EmptyVariableNameIsRejected(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();

	BList		parsed;
	BString		error;
	status_t	err	= ParseCommands(BString("Move\n  dx=$\n  dy=1.0\n"),
		&parsed,doc->GetCommandManager(),&error);
	CPPUNIT_ASSERT_EQUAL((status_t)B_BAD_VALUE,err);
	CPPUNIT_ASSERT_EQUAL((int32)0,parsed.CountItems());
}


void MacroTextTest::PropertyInfoAcceptsNodeSelectedForSelectionDrivenCommands(void)
{
	// #132 normalizes recording of ChangeValue/AddAttribute/RemoveAttribute
	// to "Node::selected=true" when it matches the current selection - the
	// DSL parser rejected that as an unknown field on all three until now
	// (user report): ChangeValue's own PropertyInfo() simply never listed
	// it, and AddAttribute/RemoveAttribute had no PropertyInfo() override
	// at all, so every field on them was "unknown".
	PDocument	*doc	= NewRegisteredTestDocument();
	BString		error;

	BList	changeValueParsed;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,ParseCommands(
		BString("ChangeValue\n  Node::selected=true\n"),
		&changeValueParsed,doc->GetCommandManager(),&error));
	CPPUNIT_ASSERT_EQUAL((int32)1,changeValueParsed.CountItems());

	BList	addAttributeParsed;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,ParseCommands(
		BString("AddAttribute\n  Node::selected=true\n"),
		&addAttributeParsed,doc->GetCommandManager(),&error));
	CPPUNIT_ASSERT_EQUAL((int32)1,addAttributeParsed.CountItems());

	BList	removeAttributeParsed;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,ParseCommands(
		BString("RemoveAttribute\n  Node::selected=true\n"),
		&removeAttributeParsed,doc->GetCommandManager(),&error));
	CPPUNIT_ASSERT_EQUAL((int32)1,removeAttributeParsed.CountItems());
}


void MacroTextTest::GeneratedAddAttributeSnippetParses(void)
{
	// pins the exact shape MacroEditor.cpp's BuildCommandSnippet() (#55
	// follow-up: drag a command from the reference list into the text
	// view with its fields pre-filled, user report - couldn't make sense
	// of the syntax by hand at all) generates for AddAttribute - the one
	// command whose real fields ("~valueContainer" needing "name"/"type"/
	// "newAttribute" with no declared schema of their own, so nothing
	// else validates this shape) are the hardest to get right by hand in
	// the first place. Also exercises "#" comment lines nested *inside* a
	// "~" field block (the type_code cheat sheet the generator adds right
	// there) - ParseCommands() skips a comment "no matter the depth" by
	// its own comment, but nothing else here happened to already combine
	// the two.
	PDocument	*doc	= NewRegisteredTestDocument();

	BString	snippet(
		"AddAttribute\n"
		"  # use ONE of node/Node::selected below, not both\n"
		"  node=@1\n"
		"  Node::selected=false\n"
		"  ~valueContainer\n"
		"    name=\"\"\n"
		"    # type: exact type_code as a decimal int32 - common ones: "
		"bool=1112493900 int32=1280265799 float=1179406164 "
		"double=1145195589 string=1129534546\n"
		"    type=1129534546\n"
		"    newAttribute=\"\"\n"
		"    # subgroup (optional, repeatable): nests into a sub-BMessage "
		"first, e.g. subgroup=\"Node::Data\"\n"
		"  ~included_node\n");

	BList		parsed;
	BString		error;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,
		ParseCommands(snippet,&parsed,doc->GetCommandManager(),&error));
	CPPUNIT_ASSERT_EQUAL((int32)1,parsed.CountItems());
}


void MacroTextTest::FindThenAddAttributeReachesEveryFoundNode(void)
{
	// user report: a hand-written Repeat{Find "Test"; AddAttribute
	// Node::selected=true} macro "only added one attribute" to three
	// nodes named Test. Same text, same three nodes, played back through
	// the real parse + PlayMacro() path - counts how many "Attribute"
	// entries each node's Node::Data ends up with.
	PDocument	*doc	= NewRegisteredTestDocument();

	BMessage	*nodes[3];
	for (int32 i = 0; i < 3; i++) {
		nodes[i]	= new BMessage(P_C_CLASS_TYPE);
		BString	name;
		name << "Test " << (i+1);
		nodes[i]->AddString("Node::name",name.String());
		nodes[i]->AddBool(P_C_NODE_SELECTED,false);
		BMessage	data;
		nodes[i]->AddMessage(P_C_NODE_DATA,&data);
		doc->GetAllNodes()->AddItem(nodes[i]);
	}

	BString	text(
		"Repeat\n"
		"  count=1\n"
		"  Find\n"
		"    searchString=\"Test\"\n"
		"  AddAttribute\n"
		"    Node::selected=true\n"
		"    ~valueContainer\n"
		"      type=1297303367\n"
		"      name=\"Attribute\"\n"
		"      subgroup=\"Node::Data\"\n"
		"      ~newAttribute\n"
		"        Name=\"Attribute\"\n"
		"        Value=true\n");
	BList		parsed;
	BString		error;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,
		ParseCommands(text,&parsed,doc->GetCommandManager(),&error));

	BMessage	macro(P_C_MACRO_TYPE);
	for (int32 i = 0; i < parsed.CountItems(); i++)
		macro.AddMessage("Macro::Commmand",(BMessage*)parsed.ItemAt(i));
	doc->GetCommandManager()->PlayMacro(&macro);

	for (int32 i = 0; i < 3; i++) {
		BMessage	data;
		CPPUNIT_ASSERT(nodes[i]->FindMessage(P_C_NODE_DATA,&data) == B_OK);
		type_code	type;
		int32		count	= 0;
		CPPUNIT_ASSERT(data.GetInfo("Attribute",&type,&count) == B_OK);
		CPPUNIT_ASSERT_EQUAL((int32)1,count);
	}
}


void MacroTextTest::EveryCommandExampleParses(void)
{
	// the tooltip examples (CommandExampleText()) are hand-written text -
	// this keeps each one honest against the real parser and the real
	// PropertyInfo() schemas, so a field rename or type change that breaks
	// an example fails here instead of quietly showing wrong syntax.
	PDocument		*doc		= NewRegisteredTestDocument();
	PCommandManager	*manager	= doc->GetCommandManager();
	int32			checked		= 0;
	for (int32 i = 0; i < manager->CountPCommand(); i++) {
		PCommand	*command	= manager->PCommandAt(i);
		const char	*example	= CommandExampleText(command->Name());
		if (example == NULL)
			continue;
		BList		parsed;
		BString		error;
		BString		text(example);
		text << "\n";
		status_t	err	= ParseCommands(text,&parsed,manager,&error);
		BString		message;
		message << command->Name() << " example: " << error;
		CPPUNIT_ASSERT_MESSAGE(message.String(),err == B_OK);
		checked++;
	}
	CPPUNIT_ASSERT(checked >= 12);
}


void MacroTextTest::IncludedNodeKeepsItsMessageType(void)
{
	// an embedded node's BMessage "what" (P_C_CLASS_TYPE / P_C_GROUP_TYPE /
	// P_C_CONNECTION_TYPE) decides which renderer GraphEditor creates and
	// whether Indexer treats it as a connection - it has to survive the
	// text round trip, not come back as 0.
	PDocument	*doc	= NewRegisteredTestDocument();
	uint32		kinds[]	= { P_C_CLASS_TYPE, P_C_GROUP_TYPE, P_C_CONNECTION_TYPE };
	for (int32 k = 0; k < 3; k++) {
		BMessage	node(kinds[k]);
		node.AddInt32("this",1);
		BMessage	insert;
		insert.AddString("Command::Name","Insert");
		insert.AddInt32("node",1);
		insert.AddMessage("included_node",&node);
		BList	commands;
		commands.AddItem(&insert);
		BString	text;
		SerializeCommands(&commands,&text);

		BList	parsed;
		BString	error;
		CPPUNIT_ASSERT_EQUAL((status_t)B_OK,
			ParseCommands(text,&parsed,doc->GetCommandManager(),&error));
		BMessage	back;
		CPPUNIT_ASSERT(((BMessage*)parsed.ItemAt(0))->FindMessage("included_node",&back) == B_OK);
		CPPUNIT_ASSERT_EQUAL(kinds[k],(uint32)back.what);
	}
}


void MacroTextTest::TypeCodesReadAsNames(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();
	BMessage	container;
	container.AddString("name","Priority");
	container.AddInt32("type",(int32)B_STRING_TYPE);
	container.AddString("newAttribute","x");
	BMessage	add;
	add.AddString("Command::Name","AddAttribute");
	add.AddBool(P_C_NODE_SELECTED,true);
	add.AddMessage("valueContainer",&container);
	BList	commands;
	commands.AddItem(&add);
	BString	text;
	SerializeCommands(&commands,&text);
	CPPUNIT_ASSERT(text.FindFirst("type=string") >= 0);
	CPPUNIT_ASSERT(text.FindFirst("1129534546") < 0);

	BList	parsed;
	BString	error;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,
		ParseCommands(text,&parsed,doc->GetCommandManager(),&error));
	BMessage	back;
	CPPUNIT_ASSERT(((BMessage*)parsed.ItemAt(0))->FindMessage("valueContainer",&back) == B_OK);
	int32		type	= 0;
	CPPUNIT_ASSERT(back.FindInt32("type",&type) == B_OK);
	CPPUNIT_ASSERT_EQUAL((int32)B_STRING_TYPE,type);

	// a plain number (and a code with no name) keeps working
	BList	numeric;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,ParseCommands(
		BString("AddAttribute\n  ~valueContainer\n    type=1129534546\n"),
		&numeric,doc->GetCommandManager(),&error));
}


void MacroTextTest::InsertPrototypeParsesAndKeepsNodeShape(void)
{
	BMessage	*insert	= BuildInsertPrototype();
	BMessage	node;
	CPPUNIT_ASSERT(insert->FindMessage("included_node",&node) == B_OK);
	CPPUNIT_ASSERT_EQUAL((uint32)P_C_CLASS_TYPE,(uint32)node.what);
	BRect		frame;
	CPPUNIT_ASSERT(node.FindRect(P_C_NODE_FRAME,&frame) == B_OK);
	BMessage	data;
	CPPUNIT_ASSERT(node.FindMessage(P_C_NODE_DATA,&data) == B_OK);
	CPPUNIT_ASSERT(node.HasMessage(P_C_NODE_PATTERN));
	int32		nodeField	= 0;
	CPPUNIT_ASSERT(insert->FindInt32("node",&nodeField) == B_OK);
	CPPUNIT_ASSERT_EQUAL((int32)1,nodeField);
	delete insert;
}


void MacroTextTest::DroppedPrototypesGetDistinctIds(void)
{
	BMessage	*first	= BuildInsertPrototype();
	BList		one;
	one.AddItem(first);
	CPPUNIT_ASSERT_EQUAL((int32)1,HighestReferencedId(&one));

	BMessage	*second	= BuildInsertPrototype();
	AssignInsertId(second,2);
	int32	secondNode	= 0;
	second->FindInt32("node",&secondNode);
	CPPUNIT_ASSERT_EQUAL((int32)2,secondNode);
	BMessage	secondEmbedded;
	second->FindMessage("included_node",&secondEmbedded);
	int32	secondThis	= 0;
	secondEmbedded.FindInt32("this",&secondThis);
	CPPUNIT_ASSERT_EQUAL((int32)2,secondThis);

	BList	both;
	both.AddItem(first);
	both.AddItem(second);
	CPPUNIT_ASSERT_EQUAL((int32)2,HighestReferencedId(&both));

	BList	empty;
	CPPUNIT_ASSERT_EQUAL((int32)-1,HighestReferencedId(&empty));
}


void MacroTextTest::RepeatedInsertCreatesDistinctNodes(void)
{
	// user report: "create six nodes" macro - Repeat around two Inserts and
	// a Batch of Select/Move played back and did nothing
	PDocument	*doc	= NewRegisteredTestDocument();

	BMessage	*insertA	= BuildInsertPrototype();
	BMessage	*insertB	= BuildInsertPrototype();
	AssignInsertId(insertB,2);

	BMessage	select;
	select.AddString("Command::Name","Select");
	select.AddInt32("node",1);
	select.AddBool("deselect",false);
	select.AddBool("selectAll",false);

	BMessage	move;
	move.AddString("Command::Name","Move");
	move.AddFloat("dx",100.0f);
	move.AddFloat("dy",100.0f);

	BMessage	batch;
	batch.AddString("Command::Name","Batch");
	batch.AddMessage("PCommand::subPCommand",&select);
	batch.AddMessage("PCommand::subPCommand",&move);

	BMessage	repeat;
	repeat.AddString("Command::Name","Repeat");
	repeat.AddInt32("count",3);
	repeat.AddMessage("PCommand::subPCommand",insertA);
	repeat.AddMessage("PCommand::subPCommand",insertB);
	repeat.AddMessage("PCommand::subPCommand",&batch);
	delete insertA;
	delete insertB;

	BMessage	macro(P_C_MACRO_TYPE);
	macro.AddMessage("Macro::Commmand",&repeat);
	doc->GetCommandManager()->PlayMacro(&macro);

	BList	*nodes	= doc->GetAllNodes();
	CPPUNIT_ASSERT_EQUAL((int32)6,nodes->CountItems());
	for (int32 a = 0; a < nodes->CountItems(); a++)
		for (int32 b = a+1; b < nodes->CountItems(); b++)
			CPPUNIT_ASSERT(nodes->ItemAt(a) != nodes->ItemAt(b));
}


static std::string SortedLines(const BString &text)
{
	std::vector<std::string>	lines;
	int32	at	= 0;
	while (at < text.Length()) {
		int32	nl	= text.FindFirst("\n",at);
		if (nl < 0)
			nl	= text.Length();
		lines.push_back(std::string(text.String()+at,nl-at));
		at	= nl+1;
	}
	std::sort(lines.begin(),lines.end());
	std::string	joined;
	for (size_t i = 0; i < lines.size(); i++)
		joined	+= lines[i] + "\n";
	return joined;
}


void MacroTextTest::MacroSurvivesDocumentSaveAndLoad(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();
	BMessage	*insert	= BuildInsertPrototype();
	BMessage	move;
	move.AddString("Command::Name","Move");
	move.AddFloat("dx",10.0f);
	move.AddFloat("dy",5.0f);

	BMessage	*macro	= new BMessage(P_C_MACRO_TYPE);
	macro->AddString("Name","kept");
	macro->AddMessage("Macro::Commmand",insert);
	macro->AddMessage("Macro::Commmand",&move);
	delete insert;
	doc->GetCommandManager()->GetMacroList()->AddItem(macro);

	BList	original;
	BMessage	entry;
	for (int32 i = 0; macro->FindMessage("Macro::Commmand",i,&entry) == B_OK; i++) {
		original.AddItem(new BMessage(entry));
		entry.MakeEmpty();
	}
	BString	text;
	SerializeCommands(&original,&text);
	for (int32 i = 0; i < original.CountItems(); i++)
		delete (BMessage*)original.ItemAt(i);

	// what Save()/Load() do around the file: archive, flatten, unflatten
	BMessage	archive;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,doc->Archive(&archive,true));
	BMallocIO	buffer;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,archive.Flatten(&buffer));
	buffer.Seek(0,SEEK_SET);
	BMessage	loaded;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,loaded.Unflatten(&buffer));

	PDocLoader	loader(doc,&loaded);
	BList		*macros	= loader.GetMacroList();
	CPPUNIT_ASSERT_EQUAL((int32)1,macros->CountItems());
	BMessage	*again	= (BMessage*)macros->ItemAt(0);
	const char	*name	= NULL;
	CPPUNIT_ASSERT(again->FindString("Name",&name) == B_OK);
	CPPUNIT_ASSERT(BString("kept") == name);

	BList		commands;
	BMessage	reloadedEntry;
	for (int32 i = 0; again->FindMessage("Macro::Commmand",i,&reloadedEntry) == B_OK; i++) {
		commands.AddItem(new BMessage(reloadedEntry));
		reloadedEntry.MakeEmpty();
	}
	BString	roundTripped;
	SerializeCommands(&commands,&roundTripped);
	// field order inside a message is not kept by flatten/unflatten
	// (Node::Frame moves) - same lines, same values is what matters
	CPPUNIT_ASSERT_EQUAL(SortedLines(text),SortedLines(roundTripped));
}


void MacroTextTest::PlayMacroReportsWhatHappened(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();
	BString		report;

	// nothing to play
	BMessage	empty(P_C_MACRO_TYPE);
	CPPUNIT_ASSERT(doc->GetCommandManager()->PlayMacro(&empty,&report) != B_OK);
	CPPUNIT_ASSERT(report.FindFirst("empty") >= 0);

	// a Select naming a node the macro never creates
	BString		text("Select\n  node=@9\n  deselect=false\n  selectAll=false\n");
	BList		parsed;
	BString		error;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,
		ParseCommands(text,&parsed,doc->GetCommandManager(),&error));
	BMessage	dangling(P_C_MACRO_TYPE);
	for (int32 i = 0; i < parsed.CountItems(); i++)
		dangling.AddMessage("Macro::Commmand",(BMessage*)parsed.ItemAt(i));
	CPPUNIT_ASSERT(doc->GetCommandManager()->PlayMacro(&dangling,&report) != B_OK);
	CPPUNIT_ASSERT(report.FindFirst("never creates") >= 0);

	// the real thing
	BMessage	*insert	= BuildInsertPrototype();
	BMessage	good(P_C_MACRO_TYPE);
	good.AddMessage("Macro::Commmand",insert);
	delete insert;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,doc->GetCommandManager()->PlayMacro(&good,&report));
	CPPUNIT_ASSERT(report.FindFirst("Played 1") >= 0);
}


void MacroTextTest::FormatAndParseFieldValueRoundTripEveryType(void)
{
	BMessage	msg;
	msg.AddBool("b",true);
	msg.AddInt8("i8",-5);
	msg.AddInt16("i16",-300);
	msg.AddInt32("i32",70000);
	msg.AddInt64("i64",5000000000LL);
	msg.AddFloat("f",1.5f);
	msg.AddDouble("d",2.5);
	msg.AddString("s","hi \"there\"");
	msg.AddPoint("pt",BPoint(3,4));
	msg.AddRect("r",BRect(1,2,3,4));
	msg.AddInt32("node",7);	// the one field name FormatFieldValue() special-cases

	static const struct { const char *field; type_code type; } kFields[] = {
		{ "b", B_BOOL_TYPE }, { "i8", B_INT8_TYPE }, { "i16", B_INT16_TYPE },
		{ "i32", B_INT32_TYPE }, { "i64", B_INT64_TYPE }, { "f", B_FLOAT_TYPE },
		{ "d", B_DOUBLE_TYPE }, { "s", B_STRING_TYPE }, { "pt", B_POINT_TYPE },
		{ "r", B_RECT_TYPE },
	};
	for (size_t i = 0; i < sizeof(kFields)/sizeof(kFields[0]); i++) {
		BString	text;
		FormatFieldValue(&msg,kFields[i].field,kFields[i].type,0,&text);
		BMessage	roundTripped;
		BString		error;
		CPPUNIT_ASSERT_EQUAL_MESSAGE(kFields[i].field,(status_t)B_OK,
			ParseFieldValue(&roundTripped,kFields[i].field,kFields[i].type,text,false,&error));
		BString	again;
		FormatFieldValue(&roundTripped,kFields[i].field,kFields[i].type,0,&again);
		CPPUNIT_ASSERT_EQUAL_MESSAGE(kFields[i].field,text,again);
	}

	// "node" - schema type is B_POINTER_TYPE, stored type is int32 (see
	// MacroText.h's FormatFieldValue doc comment)
	BString	nodeText;
	FormatFieldValue(&msg,"node",B_INT32_TYPE,0,&nodeText);
	CPPUNIT_ASSERT(nodeText == "@7");
	BMessage	nodeMsg;
	BString		error;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,ParseFieldValue(&nodeMsg,"node",B_POINTER_TYPE,nodeText,false,&error));
	int32	roundTrippedNode	= 0;
	CPPUNIT_ASSERT(nodeMsg.FindInt32("node",&roundTrippedNode) == B_OK);
	CPPUNIT_ASSERT_EQUAL((int32)7,roundTrippedNode);

	// $binding - allowed at command level, rejected inside a field block
	BMessage	bound;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,
		ParseFieldValue(&bound,"dx",B_FLOAT_TYPE,BString("$i"),true,&error));
	BMessage	bindings;
	CPPUNIT_ASSERT(bound.FindMessage("PCommand::bindings",&bindings) == B_OK);
	const char	*variable	= NULL;
	CPPUNIT_ASSERT(bindings.FindString("dx",&variable) == B_OK);
	CPPUNIT_ASSERT(BString("i") == variable);
	CPPUNIT_ASSERT_EQUAL((status_t)B_BAD_VALUE,
		ParseFieldValue(&bound,"dx",B_FLOAT_TYPE,BString("$i"),false,&error));
}


void MacroTextTest::CalculateWritesResultIntoValueContext(void)
{
	// binary op via literal operands
	PDocument	*doc	= NewRegisteredTestDocument();
	BMessage	sum;
	sum.AddString("Command::Name","Calculate");
	sum.AddFloat("left",10.0f);
	sum.AddString("operator","+");
	sum.AddFloat("right",5.0f);
	sum.AddString("resultVariable","sum");

	BMessage	macro(P_C_MACRO_TYPE);
	macro.AddMessage("Macro::Commmand",&sum);
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,doc->GetCommandManager()->PlayMacro(&macro));

	// the value context is scoped to PlayMacro() itself and gone once it
	// returns - chain a second Calculate in the SAME macro that reads the
	// first one's result via "$sum" (PCommandManager::ResolveBindings())
	// to actually observe it, exactly like Repeat's own dx=$i test does
	BMessage	rounded;
	rounded.AddString("Command::Name","Calculate");
	BMessage	bindings;
	bindings.AddString("left","sum");
	rounded.AddMessage("PCommand::bindings",&bindings);
	rounded.AddString("operator","round");
	rounded.AddString("resultVariable","roundedSum");

	BMessage	chained(P_C_MACRO_TYPE);
	BMessage	sumAgain(sum);
	chained.AddMessage("Macro::Commmand",&sumAgain);
	chained.AddMessage("Macro::Commmand",&rounded);
	BString	report;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,doc->GetCommandManager()->PlayMacro(&chained,&report));
	CPPUNIT_ASSERT(report.FindFirst("Played 2") >= 0);
}


void MacroTextTest::CalculateSupportsEveryOperator(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();
	static const struct { const char *op; float left; float right; } kOps[] = {
		{ "+", 2.0f, 3.0f }, { "-", 5.0f, 2.0f }, { "*", 4.0f, 2.5f },
		{ "/", 9.0f, 3.0f }, { "mod", 7.0f, 3.0f }, { "min", 2.0f, 9.0f },
		{ "max", 2.0f, 9.0f }, { "round", 2.4f, 0.0f }, { "floor", 2.9f, 0.0f },
		{ "ceil", 2.1f, 0.0f }, { "abs", -3.0f, 0.0f },
	};
	for (size_t i = 0; i < sizeof(kOps)/sizeof(kOps[0]); i++) {
		BMessage	calc;
		calc.AddString("Command::Name","Calculate");
		calc.AddFloat("left",kOps[i].left);
		calc.AddString("operator",kOps[i].op);
		calc.AddFloat("right",kOps[i].right);
		calc.AddString("resultVariable","r");
		BMessage	macro(P_C_MACRO_TYPE);
		macro.AddMessage("Macro::Commmand",&calc);
		BString	error;
		CPPUNIT_ASSERT_EQUAL_MESSAGE(kOps[i].op,(status_t)B_OK,
			doc->GetCommandManager()->PlayMacro(&macro,&error));
	}

	// unrecognized operator: no crash, just no result written (no silent
	// fallback - see Calculate::Do())
	BMessage	bad;
	bad.AddString("Command::Name","Calculate");
	bad.AddFloat("left",1.0f);
	bad.AddString("operator","???");
	bad.AddString("resultVariable","r");
	BMessage	macro(P_C_MACRO_TYPE);
	macro.AddMessage("Macro::Commmand",&bad);
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,doc->GetCommandManager()->PlayMacro(&macro));
}


void MacroTextTest::CalculateExampleParses(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();
	BList		parsed;
	BString		error;
	BString		text(CommandExampleText("Calculate"));
	text << "\n";
	CPPUNIT_ASSERT_EQUAL_MESSAGE(error.String(),(status_t)B_OK,
		ParseCommands(text,&parsed,doc->GetCommandManager(),&error));
}


// MacroOutlineView only calls back into these three - real MacroEditor is
// not part of the test binary
void MacroEditor::CommitOutlineChange(void) {}
void MacroEditor::ShowOutlineError(const char*) {}
PCommandManager* MacroEditor::CommandManagerForOutline(void) {return NULL;}


static BMessage* FindTopLevelCommand(BList *commands, const char *name, int32 occurrence = 0)
{
	int32	seen	= 0;
	for (int32 i = 0; i < commands->CountItems(); i++) {
		BMessage	*cmd	= (BMessage*)commands->ItemAt(i);
		const char	*cmdName	= NULL;
		cmd->FindString("Command::Name",&cmdName);
		if ((cmdName != NULL) && (strcmp(cmdName,name) == 0)) {
			if (seen == occurrence)
				return cmd;
			seen++;
		}
	}
	return NULL;
}


void MacroTextTest::MoveCommandReparentsIntoAnotherContainer(void)
{
	// Repeat[ Move(dx=1) ], Batch[ Move(dx=2) ] - drag Repeat's Move into Batch
	BMessage	moveA;
	moveA.AddString("Command::Name","Move");
	moveA.AddFloat("dx",1.0f);
	BMessage	repeat;
	repeat.AddString("Command::Name","Repeat");
	repeat.AddInt32("count",1);
	repeat.AddMessage("PCommand::subPCommand",&moveA);

	BMessage	moveB;
	moveB.AddString("Command::Name","Move");
	moveB.AddFloat("dx",2.0f);
	BMessage	batch;
	batch.AddString("Command::Name","Batch");
	batch.AddMessage("PCommand::subPCommand",&moveB);

	BList	commands;
	commands.AddItem(&repeat);
	commands.AddItem(&batch);

	MacroOutlineView	view(BRect(0,0,300,300),"t",B_FOLLOW_ALL_SIDES);
	view.SetCommands(&commands);

	// Repeat's Move (topLevel 0, path [subPCommand#0]) dropped onto Batch
	// (topLevel 1, self path empty) - lands as Batch's LAST subcommand
	MacroPath	sourcePath;
	MacroPathStep	step; step.field = "PCommand::subPCommand"; step.index = 0;
	sourcePath.push_back(step);
	// find Batch's own row index
	MacroPath	emptySelf;
	int32	batchRow	= view.RowIndexForCommand(1,emptySelf);
	CPPUNIT_ASSERT(batchRow >= 0);
	view.MoveCommandRow(0,sourcePath,batchRow,BPoint(0,0));

	BList		*result	= view.Commands();
	CPPUNIT_ASSERT_EQUAL((int32)2,result->CountItems());
	BMessage	*newRepeat	= FindTopLevelCommand(result,"Repeat");
	BMessage	*newBatch	= FindTopLevelCommand(result,"Batch");
	CPPUNIT_ASSERT(newRepeat != NULL && newBatch != NULL);
	CPPUNIT_ASSERT(!newRepeat->HasMessage("PCommand::subPCommand"));

	BMessage	child;
	int32	subCount	= 0;
	for (int32 i = 0; newBatch->FindMessage("PCommand::subPCommand",i,&child) == B_OK; i++)
		subCount++;
	CPPUNIT_ASSERT_EQUAL((int32)2,subCount);
	BMessage	last;
	newBatch->FindMessage("PCommand::subPCommand",1,&last);
	float	lastDx	= 0.0f;
	last.FindFloat("dx",&lastDx);
	CPPUNIT_ASSERT_EQUAL(1.0f,lastDx);	// the moved one landed last
}


void MacroTextTest::MoveCommandPromotesToTopLevel(void)
{
	BMessage	move;
	move.AddString("Command::Name","Move");
	move.AddFloat("dx",5.0f);
	BMessage	repeat;
	repeat.AddString("Command::Name","Repeat");
	repeat.AddInt32("count",1);
	repeat.AddMessage("PCommand::subPCommand",&move);

	BList	commands;
	commands.AddItem(&repeat);

	MacroOutlineView	view(BRect(0,0,300,300),"t",B_FOLLOW_ALL_SIDES);
	view.SetCommands(&commands);

	MacroPath	sourcePath;
	MacroPathStep	step; step.field = "PCommand::subPCommand"; step.index = 0;
	sourcePath.push_back(step);
	// drop below everything -> promoted to the macro's own top level
	view.MoveCommandRow(0,sourcePath,-1,BPoint(0,10000));

	BList	*result	= view.Commands();
	CPPUNIT_ASSERT_EQUAL((int32)2,result->CountItems());
	BMessage	*promoted	= FindTopLevelCommand(result,"Move");
	CPPUNIT_ASSERT(promoted != NULL);
	BMessage	*newRepeat	= FindTopLevelCommand(result,"Repeat");
	CPPUNIT_ASSERT(newRepeat != NULL);
	CPPUNIT_ASSERT(!newRepeat->HasMessage("PCommand::subPCommand"));
}


void MacroTextTest::MoveCommandRefusesDroppingIntoOwnSubtree(void)
{
	BMessage	move;
	move.AddString("Command::Name","Move");
	move.AddFloat("dx",5.0f);
	BMessage	batch;
	batch.AddString("Command::Name","Batch");
	batch.AddMessage("PCommand::subPCommand",&move);

	BList	commands;
	commands.AddItem(&batch);

	MacroOutlineView	view(BRect(0,0,300,300),"t",B_FOLLOW_ALL_SIDES);
	view.SetCommands(&commands);

	// drop Batch (top-level, empty path) onto its own child Move
	MacroPath	moveSelf;
	MacroPathStep	moveStep; moveStep.field = "PCommand::subPCommand"; moveStep.index = 0;
	moveSelf.push_back(moveStep);
	int32	moveRow	= view.RowIndexForCommand(0,moveSelf);
	CPPUNIT_ASSERT(moveRow >= 0);
	MacroPath	emptyPath;
	view.MoveCommandRow(0,emptyPath,moveRow,BPoint(0,0));

	// nothing changed - still exactly one top-level Batch with its Move inside
	BList	*result	= view.Commands();
	CPPUNIT_ASSERT_EQUAL((int32)1,result->CountItems());
	BMessage	*stillBatch	= (BMessage*)result->ItemAt(0);
	const char	*name	= NULL;
	stillBatch->FindString("Command::Name",&name);
	CPPUNIT_ASSERT(BString("Batch") == name);
	CPPUNIT_ASSERT(stillBatch->HasMessage("PCommand::subPCommand"));
}


void MacroTextTest::MoveCommandReordersTopLevelSiblings(void)
{
	BMessage	first;
	first.AddString("Command::Name","Move");
	first.AddFloat("dx",1.0f);
	BMessage	second;
	second.AddString("Command::Name","Move");
	second.AddFloat("dx",2.0f);

	BList	commands;
	commands.AddItem(&first);
	commands.AddItem(&second);

	MacroOutlineView	view(BRect(0,0,300,300),"t",B_FOLLOW_ALL_SIDES);
	view.SetCommands(&commands);

	// move the SECOND top-level command to before the first one (its
	// upper half - ItemFrame() has no real height in this headless test,
	// so this is the one half-detection that's independent of that)
	MacroPath	emptyPath;
	MacroPath	firstSelf;
	int32	firstRow	= view.RowIndexForCommand(0,firstSelf);
	CPPUNIT_ASSERT(firstRow >= 0);
	// a row never actually laid out in a real, attached window (this test
	// has no BWindow at all) has no real height - ItemFrame() can come back
	// with a nonsensical/negative one, so a point pulled from it can't be
	// trusted to land in a specific half; a clearly far-off point still
	// reliably resolves to "before" (any real row's own top/bottom are
	// nowhere near this) regardless of that
	view.MoveCommandRow(1,emptyPath,firstRow,BPoint(0,-100000));

	BList	*result	= view.Commands();
	CPPUNIT_ASSERT_EQUAL((int32)2,result->CountItems());
	float	dx0	= 0.0f, dx1 = 0.0f;
	((BMessage*)result->ItemAt(0))->FindFloat("dx",&dx0);
	((BMessage*)result->ItemAt(1))->FindFloat("dx",&dx1);
	CPPUNIT_ASSERT_EQUAL(2.0f,dx0);
	CPPUNIT_ASSERT_EQUAL(1.0f,dx1);
}


void MacroTextTest::InsertResultVariableEnablesFollowUpReference(void)
{
	// Insert resultVariable="n", then Select node=$n right after - both in
	// the SAME macro/PlayMacro() call, since the value context is scoped
	// to exactly that (see PCommandManager::PlayMacro())
	PDocument	*doc	= NewRegisteredTestDocument();
	BMessage	*insert	= BuildInsertPrototype();
	insert->AddString("resultVariable","n");

	BMessage	select;
	select.AddString("Command::Name","Select");
	BMessage	bindings;
	bindings.AddString("node","n");
	select.AddMessage("PCommand::bindings",&bindings);
	select.AddBool("deselect",true);
	select.AddBool("selectAll",false);

	BMessage	macro(P_C_MACRO_TYPE);
	macro.AddMessage("Macro::Commmand",insert);
	macro.AddMessage("Macro::Commmand",&select);
	delete insert;

	BString	report;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(report.String(),(status_t)B_OK,
		doc->GetCommandManager()->PlayMacro(&macro,&report));

	BList	*nodes	= doc->GetAllNodes();
	CPPUNIT_ASSERT_EQUAL((int32)1,nodes->CountItems());
	bool	selected	= false;
	((BMessage*)nodes->ItemAt(0))->FindBool(P_C_NODE_SELECTED,&selected);
	CPPUNIT_ASSERT(selected);
}


void MacroTextTest::CalculatedNodeIdBindsAsInt32NotFloat(void)
{
	// a Calculate result (always float) bound into a "node" field (schema
	// B_POINTER_TYPE, stored int32) must coerce, or the target command's
	// own FindInt32()/FindPointer() could never read it back - without the
	// coercion, ResolveBindings() would add "node" as a float field,
	// Indexer::DeIndexCommand()'s FindInt32("node",...) walk would never
	// even see it (wrong type), and Select would silently select nothing
	// at all (not an error PlayMacro() would report either) - so the only
	// reliable way to tell coerced-and-wrong-id apart from not-coerced-
	// at-all is to check the node actually ended up selected.
	PDocument	*doc	= NewRegisteredTestDocument();
	BMessage	*insert	= BuildInsertPrototype();	// embeds a node with this=1

	BMessage	calc;
	calc.AddString("Command::Name","Calculate");
	calc.AddFloat("left",1.0f);
	calc.AddString("operator","+");
	calc.AddFloat("right",0.0f);
	calc.AddString("resultVariable","computedId");

	BMessage	select;
	select.AddString("Command::Name","Select");
	BMessage	bindings;
	bindings.AddString("node","computedId");
	select.AddMessage("PCommand::bindings",&bindings);
	select.AddBool("deselect",false);
	select.AddBool("selectAll",false);

	BMessage	macro(P_C_MACRO_TYPE);
	macro.AddMessage("Macro::Commmand",insert);
	macro.AddMessage("Macro::Commmand",&calc);
	macro.AddMessage("Macro::Commmand",&select);
	delete insert;

	BString	report;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(report.String(),(status_t)B_OK,
		doc->GetCommandManager()->PlayMacro(&macro,&report));

	BList	*nodes	= doc->GetAllNodes();
	CPPUNIT_ASSERT_EQUAL((int32)1,nodes->CountItems());
	bool	selected	= false;
	((BMessage*)nodes->ItemAt(0))->FindBool(P_C_NODE_SELECTED,&selected);
	CPPUNIT_ASSERT(selected);
}


void MacroTextTest::DeleteSelectedRowsRemovesOnlyChosenSiblings(void)
{
	BMessage	m0; m0.AddString("Command::Name","Move"); m0.AddFloat("dx",1.0f);
	BMessage	m1; m1.AddString("Command::Name","Move"); m1.AddFloat("dx",2.0f);
	BMessage	m2; m2.AddString("Command::Name","Move"); m2.AddFloat("dx",3.0f);
	BMessage	batch;
	batch.AddString("Command::Name","Batch");
	batch.AddMessage("PCommand::subPCommand",&m0);
	batch.AddMessage("PCommand::subPCommand",&m1);
	batch.AddMessage("PCommand::subPCommand",&m2);

	BList	commands;
	commands.AddItem(&batch);

	MacroOutlineView	view(BRect(0,0,300,300),"t",B_FOLLOW_ALL_SIDES);
	view.SetCommands(&commands);

	MacroPath	p0,p2;
	MacroPathStep	s0; s0.field = "PCommand::subPCommand"; s0.index = 0; p0.push_back(s0);
	MacroPathStep	s2; s2.field = "PCommand::subPCommand"; s2.index = 2; p2.push_back(s2);
	int32	row0	= view.RowIndexForCommand(0,p0);
	int32	row2	= view.RowIndexForCommand(0,p2);
	CPPUNIT_ASSERT((row0 >= 0) && (row2 >= 0));
	view.Select(row0,false);
	view.Select(row2,true);

	view.DeleteSelectedRows();

	BList	*result	= view.Commands();
	CPPUNIT_ASSERT_EQUAL((int32)1,result->CountItems());
	BMessage	*newBatch	= (BMessage*)result->ItemAt(0);
	BMessage	child;
	int32	count	= 0;
	float	remainingDx	= 0.0f;
	for (int32 i = 0; newBatch->FindMessage("PCommand::subPCommand",i,&child) == B_OK; i++) {
		count++;
		child.FindFloat("dx",&remainingDx);
	}
	CPPUNIT_ASSERT_EQUAL((int32)1,count);
	CPPUNIT_ASSERT_EQUAL(2.0f,remainingDx);
}


void MacroTextTest::DeleteSelectedRowsSkipsDescendantsOfAnotherSelectedRow(void)
{
	BMessage	move;
	move.AddString("Command::Name","Move");
	move.AddFloat("dx",1.0f);
	BMessage	batch;
	batch.AddString("Command::Name","Batch");
	batch.AddMessage("PCommand::subPCommand",&move);

	BMessage	second;
	second.AddString("Command::Name","Move");
	second.AddFloat("dx",9.0f);

	BList	commands;
	commands.AddItem(&batch);
	commands.AddItem(&second);

	MacroOutlineView	view(BRect(0,0,300,300),"t",B_FOLLOW_ALL_SIDES);
	view.SetCommands(&commands);

	MacroPath	emptyPath, movePath;
	MacroPathStep	step; step.field = "PCommand::subPCommand"; step.index = 0;
	movePath.push_back(step);
	int32	batchRow	= view.RowIndexForCommand(0,emptyPath);
	int32	moveRow		= view.RowIndexForCommand(0,movePath);
	CPPUNIT_ASSERT((batchRow >= 0) && (moveRow >= 0));
	// select the container AND its own child at once - deleting both
	// should not double-free/crash, and should still just remove Batch
	view.Select(batchRow,false);
	view.Select(moveRow,true);

	view.DeleteSelectedRows();

	BList	*result	= view.Commands();
	CPPUNIT_ASSERT_EQUAL((int32)1,result->CountItems());
	const char	*name	= NULL;
	((BMessage*)result->ItemAt(0))->FindString("Command::Name",&name);
	CPPUNIT_ASSERT(BString("Move") == name);
}


void MacroTextTest::AddNamedFieldAddsCustomFieldToGenericBlock(void)
{
	// a generic block (no schema - see MacroText.h) still needs a way to
	// grow a brand new, user-named field (user report, forward-looking:
	// e.g. UML-specific node attributes later) - AddNamedField() is the
	// part of that reachable without a modal dialog (the freeform "+ Feld
	// hinzufügen" flow prompts for the name via InputRequest first, see
	// MacroOutlineView::MessageReceived()'s 'mvFT' case)
	BMessage	container;
	container.AddString("name","Priority");

	BMessage	changeValue;
	changeValue.AddString("Command::Name","ChangeValue");
	changeValue.AddMessage("valueContainer",&container);

	BList	commands;
	commands.AddItem(&changeValue);

	MacroOutlineView	view(BRect(0,0,300,300),"t",B_FOLLOW_ALL_SIDES);
	view.SetCommands(&commands);

	MacroPath	blockPath;
	MacroPathStep	step; step.field = "valueContainer"; step.index = 0;
	blockPath.push_back(step);
	view.AddNamedField(0,blockPath,"umlStereotype",B_STRING_TYPE);

	BList		*result	= view.Commands();
	BMessage	block;
	((BMessage*)result->ItemAt(0))->FindMessage("valueContainer",&block);
	const char	*existingName	= NULL;
	block.FindString("name",&existingName);
	CPPUNIT_ASSERT(BString("Priority") == existingName);	// untouched
	const char	*added	= NULL;
	CPPUNIT_ASSERT(block.FindString("umlStereotype",&added) == B_OK);
	CPPUNIT_ASSERT(BString("") == added);	// zero value, ready to edit
}


void MacroTextTest::AddFieldMenuOffersRepeatableFieldAgain(void)
{
	// Select's "node" is repeatable (Select::Do() loops FindPointer("node",
	// i,...)) - adding it via "+ Feld hinzufügen" twice should give two
	// separate entries, not overwrite the first (user report)
	BMessage	select;
	select.AddString("Command::Name","Select");

	BList	commands;
	commands.AddItem(&select);

	MacroOutlineView	view(BRect(0,0,300,300),"t",B_FOLLOW_ALL_SIDES);
	view.SetCommands(&commands);

	MacroPath	empty;
	view.AddNamedField(0,empty,"node",B_POINTER_TYPE);
	view.AddNamedField(0,empty,"node",B_POINTER_TYPE);

	BList	*result	= view.Commands();
	BMessage	*again	= (BMessage*)result->ItemAt(0);
	type_code	type;
	int32		count	= 0;
	again->GetInfo("node",&type,&count);
	CPPUNIT_ASSERT_EQUAL((int32)2,count);
}


void MacroTextTest::MoveCommandRowsMovesSeveralTogetherInOrder(void)
{
	// three top-level Moves - drag the first two together to after the
	// third, they should land in their own relative order (1,2), not
	// reversed or interleaved
	BMessage	m0; m0.AddString("Command::Name","Move"); m0.AddFloat("dx",1.0f);
	BMessage	m1; m1.AddString("Command::Name","Move"); m1.AddFloat("dx",2.0f);
	BMessage	m2; m2.AddString("Command::Name","Move"); m2.AddFloat("dx",3.0f);

	BList	commands;
	commands.AddItem(&m0);
	commands.AddItem(&m1);
	commands.AddItem(&m2);

	MacroOutlineView	view(BRect(0,0,300,300),"t",B_FOLLOW_ALL_SIDES);
	view.SetCommands(&commands);

	MacroPath	empty;
	int32	row2	= view.RowIndexForCommand(2,empty);
	CPPUNIT_ASSERT(row2 >= 0);

	std::vector<std::pair<int32,MacroPath> >	sources;
	sources.push_back(std::make_pair((int32)0,empty));
	sources.push_back(std::make_pair((int32)1,empty));
	view.MoveCommandRows(sources,row2,BPoint(0,1000000));	// far below -> after

	BList	*result	= view.Commands();
	CPPUNIT_ASSERT_EQUAL((int32)3,result->CountItems());
	float	dx0 = 0, dx1 = 0, dx2 = 0;
	((BMessage*)result->ItemAt(0))->FindFloat("dx",&dx0);
	((BMessage*)result->ItemAt(1))->FindFloat("dx",&dx1);
	((BMessage*)result->ItemAt(2))->FindFloat("dx",&dx2);
	CPPUNIT_ASSERT_EQUAL(3.0f,dx0);
	CPPUNIT_ASSERT_EQUAL(1.0f,dx1);
	CPPUNIT_ASSERT_EQUAL(2.0f,dx2);
}


void MacroTextTest::MoveCommandRowsOntoContainerAppendsAtEnd(void)
{
	BMessage	existing;
	existing.AddString("Command::Name","Move");
	existing.AddFloat("dx",9.0f);
	BMessage	batch;
	batch.AddString("Command::Name","Batch");
	batch.AddMessage("PCommand::subPCommand",&existing);

	BMessage	a; a.AddString("Command::Name","Move"); a.AddFloat("dx",1.0f);
	BMessage	b; b.AddString("Command::Name","Move"); b.AddFloat("dx",2.0f);

	BList	commands;
	commands.AddItem(&batch);
	commands.AddItem(&a);
	commands.AddItem(&b);

	MacroOutlineView	view(BRect(0,0,300,300),"t",B_FOLLOW_ALL_SIDES);
	view.SetCommands(&commands);

	MacroPath	empty;
	int32	batchRow	= view.RowIndexForCommand(0,empty);
	CPPUNIT_ASSERT(batchRow >= 0);

	std::vector<std::pair<int32,MacroPath> >	sources;
	sources.push_back(std::make_pair((int32)1,empty));
	sources.push_back(std::make_pair((int32)2,empty));
	view.MoveCommandRows(sources,batchRow,BPoint(0,0));

	BList		*result	= view.Commands();
	CPPUNIT_ASSERT_EQUAL((int32)1,result->CountItems());
	BMessage	*newBatch	= (BMessage*)result->ItemAt(0);
	BMessage	child;
	int32		count	= 0;
	float		dxs[3]	= {0,0,0};
	for (int32 i = 0; newBatch->FindMessage("PCommand::subPCommand",i,&child) == B_OK; i++) {
		child.FindFloat("dx",&dxs[i]);
		count++;
	}
	CPPUNIT_ASSERT_EQUAL((int32)3,count);
	CPPUNIT_ASSERT_EQUAL(9.0f,dxs[0]);	// already there, untouched
	CPPUNIT_ASSERT_EQUAL(1.0f,dxs[1]);	// appended, in order
	CPPUNIT_ASSERT_EQUAL(2.0f,dxs[2]);
}


void MacroTextTest::AddNodeReferenceWiresChipIdIntoTargetCommand(void)
{
	// dragging Insert's own node chip onto Select should add a
	// "node=@<that id>" field to Select (user report)
	BMessage	*insert	= BuildInsertPrototype();	// this=1
	BMessage	select;
	select.AddString("Command::Name","Select");

	PDocument	*doc	= NewRegisteredTestDocument();
	BList	commands;
	commands.AddItem(insert);
	commands.AddItem(&select);

	MacroOutlineView	view(BRect(0,0,300,300),"t",B_FOLLOW_ALL_SIDES);
	view.SetRegistryForTests(doc->GetCommandManager());
	view.SetCommands(&commands);

	MacroPath	empty;
	view.AddNodeReference(1,empty,1);

	BList		*result	= view.Commands();
	BMessage	*newSelect	= (BMessage*)result->ItemAt(1);
	int32		node	= 0;
	CPPUNIT_ASSERT(newSelect->FindInt32("node",&node) == B_OK);
	CPPUNIT_ASSERT_EQUAL((int32)1,node);
}


void MacroTextTest::AddNodeReferenceRefusesCommandWithNoNodeField(void)
{
	BMessage	sleep;
	sleep.AddString("Command::Name","Sleep");
	sleep.AddInt32("milliseconds",100);

	PDocument	*doc	= NewRegisteredTestDocument();
	BList	commands;
	commands.AddItem(&sleep);

	MacroOutlineView	view(BRect(0,0,300,300),"t",B_FOLLOW_ALL_SIDES);
	view.SetRegistryForTests(doc->GetCommandManager());
	view.SetCommands(&commands);

	MacroPath	empty;
	view.AddNodeReference(0,empty,1);

	BList		*result	= view.Commands();
	int32		node	= 0;
	CPPUNIT_ASSERT(((BMessage*)result->ItemAt(0))->FindInt32("node",&node) != B_OK);
}


void MacroTextTest::ReplaceNodeReferenceFieldOverwritesExistingValue(void)
{
	// dropped directly onto Select's own existing "node:" field row (not
	// its header) - replaces that one instance instead of adding a second
	BMessage	select;
	select.AddString("Command::Name","Select");
	select.AddInt32("node",1);

	PDocument	*doc	= NewRegisteredTestDocument();
	BList	commands;
	commands.AddItem(&select);

	MacroOutlineView	view(BRect(0,0,300,300),"t",B_FOLLOW_ALL_SIDES);
	view.SetRegistryForTests(doc->GetCommandManager());
	view.SetCommands(&commands);

	MacroPath	empty;
	view.ReplaceNodeReferenceField(0,empty,"node",0,2);

	BList		*result	= view.Commands();
	BMessage	*newSelect	= (BMessage*)result->ItemAt(0);
	int32		node	= 0;
	CPPUNIT_ASSERT(newSelect->FindInt32("node",0,&node) == B_OK);
	CPPUNIT_ASSERT_EQUAL((int32)2,node);		// overwritten, not appended
	int32		second	= 0;
	CPPUNIT_ASSERT(newSelect->FindInt32("node",1,&second) != B_OK);	// still one entry, not two
}


// ------------------------------------------- "${name}" text interpolation --

static status_t PlayMacroText(PDocument *doc, const char *text, BString *report)
{
	BList		parsed;
	BString		error;
	if (ParseCommands(BString(text),&parsed,doc->GetCommandManager(),&error) != B_OK) {
		*report	= error;
		return B_BAD_VALUE;
	}
	BMessage	macro(P_C_MACRO_TYPE);
	for (int32 i = 0; i < parsed.CountItems(); i++) {
		macro.AddMessage("Macro::Commmand",(BMessage*)parsed.ItemAt(i));
		delete (BMessage*)parsed.ItemAt(i);
	}
	return doc->GetCommandManager()->PlayMacro(&macro,report);
}


static BString NodeName(BMessage *node)
{
	BMessage	data;
	const char	*name	= "";
	node->FindMessage(P_C_NODE_DATA,&data);
	data.FindString(P_C_NODE_NAME,&name);
	return BString(name);
}


void MacroTextTest::InterpolatesCounterIntoInsertedNodeNames(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();
	BString		report;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(report.String(),(status_t)B_OK,PlayMacroText(doc,
		"Repeat\n"
		"  count=3\n"
		"  counterVariable=\"i\"\n"
		"  Insert\n"
		"    node=@1\n"
		"    ~included_node\n"
		"      this=1\n"
		"      what=class\n"
		"      Node::Frame=[0.0,0.0,50.0,50.0]\n"
		"      ~Node::Data\n"
		"        Node::name=\"N ${i}\"\n",&report));
	BList	*nodes	= doc->GetAllNodes();
	CPPUNIT_ASSERT_EQUAL((int32)3,nodes->CountItems());
	// the loop's template keeps its placeholder - every pass gets its own value
	CPPUNIT_ASSERT(NodeName((BMessage*)nodes->ItemAt(0)) == "N 0");
	CPPUNIT_ASSERT(NodeName((BMessage*)nodes->ItemAt(1)) == "N 1");
	CPPUNIT_ASSERT(NodeName((BMessage*)nodes->ItemAt(2)) == "N 2");
}


void MacroTextTest::InterpolatedNodeStaysReachableById(void)
{
	// Insert puts in a filled-in copy, not the template - a later "@1"
	// must still reach the node actually in the document. Also: a whole
	// float from Calculate reads "6", "$${" stays a literal "${".
	PDocument	*doc	= NewRegisteredTestDocument();
	BString		report;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(report.String(),(status_t)B_OK,PlayMacroText(doc,
		"Calculate\n"
		"  left=5.0\n"
		"  operator=\"+\"\n"
		"  right=1.0\n"
		"  resultVariable=\"x\"\n"
		"Insert\n"
		"  node=@1\n"
		"  ~included_node\n"
		"    this=1\n"
		"    what=class\n"
		"    Node::Frame=[0.0,0.0,50.0,50.0]\n"
		"    ~Node::Data\n"
		"      Node::name=\"${x} $${x}\"\n"
		"Select\n"
		"  node=@1\n",&report));
	BList		*nodes	= doc->GetAllNodes();
	CPPUNIT_ASSERT_EQUAL((int32)1,nodes->CountItems());
	BMessage	*node	= (BMessage*)nodes->ItemAt(0);
	CPPUNIT_ASSERT(NodeName(node) == "6 ${x}");
	CPPUNIT_ASSERT(doc->GetSelected()->HasItem(node));
}


void MacroTextTest::UnknownPlaceholderIsReported(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();
	BString		report;
	CPPUNIT_ASSERT(PlayMacroText(doc,
		"Insert\n"
		"  node=@1\n"
		"  ~included_node\n"
		"    this=1\n"
		"    what=class\n"
		"    Node::Frame=[0.0,0.0,50.0,50.0]\n"
		"    ~Node::Data\n"
		"      Node::name=\"N ${nope}\"\n",&report) != B_OK);
	CPPUNIT_ASSERT(report.FindFirst("nope") >= 0);
	// left as written, not silently emptied
	CPPUNIT_ASSERT(NodeName((BMessage*)doc->GetAllNodes()->ItemAt(0)) == "N ${nope}");
}


void MacroTextTest::ConnectionFollowsInterpolatedNodes(void)
{
	// node + node + connection in one Insert (what GraphEditor records for
	// "insert connected"), repeated: every pass's connection must join that
	// pass's own copies, never the templates
	PDocument	*doc	= NewRegisteredTestDocument();
	BString		report;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(report.String(),(status_t)B_OK,PlayMacroText(doc,
		"Repeat\n"
		"  count=2\n"
		"  counterVariable=\"i\"\n"
		"  Insert\n"
		"    node=@1\n"
		"    node=@2\n"
		"    node=@3\n"
		"    ~included_node\n"
		"      this=1\n"
		"      what=class\n"
		"      Node::Frame=[0.0,0.0,50.0,50.0]\n"
		"      ~Node::Data\n"
		"        Node::name=\"A${i}\"\n"
		"    ~included_node\n"
		"      this=2\n"
		"      what=class\n"
		"      Node::Frame=[100.0,0.0,150.0,50.0]\n"
		"      ~Node::Data\n"
		"        Node::name=\"B${i}\"\n"
		"    ~included_node\n"
		"      this=3\n"
		"      what=connection\n"
		"      Node::from=1\n"
		"      Node::to=2\n",&report));
	BList	*nodes			= doc->GetAllNodes();
	BList	*connections	= doc->GetAllConnections();
	CPPUNIT_ASSERT_EQUAL((int32)4,nodes->CountItems());
	CPPUNIT_ASSERT_EQUAL((int32)2,connections->CountItems());
	for (int32 i = 0; i < connections->CountItems(); i++) {
		BMessage	*connection	= (BMessage*)connections->ItemAt(i);
		BMessage	*from		= NULL;
		BMessage	*to			= NULL;
		connection->FindPointer(P_C_NODE_CONNECTION_FROM,(void**)&from);
		connection->FindPointer(P_C_NODE_CONNECTION_TO,(void**)&to);
		CPPUNIT_ASSERT(nodes->HasItem(from));
		CPPUNIT_ASSERT(nodes->HasItem(to));
		BString	expectedTo(NodeName(from));
		expectedTo.ReplaceFirst("A","B");
		CPPUNIT_ASSERT(NodeName(to) == expectedTo);
	}
}


void MacroTextTest::PlaceholderAloneIsNotABinding(void)
{
	// the tree editor passes a string field's text without quotes - "${i}"
	// alone must not turn into a binding to a variable named "{i}"
	BMessage	msg;
	BString		error;
	CPPUNIT_ASSERT(ParseFieldValue(&msg,"name",B_STRING_TYPE,BString("${i}"),true,&error) != B_OK);
	CPPUNIT_ASSERT(!msg.HasMessage("PCommand::bindings"));
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,
		ParseFieldValue(&msg,"name",B_STRING_TYPE,BString("\"${i}\""),true,&error));
	const char	*value	= NULL;
	CPPUNIT_ASSERT(msg.FindString("name",&value) == B_OK);
	CPPUNIT_ASSERT(BString(value) == "${i}");
}


void MacroTextTest::InterpolatesCommandSettingsInNestedBlocks(void)
{
	// the generic part: any command's own string fields, nested blocks
	// included - here AddAttribute's ~valueContainer/newAttribute
	PDocument	*doc	= NewRegisteredTestDocument();
	BString		report;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(report.String(),(status_t)B_OK,PlayMacroText(doc,
		"Calculate\n"
		"  left=2.5\n"
		"  operator=\"+\"\n"
		"  right=0.0\n"
		"  resultVariable=\"x\"\n"
		"Insert\n"
		"  node=@1\n"
		"  ~included_node\n"
		"    this=1\n"
		"    what=class\n"
		"    Node::Frame=[0.0,0.0,50.0,50.0]\n"
		"    ~Node::Data\n"
		"      Node::name=\"plain\"\n"
		"AddAttribute\n"
		"  node=@1\n"
		"  ~valueContainer\n"
		"    name=\"Pos\"\n"
		"    subgroup=\"Node::Data\"\n"
		"    type=string\n"
		"    newAttribute=\"P ${x}\"\n",&report));
	BMessage	data;
	const char	*value	= NULL;
	((BMessage*)doc->GetAllNodes()->ItemAt(0))->FindMessage(P_C_NODE_DATA,&data);
	CPPUNIT_ASSERT(data.FindString("Pos",&value) == B_OK);
	CPPUNIT_ASSERT(BString(value) == "P 2.5");
}


void MacroTextTest::LayoutKeepsGraphInsideGrownCanvas(void)
{
	// 30 unconnected nodes: dot stacks them into one long column, far
	// taller than the old graph - it must neither land above/left of the
	// canvas origin nor stay outside the document's bounds
	PDocument	*doc	= NewRegisteredTestDocument();
	BString		report;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(report.String(),(status_t)B_OK,PlayMacroText(doc,
		"Repeat\n"
		"  count=30\n"
		"  Insert\n"
		"    node=@1\n"
		"    ~included_node\n"
		"      this=1\n"
		"      what=class\n"
		"      Node::Frame=[100.0,100.0,200.0,140.0]\n"
		"Layout\n"
		"  direction=\"LR\"\n"
		"  engine=\"dot\"\n",&report));
	BList	*nodes	= doc->GetAllNodes();
	CPPUNIT_ASSERT_EQUAL((int32)30,nodes->CountItems());
	BRect	bounds	= doc->Bounds();
	float	lowest	= 0;
	for (int32 i = 0; i < nodes->CountItems(); i++) {
		BRect	frame;
		CPPUNIT_ASSERT(((BMessage*)nodes->ItemAt(i))->FindRect(P_C_NODE_FRAME,&frame) == B_OK);
		CPPUNIT_ASSERT(frame.left >= 0);
		CPPUNIT_ASSERT(frame.top >= 0);
		CPPUNIT_ASSERT(frame.right <= bounds.right);
		CPPUNIT_ASSERT(frame.bottom <= bounds.bottom);
		if (frame.bottom > lowest)
			lowest	= frame.bottom;
	}
	// actually spread out, not still stacked where Insert put them
	CPPUNIT_ASSERT(lowest > 1000);
}


// ------------------------------------- SelectConnected / GetValue (#140ff) --

static BMessage* AddTestNode(PDocument *doc, const char *name)
{
	BMessage	*node	= new BMessage(P_C_CLASS_TYPE);
	BMessage	data;
	data.AddString(P_C_NODE_NAME,name);
	node->AddMessage(P_C_NODE_DATA,&data);
	node->AddRect(P_C_NODE_FRAME,BRect(0,0,50,50));
	doc->GetAllNodes()->AddItem(node);
	return node;
}


static void AddTestConnection(PDocument *doc, BMessage *from, BMessage *to)
{
	BMessage	*connection	= new BMessage(P_C_CONNECTION_TYPE);
	connection->AddPointer(P_C_NODE_CONNECTION_FROM,from);
	connection->AddPointer(P_C_NODE_CONNECTION_TO,to);
	doc->GetAllConnections()->AddItem(connection);
}


static BString SelectedNames(PDocument *doc)
{
	std::vector<std::string>	names;
	for (int32 i = 0; i < doc->GetSelected()->CountItems(); i++)
		names.push_back(NodeName((BMessage*)doc->GetSelected()->ItemAt(i)).String());
	std::sort(names.begin(),names.end());
	BString	joined;
	for (size_t i = 0; i < names.size(); i++)
		joined << names[i].c_str();
	return joined;
}


static void SelectOnly(PDocument *doc, BMessage *node)
{
	doc->GetSelected()->MakeEmpty();
	doc->GetSelected()->AddItem(node);
}


void MacroTextTest::SelectConnectedFollowsConnections(void)
{
	// D -> A -> B -> C -> A (cycle), E unconnected
	PDocument	*doc	= NewRegisteredTestDocument();
	BMessage	*a	= AddTestNode(doc,"A");
	BMessage	*b	= AddTestNode(doc,"B");
	BMessage	*c	= AddTestNode(doc,"C");
	BMessage	*d	= AddTestNode(doc,"D");
	AddTestNode(doc,"E");
	AddTestConnection(doc,d,a);
	AddTestConnection(doc,a,b);
	AddTestConnection(doc,b,c);
	AddTestConnection(doc,c,a);
	PCommand	*command	= doc->GetCommandManager()->GetPCommand((char*)"SelectConnected");
	CPPUNIT_ASSERT(command != NULL);

	struct { const char *direction; int32 depth; const char *expected; } cases[] = {
		{ "outgoing",	0,	"ABC" },	// the cycle back to A must end the walk
		{ "incoming",	0,	"ABCD" },	// A <- D, A <- C <- B
		{ "both",		1,	"ABCD" },	// direct neighbours only: B, D, C
		{ "outgoing",	1,	"AB" },
	};
	for (size_t i = 0; i < sizeof(cases)/sizeof(cases[0]); i++) {
		SelectOnly(doc,a);
		BMessage	settings;
		settings.AddString("Command::Name","SelectConnected");
		settings.AddString("direction",cases[i].direction);
		settings.AddInt32("depth",cases[i].depth);
		BMessage	*result	= command->Do(doc,&settings);
		CPPUNIT_ASSERT_EQUAL_MESSAGE(cases[i].direction,
			std::string(cases[i].expected),std::string(SelectedNames(doc).String()));
		command->Undo(doc,result);
		CPPUNIT_ASSERT(SelectedNames(doc) == "A");
	}
}


static const char *kCostGraph =
	"Insert\n"
	"  node=@1\n"
	"  node=@2\n"
	"  node=@3\n"
	"  node=@4\n"
	"  node=@5\n"
	"  node=@6\n"
	"  ~included_node\n"
	"    this=1\n"
	"    what=class\n"
	"    Node::Frame=[0.0,0.0,50.0,50.0]\n"
	"    ~Node::Data\n"
	"      Node::name=\"A\"\n"
	"      Kosten=\"1.5\"\n"			// text, as the GraphEditor toolbar adds it
	"  ~included_node\n"
	"    this=2\n"
	"    what=class\n"
	"    Node::Frame=[0.0,0.0,50.0,50.0]\n"
	"    ~Node::Data\n"
	"      Node::name=\"B\"\n"
	"%s"									// B's Kosten line, or none
	"  ~included_node\n"
	"    this=3\n"
	"    what=class\n"
	"    Node::Frame=[0.0,0.0,50.0,50.0]\n"
	"    ~Node::Data\n"
	"      Node::name=\"C\"\n"
	"      Kosten=0.25\n"
	"  ~included_node\n"
	"    this=4\n"
	"    what=class\n"
	"    Node::Frame=[0.0,0.0,50.0,50.0]\n"
	"    ~Node::Data\n"
	"      Node::name=\"Z\"\n"
	"      Kosten=100.0\n"					// not connected - must not count
	"  ~included_node\n"
	"    this=5\n"
	"    what=connection\n"
	"    Node::from=1\n"
	"    Node::to=2\n"
	"  ~included_node\n"
	"    this=6\n"
	"    what=connection\n"
	"    Node::from=2\n"
	"    Node::to=3\n"
	"Select\n"
	"  node=@1\n"
	"SelectConnected\n"
	"Calculate\n"
	"  left=0.0\n"
	"  operator=\"+\"\n"
	"  right=0.0\n"
	"  resultVariable=\"summe\"\n"
	"ForEach\n"
	"  nodeVariable=\"n\"\n"
	"  GetValue\n"
	"    node=$n\n"
	"    ~valueContainer\n"
	"      name=\"Kosten\"\n"
	"      subgroup=\"Node::Data\"\n"
	"    resultVariable=\"k\"\n"
	"%s"									// optional default line
	"  Calculate\n"
	"    left=$summe\n"
	"    operator=\"+\"\n"
	"    right=$k\n"
	"    resultVariable=\"summe\"\n"
	"Insert\n"
	"  node=@7\n"
	"  ~included_node\n"
	"    this=7\n"
	"    what=class\n"
	"    Node::Frame=[0.0,0.0,50.0,50.0]\n"
	"    ~Node::Data\n"
	"      Node::name=\"S=${summe}\"\n";


static BString PlayCostGraph(PDocument *doc, const char *costOfB, const char *defaultLine, status_t *status)
{
	BString	text;
	text.SetToFormat(kCostGraph,costOfB,defaultLine);
	BString	report;
	*status	= PlayMacroText(doc,text.String(),&report);
	BList	*nodes	= doc->GetAllNodes();
	BString	sumNode	= NodeName((BMessage*)nodes->ItemAt(nodes->CountItems()-1));
	return sumNode << " | " << report;
}


void MacroTextTest::GetValueSumsConnectedAttribute(void)
{
	// the user's case: everything reachable from the selected node, one
	// attribute summed - text "1.5", int32 2, float 0.25; Z isn't connected
	PDocument	*doc	= NewRegisteredTestDocument();
	status_t	status	= B_ERROR;
	BString		result	= PlayCostGraph(doc,"      Kosten=2\n","",&status);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(result.String(),(status_t)B_OK,status);
	CPPUNIT_ASSERT_MESSAGE(result.String(),result.StartsWith("S=3.75 |"));
}


void MacroTextTest::GetValueReportsMissingAttribute(void)
{
	// B has no "Kosten": an error, not a silent 0 - unless default is given
	PDocument	*doc	= NewRegisteredTestDocument();
	status_t	status	= B_OK;
	BString		result	= PlayCostGraph(doc,"","",&status);
	CPPUNIT_ASSERT_MESSAGE(result.String(),status != B_OK);
	CPPUNIT_ASSERT_MESSAGE(result.String(),result.FindFirst("Kosten") >= 0);

	PDocument	*doc2	= NewRegisteredTestDocument();
	result	= PlayCostGraph(doc2,"","    default=0.0\n",&status);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(result.String(),(status_t)B_OK,status);
	CPPUNIT_ASSERT_MESSAGE(result.String(),result.StartsWith("S=1.75 |"));
}


void MacroTextTest::TextThatIsNoNumberIsReported(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();
	status_t	status	= B_OK;
	BString		result	= PlayCostGraph(doc,"      Kosten=\"viel\"\n","",&status);
	CPPUNIT_ASSERT_MESSAGE(result.String(),status != B_OK);
	CPPUNIT_ASSERT_MESSAGE(result.String(),result.FindFirst("viel") >= 0);
}


void MacroTextTest::GetValueReadsEditorAttribute(void)
{
	// GraphEditor's "add attribute" stores Node::Data/Kosten as a block
	// {Name, Value} - GetValue with name="Kosten" must read its Value
	PDocument	*doc	= NewRegisteredTestDocument();
	status_t	status	= B_ERROR;
	BString		result	= PlayCostGraph(doc,
		"      ~Kosten\n"
		"        Name=\"Kosten\"\n"
		"        Value=\"4\"\n","",&status);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(result.String(),(status_t)B_OK,status);
	CPPUNIT_ASSERT_MESSAGE(result.String(),result.StartsWith("S=5.75 |"));
}
