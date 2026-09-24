#include "MacroTextTest.h"

#include <app/Message.h>
#include <app/PropertyInfo.h>
#include <interface/Rect.h>
#include <support/List.h>
#include <support/String.h>

#include "AddAttribute.h"
#include "Ask.h"
#include "Batch.h"
#include "ForEach.h"
#include "If.h"
#include "Layout.h"
#include "Remember.h"
#include "Sleep.h"
#include "BasePlugin.h"
#include "ChangeValue.h"
#include "Find.h"
#include "Group.h"
#include "Insert.h"
#include <interface/Font.h>
#include <interface/GraphicsDefs.h>
#include <interface/TextView.h>
#include <map>

// the fold toggle is private on purpose; the test drives it directly
#define private public
#include "MacroTextView.h"
#undef private
#include "MacroEditor.h"
#include "MacroText.h"
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


void MacroTextTest::SnippetDropSnapsToLineBoundaryWithIndent(void)
{
	// user report: dropping a command onto the word "Find" inserted the
	// snippet mid-word and broke that command (and its Repeat).
	BString	text(
		"Repeat\n"
		"  count=3\n"
		"  Find\n"
		"    searchString=\"Test\"\n"
		"  Move\n"
		"    dx=1.0\n");
	BString	snippet("Sleep\n  milliseconds=0\n");
	int32	offset	= -1;
	BString	inserted;

	// upper half of the "  Find" line (line 2): before it, depth 1
	SnippetInsertion(text,2,false,snippet,&offset,&inserted);
	CPPUNIT_ASSERT_EQUAL((int32)text.FindFirst("  Find"),offset);
	CPPUNIT_ASSERT(inserted == "  Sleep\n    milliseconds=0\n");

	// lower half of the "  Find" line: after it AND its nested searchString
	SnippetInsertion(text,2,true,snippet,&offset,&inserted);
	CPPUNIT_ASSERT_EQUAL((int32)text.FindFirst("  Move"),offset);
	CPPUNIT_ASSERT(inserted == "  Sleep\n    milliseconds=0\n");

	// lower half of the top-level "Repeat": after the whole block, depth 0
	SnippetInsertion(text,0,true,snippet,&offset,&inserted);
	CPPUNIT_ASSERT_EQUAL(text.Length(),offset);
	CPPUNIT_ASSERT(inserted == "Sleep\n  milliseconds=0\n");

	// last line without a trailing newline: gets one first
	BString	noNewline("Move\n  dx=1.0");
	SnippetInsertion(noNewline,1,true,snippet,&offset,&inserted);
	CPPUNIT_ASSERT_EQUAL(noNewline.Length(),offset);
	CPPUNIT_ASSERT(inserted == "\n  Sleep\n    milliseconds=0\n");

	// empty text
	SnippetInsertion(BString(""),0,false,snippet,&offset,&inserted);
	CPPUNIT_ASSERT_EQUAL((int32)0,offset);
	CPPUNIT_ASSERT(inserted == "Sleep\n  milliseconds=0\n");

	// the result of dropping into the middle of the macro still parses
	BString	result(text);
	SnippetInsertion(text,2,false,snippet,&offset,&inserted);
	result.Insert(inserted,offset);
	PDocument	*doc	= NewRegisteredTestDocument();
	BList		parsed;
	BString		error;
	BString		message("dropped result: ");
	status_t	err	= ParseCommands(result,&parsed,doc->GetCommandManager(),&error);
	message << error;
	CPPUNIT_ASSERT_MESSAGE(message.String(),err == B_OK);
}


