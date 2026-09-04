#include "LayoutEditorTest.h"

#include <string.h>

#include <app/Message.h>
#include <interface/Rect.h>
#include <support/List.h>

#include "BasePlugin.h"
#include "Batch.h"
#include "ChangeValue.h"
#include "LayoutEditor.h"
#include "PCommandManager.h"
#include "PDocument.h"
#include "ProjectConceptorDefs.h"
#include "TestDocument.h"

CPPUNIT_TEST_SUITE_REGISTRATION(LayoutEditorTest);

namespace {

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

}


void LayoutEditorTest::BuildLayoutCommandShapesOneUndoableBatch(void)
{
	// one "Batch" wrapper, one "ChangeValue" subPCommand per node/frame
	// pair - see BuildLayoutCommand()'s own comment for why ChangeValue.
	BMessage	node1(P_C_CLASS_TYPE);
	BMessage	node2(P_C_CLASS_TYPE);

	BMessage	positions;
	positions.AddPointer("node",&node1);
	positions.AddRect("frame",BRect(10,10,110,90));
	positions.AddPointer("node",&node2);
	positions.AddRect("frame",BRect(200,10,300,90));

	LayoutEditor	editor;
	BMessage	*wrapper	= editor.BuildLayoutCommand(&positions);
	CPPUNIT_ASSERT(wrapper != NULL);

	const char	*commandName	= NULL;
	CPPUNIT_ASSERT(wrapper->FindString("Command::Name",&commandName) == B_OK);
	CPPUNIT_ASSERT(strcmp(commandName,"Batch") == 0);

	BMessage	sub1;
	CPPUNIT_ASSERT(wrapper->FindMessage("PCommand::subPCommand",0,&sub1) == B_OK);
	const char	*subName	= NULL;
	CPPUNIT_ASSERT(sub1.FindString("Command::Name",&subName) == B_OK);
	CPPUNIT_ASSERT(strcmp(subName,"ChangeValue") == 0);
	void	*nodePtr1	= NULL;
	CPPUNIT_ASSERT(sub1.FindPointer("node",&nodePtr1) == B_OK);
	CPPUNIT_ASSERT_EQUAL((void *)&node1,nodePtr1);

	BMessage	valueContainer1;
	CPPUNIT_ASSERT(sub1.FindMessage("valueContainer",&valueContainer1) == B_OK);
	const char	*fieldName	= NULL;
	CPPUNIT_ASSERT(valueContainer1.FindString("name",&fieldName) == B_OK);
	CPPUNIT_ASSERT(strcmp(fieldName,P_C_NODE_FRAME) == 0);
	BRect	newValue1;
	CPPUNIT_ASSERT(valueContainer1.FindRect("newValue",&newValue1) == B_OK);
	CPPUNIT_ASSERT(newValue1 == BRect(10,10,110,90));

	BMessage	sub2;
	CPPUNIT_ASSERT(wrapper->FindMessage("PCommand::subPCommand",1,&sub2) == B_OK);
	void	*nodePtr2	= NULL;
	CPPUNIT_ASSERT(sub2.FindPointer("node",&nodePtr2) == B_OK);
	CPPUNIT_ASSERT_EQUAL((void *)&node2,nodePtr2);
	BMessage	valueContainer2;
	CPPUNIT_ASSERT(sub2.FindMessage("valueContainer",&valueContainer2) == B_OK);
	BRect	newValue2;
	CPPUNIT_ASSERT(valueContainer2.FindRect("newValue",&newValue2) == B_OK);
	CPPUNIT_ASSERT(newValue2 == BRect(200,10,300,90));
}


void LayoutEditorTest::CenterOnOldBoundsShiftsToMatchOldCenter(void)
{
	// old graph's bounding box is centered on (150,50); a fresh dot layout
	// starts near its own origin - CenterOnOldBounds() must shift every
	// "frame" entry so the NEW bounding box is re-centered on (150,50) too.
	BMessage	node1(P_C_CLASS_TYPE);
	node1.AddRect(P_C_NODE_FRAME,BRect(0,0,100,40));
	BMessage	node2(P_C_CLASS_TYPE);
	node2.AddRect(P_C_NODE_FRAME,BRect(200,60,300,100));

	BList	nodes;
	nodes.AddItem(&node1);
	nodes.AddItem(&node2);

	// new layout's own bounding box is centered on (55,15), nowhere near
	// the old (150,50) - e.g. dot's own coordinate space near its origin.
	BMessage	positions;
	positions.AddPointer("node",&node1);
	positions.AddRect("frame",BRect(0,0,10,10));
	positions.AddPointer("node",&node2);
	positions.AddRect("frame",BRect(100,20,110,30));

	LayoutEditor	editor;
	editor.CenterOnOldBounds(&nodes,&positions);

	BRect	frame1;
	CPPUNIT_ASSERT(positions.FindRect("frame",0,&frame1) == B_OK);
	BRect	frame2;
	CPPUNIT_ASSERT(positions.FindRect("frame",1,&frame2) == B_OK);

	// shift was (150,50)-(55,15) = (95,35); sizes stay untouched
	CPPUNIT_ASSERT(frame1 == BRect(95,35,105,45));
	CPPUNIT_ASSERT(frame2 == BRect(195,55,205,65));
}


void LayoutEditorTest::BatchAppliesAndUndoesAllSubcommands(void)
{
	// #116 regression (undo must restore every subcommand, not just the
	// last). Batch::Do()/Undo() called directly, same-thread - not via
	// ApplyLayout()'s SendMessage() path, which crashes a leaked
	// PDocument's looper post-teardown (see #117).
	PDocument	*doc	= NewHeadlessTestDocument();
	doc->GetCommandManager()->RegisterPCommand(new TestChangeValuePlugin());

	BMessage	node1(P_C_CLASS_TYPE);
	node1.AddRect(P_C_NODE_FRAME,BRect(0,0,100,80));
	BMessage	node2(P_C_CLASS_TYPE);
	node2.AddRect(P_C_NODE_FRAME,BRect(200,0,300,80));

	BMessage	positions;
	positions.AddPointer("node",&node1);
	positions.AddRect("frame",BRect(500,500,600,580));
	positions.AddPointer("node",&node2);
	positions.AddRect("frame",BRect(700,500,800,580));

	LayoutEditor	editor;
	BMessage	*wrapper	= editor.BuildLayoutCommand(&positions);
	CPPUNIT_ASSERT(wrapper != NULL);

	Batch	batch;
	batch.SetManager(doc->GetCommandManager());
	BMessage	*result	= batch.Do(doc,wrapper);
	CPPUNIT_ASSERT(result != NULL);

	BRect	frame1(0,0,-1,-1);
	node1.FindRect(P_C_NODE_FRAME,&frame1);
	CPPUNIT_ASSERT(frame1 == BRect(500,500,600,580));
	BRect	frame2(0,0,-1,-1);
	node2.FindRect(P_C_NODE_FRAME,&frame2);
	CPPUNIT_ASSERT(frame2 == BRect(700,500,800,580));

	batch.Undo(doc,result);

	BRect	restored1(0,0,-1,-1);
	node1.FindRect(P_C_NODE_FRAME,&restored1);
	CPPUNIT_ASSERT(restored1 == BRect(0,0,100,80));
	BRect	restored2(0,0,-1,-1);
	node2.FindRect(P_C_NODE_FRAME,&restored2);
	CPPUNIT_ASSERT(restored2 == BRect(200,0,300,80));
}