void MacroTextTest::IndentChangeMovesInAndOutWithoutGoingNegative(void)
{
	CPPUNIT_ASSERT_EQUAL((int32)2,IndentChange(BString("Find"),1));
	CPPUNIT_ASSERT_EQUAL((int32)4,IndentChange(BString("  Find"),2));
	CPPUNIT_ASSERT_EQUAL((int32)-2,IndentChange(BString("    Find"),-1));
	// already at depth 0 - nothing to remove
	CPPUNIT_ASSERT_EQUAL((int32)0,IndentChange(BString("Find"),-1));
	// never more than the line's own leading spaces
	CPPUNIT_ASSERT_EQUAL((int32)-2,IndentChange(BString("  Find"),-3));

	// end to end: a block moved in one level is a subcommand of the
	// command line above it, and parses as one
	BString	block("Find\n  searchString=\"Test\"\n");
	BString	moved;
	BString	line;
	for (int32 i = 0; i < block.Length(); i++) {
		if (block[i] != '\n') {
			line.Append(block.String()+i,1);
			continue;
		}
		for (int32 n = IndentChange(line,1); n > 0; n--)
			moved << " ";
		moved << line << "\n";
		line.SetTo("");
	}
	BString	text("Repeat\n  count=2\n");
	text << moved;
	PDocument	*doc	= NewRegisteredTestDocument();
	BList		parsed;
	BString		error;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,
		ParseCommands(text,&parsed,doc->GetCommandManager(),&error));
	BMessage	*repeat	= (BMessage*)parsed.ItemAt(0);
	CPPUNIT_ASSERT(repeat->HasMessage("PCommand::subPCommand"));
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
	PDocument	*doc	= NewRegisteredTestDocument();
	BString		text;
	InsertPrototypeText(&text);
	BList		parsed;
	BString		error;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,
		ParseCommands(text,&parsed,doc->GetCommandManager(),&error));
	BMessage	*insert	= (BMessage*)parsed.ItemAt(0);
	BMessage	node;
	CPPUNIT_ASSERT(insert->FindMessage("included_node",&node) == B_OK);
	CPPUNIT_ASSERT_EQUAL((uint32)P_C_CLASS_TYPE,(uint32)node.what);
	BRect		frame;
	CPPUNIT_ASSERT(node.FindRect(P_C_NODE_FRAME,&frame) == B_OK);
	BMessage	data;
	CPPUNIT_ASSERT(node.FindMessage(P_C_NODE_DATA,&data) == B_OK);
	CPPUNIT_ASSERT(node.HasMessage(P_C_NODE_PATTERN));
}


// MacroTextView only calls back into these two - real MacroEditor is not
// part of the test binary
bool MacroEditor::ApplyEdits(bool) {return true;}
void MacroEditor::UpdateCursorPosition(int32, int32) {}


void MacroTextTest::FoldedChipsSurviveWrapAndNewLine(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();
	BString		insertText;
	InsertPrototypeText(&insertText);

	// two dropped Insert prototypes (both carry this=1), wrapped in Repeat
	BString	canonical("Repeat\n  count=2\n");
	for (int32 copy = 0; copy < 2; copy++) {
		int32	at	= 0;
		while (at < insertText.Length()) {
			int32	nl	= insertText.FindFirst("\n",at);
			canonical << "  " << BString(insertText.String()+at,nl+1-at);
			at	= nl+1;
		}
	}

	MacroTextView	view(BRect(0,0,300,300),"t",BRect(4,4,296,296),B_FOLLOW_ALL_SIDES,B_WILL_DRAW);
	view.SetMacroText(canonical);

	BList		parsed;
	BString		expanded;
	BString		error;
	view.ExpandedText(&expanded);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(error.String(),(status_t)B_OK,
		ParseCommands(expanded,&parsed,doc->GetCommandManager(),&error));
	CPPUNIT_ASSERT(expanded == canonical);

	// Enter after the last chip
	BString	text(view.Text());
	int32	lastChip	= text.FindLast(">> ~included_node");
	int32	chipEnd		= text.FindFirst("\n",lastChip);
	CPPUNIT_ASSERT(lastChip > 0);
	view.Insert(chipEnd,"\n",1);

	expanded	= "";
	view.ExpandedText(&expanded);
	BList	parsed2;
	error	= "";
	CPPUNIT_ASSERT_EQUAL_MESSAGE(error.String(),(status_t)B_OK,
		ParseCommands(expanded,&parsed2,doc->GetCommandManager(),&error));

	// chips still expand: first one via the double-click path
	BString	afterToggle(view.Text());
	int32	firstChip	= afterToggle.FindFirst(">> ~included_node");
	int32	firstChipEnd	= afterToggle.FindFirst("\n",firstChip);
	int32	firstChipStart	= afterToggle.FindLast("\n",firstChip)+1;
	view.ToggleFoldAtLine(firstChipStart,firstChipEnd+1);
	expanded	= "";
	view.ExpandedText(&expanded);
	BList	parsed3;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(error.String(),(status_t)B_OK,
		ParseCommands(expanded,&parsed3,doc->GetCommandManager(),&error));
}


void MacroTextTest::ChipsFollowIndentShiftOfTheirLine(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();
	BString		canonical;
	InsertPrototypeText(&canonical);
	BString		insertText(canonical);
	canonical << insertText;

	MacroTextView	view(BRect(0,0,300,300),"t",BRect(4,4,296,296),B_FOLLOW_ALL_SIDES,B_WILL_DRAW);
	view.SetMacroText(canonical);

	// wrap: everything indented one level, Repeat in front
	view.Select(0,view.TextLength());
	view.ShiftSelectedLines(1);
	BString	header("Repeat\n  count=2\n  counterVariable=\"\"\n");
	view.Insert(0,header.String(),header.Length());

	BString	expanded;
	view.ExpandedText(&expanded);
	BList	parsed;
	BString	error;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(error.String(),(status_t)B_OK,
		ParseCommands(expanded,&parsed,doc->GetCommandManager(),&error));

	// drop the counterVariable line, put it back, Enter after the last chip
	view.Delete(header.Length()-BString("  counterVariable=\"\"\n").Length(),header.Length());
	view.Insert(BString("Repeat\n  count=2\n").Length(),"  counterVariable=\"\"\n",22);
	BString	text(view.Text());
	int32	lastChip	= text.FindLast(">> ~included_node");
	view.Insert(text.FindFirst("\n",lastChip),"\n",1);

	expanded	= "";
	view.ExpandedText(&expanded);
	BList	parsed2;
	error	= "";
	CPPUNIT_ASSERT_EQUAL_MESSAGE(error.String(),(status_t)B_OK,
		ParseCommands(expanded,&parsed2,doc->GetCommandManager(),&error));
}


void MacroTextTest::EditedChipIdStillFindsItsBlock(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();
	BString		canonical;
	InsertPrototypeText(&canonical);
	BString		second(canonical);
	RenumberInsertPrototype(&second,2);
	canonical << second;

	MacroTextView	view(BRect(0,0,300,300),"t",BRect(4,4,296,296),B_FOLLOW_ALL_SIDES,B_WILL_DRAW);
	view.SetMacroText(canonical);

	// retype the second chip's id, [@2] -> [@7]
	BString	text(view.Text());
	int32	at	= text.FindFirst("[@2]");
	CPPUNIT_ASSERT(at > 0);
	view.Delete(at+2,at+3);
	view.Insert(at+2,"7",1);

	BString	expanded;
	view.ExpandedText(&expanded);
	CPPUNIT_ASSERT(expanded.FindFirst("this=7") >= 0);
	BList	parsed;
	BString	error;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(error.String(),(status_t)B_OK,
		ParseCommands(expanded,&parsed,doc->GetCommandManager(),&error));
}


void MacroTextTest::DroppedPrototypesGetDistinctIds(void)
{
	BString	first;
	InsertPrototypeText(&first);
	CPPUNIT_ASSERT_EQUAL((int32)1,HighestReferencedId(first));
	BString	second(first);
	RenumberInsertPrototype(&second,2);
	CPPUNIT_ASSERT(second.FindFirst("node=@2") >= 0);
	CPPUNIT_ASSERT(second.FindFirst("this=2") >= 0);
	CPPUNIT_ASSERT(second.FindFirst("node=@1") < 0);
	CPPUNIT_ASSERT_EQUAL((int32)2,HighestReferencedId(second));
	CPPUNIT_ASSERT_EQUAL((int32)-1,HighestReferencedId(BString("Find\n")));
}


void MacroTextTest::RepeatedInsertCreatesDistinctNodes(void)
{
	// user report: "create six nodes" macro - Repeat around two Inserts and
	// a Batch of Select/Move played back and did nothing
	PDocument	*doc	= NewRegisteredTestDocument();
	BString		first;
	InsertPrototypeText(&first);
	BString		second(first);
	RenumberInsertPrototype(&second,2);
	BString		text("Repeat\n  count=3\n");
	BString		both(first);
	both << second;
	int32		at	= 0;
	while (at < both.Length()) {
		int32	nl	= both.FindFirst("\n",at);
		text << "  " << BString(both.String()+at,nl+1-at);
		at	= nl+1;
	}
	text <<
		"  Batch\n"
		"    Select\n"
		"      node=@1\n"
		"      deselect=false\n"
		"      selectAll=false\n"
		"    Move\n"
		"      dx=100.0\n"
		"      dy=100.0\n";

	BList		parsed;
	BString		error;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(error.String(),(status_t)B_OK,
		ParseCommands(text,&parsed,doc->GetCommandManager(),&error));
	BMessage	macro(P_C_MACRO_TYPE);
	for (int32 i = 0; i < parsed.CountItems(); i++)
		macro.AddMessage("Macro::Commmand",(BMessage*)parsed.ItemAt(i));
	doc->GetCommandManager()->PlayMacro(&macro);

	BList	*nodes	= doc->GetAllNodes();
	CPPUNIT_ASSERT_EQUAL((int32)6,nodes->CountItems());
	for (int32 a = 0; a < nodes->CountItems(); a++)
		for (int32 b = a+1; b < nodes->CountItems(); b++)
			CPPUNIT_ASSERT(nodes->ItemAt(a) != nodes->ItemAt(b));
}


void MacroTextTest::FoldedChipInFileGivesSpecificError(void)
{
	PDocument	*doc	= NewRegisteredTestDocument();
	BList		parsed;
	BString		error;
	CPPUNIT_ASSERT_EQUAL((status_t)B_BAD_VALUE,ParseCommands(
		BString("Insert\n  node=@1\n  >> ~included_node[@1] \"New Node 1\"\n"),
		&parsed,doc->GetCommandManager(),&error));
	CPPUNIT_ASSERT(error.StartsWith("line 3: folded chip"));
}
