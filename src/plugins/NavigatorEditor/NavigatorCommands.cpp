#include "NavigatorCommands.h"

#include <app/Messenger.h>
#include <interface/GraphicsDefs.h>
#include <interface/MenuItem.h>
#include <interface/OutlineListView.h>
#include <interface/PopUpMenu.h>
#include <support/List.h>
#include <support/TypeConstants.h>

#include <Catalog.h>
#include <stdlib.h>
#include <string.h>

#include "PDocument.h"
#include "ProjectConceptorDefs.h"
#include "InputRequest.h"
#include "NavigatorEditor.h"
#include "NodeItem.h"
#include "BaseListItem.h"
#include "MessageListView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "NavigatorCommands"


// Same node shape GenerateStressFixture.cpp uses (verified against a real
// node dumped from the running app) - packed int32 colors, not AddRGBColor
// (ClassRenderer reads them back via FindInt32(), not FindColor()).
static BMessage* NavBuildNewNode(const char *name)
{
	BMessage	*data	= new BMessage();
	data->AddString(P_C_NODE_NAME,name);

	BMessage	*font	= new BMessage('fOTy');
	font->AddInt8("Font::Encoding",0);
	font->AddInt16("Font::Face",0x40);
	font->AddString("Font::Family","Noto Sans");
	font->AddInt32("Font::Flags",0);
	font->AddFloat("Font::Rotation",0.0);
	font->AddFloat("Font::Shear",90.0);
	font->AddFloat("Font::Size",12.0);
	font->AddInt8("Font::Spacing",2);
	font->AddString("Font::Style","Regular");
	font->AddInt32("Font::Color",(int32)0xffb5976f);

	BMessage	*pattern	= new BMessage();
	rgb_color	fillColor	= {152,180,190,255};
	pattern->AddInt32("FillColor",*(int32*)&fillColor);
	rgb_color	borderColor	= {0,0,0,255};
	pattern->AddInt32("BorderColor",*(int32*)&borderColor);
	pattern->AddFloat("PenSize",1.0);
	pattern->AddInt8("DrawingMode",B_OP_ALPHA);
	rgb_color	highColor	= {0,0,0,255};
	pattern->AddInt32("HighColor",*(int32*)&highColor);
	rgb_color	lowColor	= {128,128,128,255};
	pattern->AddInt32("LowColor",*(int32*)&lowColor);
	pattern->AddData("Pattern",B_PATTERN_TYPE,(const void *)&B_SOLID_HIGH,sizeof(B_SOLID_HIGH),false);

	BMessage	*node	= new BMessage(P_C_CLASS_TYPE);
	node->AddMessage(P_C_NODE_DATA,data);
	// Navigator has no canvas to place a new node against - a fixed spot
	// near the origin, same as any other freshly inserted node until it's
	// dragged into position in GraphEditor.
	node->AddRect(P_C_NODE_FRAME,BRect(20,20,120,60));
	node->AddMessage(P_C_NODE_FONT,font);
	node->AddMessage(P_C_NODE_PATTERN,pattern);
	node->AddBool(P_C_NODE_SELECTED,false);
	return node;
}

static void NavInsertNode(PDocument *doc, BMessage *parentNode)
{
	InputRequest	*inputAlert	= new InputRequest(B_TRANSLATE("Node name"),
		B_TRANSLATE("Name"),B_TRANSLATE("Untitled"),B_TRANSLATE("OK"),B_TRANSLATE("Cancel"));
	char			*input		= NULL;
	if (inputAlert->Go(&input) >= 1) {
		free(input);
		return;
	}

	BMessage	*node	= NavBuildNewNode(input);
	free(input);
	if (parentNode != NULL)
		node->AddPointer(P_C_NODE_PARENT,parentNode);

	BMessage	*commandMessage	= new BMessage(P_C_EXECUTE_COMMAND);
	commandMessage->AddString("Command::Name","Insert");
	commandMessage->AddPointer("node",(void *)node);
	BMessenger(doc).SendMessage(commandMessage);
}

// A node's structural fields - always present, read by GraphEditor/
// LayoutEditor/the renderers. The generic "delete field" menu leaves
// these out: unlike a field the user added through NavigatorEditor
// itself, removing one of these doesn't just make the data invisible
// elsewhere, it breaks this app's own rendering (a node with no Frame,
// say). "Node::name" isn't listed separately - it lives inside
// Node::data, not at the node's own top level.
static bool NavIsStructuralField(const char *name)
{
	return strcmp(name,P_C_NODE_DATA) == 0
		|| strcmp(name,P_C_NODE_FRAME) == 0
		|| strcmp(name,P_C_NODE_FONT) == 0
		|| strcmp(name,P_C_NODE_PATTERN) == 0
		|| strcmp(name,P_C_NODE_SELECTED) == 0
		|| strcmp(name,P_C_NODE_OUTGOING) == 0
		|| strcmp(name,P_C_NODE_INCOMING) == 0
		|| strcmp(name,P_C_NODE_PARENT) == 0
		|| strcmp(name,P_C_NODE_ALLNODES) == 0
		|| strstr(name,"GraphEditor") != NULL
		|| strcmp(name,"ProjectConceptor::doc") == 0;
}

// Adds a plain field directly to "node" itself - not wrapped in
// GraphEditor's own Name/Value attribute shape (see AddAttribute's
// G_E_ADD_ATTRIBUTE handling in GraphEditor.cpp), which only its own
// ClassRenderer knows how to draw. NavigatorEditor's job is to reach
// the BMessage directly: whatever field type BMessage itself supports,
// this can add, whether or not any other editor can make sense of it
// afterwards. AddAttribute::DoAddAttribute() is already fully generic
// (just AddData(name,type,value,size) on whatever "subgroup" chain it's
// given - empty here, meaning the node itself) - no new command needed.
static void NavAddField(PDocument *doc, BMessage *node, int32 type)
{
	InputRequest	*inputAlert	= new InputRequest(B_TRANSLATE("Field name"),
		B_TRANSLATE("Name"),B_TRANSLATE("Field"),B_TRANSLATE("OK"),B_TRANSLATE("Cancel"));
	char			*input		= NULL;
	if (inputAlert->Go(&input) >= 1) {
		free(input);
		return;
	}

	BMessage	*addMessage		= new BMessage(P_C_EXECUTE_COMMAND);
	addMessage->AddString("Command::Name","AddAttribute");
	addMessage->AddPointer("node",(void *)node);
	BMessage	*valueContainer	= new BMessage();
	valueContainer->AddInt32("type",type);
	valueContainer->AddString("name",input);
	// A sensible empty/zero default - the field shows up right away and
	// can be edited in place like any other row.
	switch (type) {
		case B_BOOL_TYPE:	valueContainer->AddBool("newAttribute",false); break;
		case B_INT32_TYPE:	valueContainer->AddInt32("newAttribute",0); break;
		case B_FLOAT_TYPE:	valueContainer->AddFloat("newAttribute",0.0f); break;
		case B_STRING_TYPE:	valueContainer->AddString("newAttribute",""); break;
		case B_RECT_TYPE:	valueContainer->AddRect("newAttribute",BRect(0,0,0,0)); break;
		case B_MESSAGE_TYPE: {
			// AddMessage() stores a submessage as flattened B_MESSAGE_TYPE
			// data internally - the exact same shape AddAttribute's own
			// generic AddData(name,type,value,size) then writes onto the
			// node, so an empty BMessage here round-trips through
			// FindMessage() like any other nested field.
			BMessage	empty;
			valueContainer->AddMessage("newAttribute",&empty);
			break;
		}
	}
	addMessage->AddMessage("valueContainer",valueContainer);
	free(input);
	BMessenger(doc).SendMessage(addMessage);
}

// subgroup == NULL means "name" lives directly on "node" itself; a
// non-NULL subgroup (only ever P_C_NODE_DATA today) reaches one of
// GraphEditor's own Name/Value-wrapped attributes instead.
static void NavDeleteField(PDocument *doc, BMessage *node, const char *subgroup,
	const char *name, int32 index)
{
	BMessage	*removeMessage	= new BMessage(P_C_EXECUTE_COMMAND);
	removeMessage->AddString("Command::Name","RemoveAttribute");
	removeMessage->AddPointer("node",(void *)node);
	BMessage	*valueContainer	= new BMessage();
	if (subgroup != NULL)
		valueContainer->AddString("subgroup",subgroup);
	valueContainer->AddString("name",name);
	valueContainer->AddInt32("index",index);
	removeMessage->AddMessage("valueContainer",valueContainer);
	BMessenger(doc).SendMessage(removeMessage);
}

static void NavDeleteNode(PDocument *doc, BMessage *node)
{
	BList	*selected	= doc->GetSelected();
	for (int32 i=selected->CountItems()-1; i>=0; i--)
		((BMessage *)selected->ItemAt(i))->ReplaceBool(P_C_NODE_SELECTED,0,false);
	selected->MakeEmpty();
	node->ReplaceBool(P_C_NODE_SELECTED,0,true);
	selected->AddItem(node);

	BMessage	*deleteMessage	= new BMessage(P_C_EXECUTE_COMMAND);
	deleteMessage->AddString("Command::Name","Delete");
	BMessenger(doc).SendMessage(deleteMessage);
}

// One menu item per deletable field, both kinds:
//  - GraphEditor's own Name/Value-wrapped attributes under Node::data
//    (each its own B_MESSAGE_TYPE field there)
//  - plain top-level fields NavAddField() creates directly on the node,
//    excluding the structural ones (see NavIsStructuralField())
// The payload carries enough for NavDeleteField() to reach either kind
// without the caller needing to know which one it picked.
static void NavAddDeleteFieldItems(BMenu *deleteMenu, BMessage *node)
{
	BMessage	data;
	char		*name;
	uint32		type;
	int32		count;
	int32		i	= 0;
	if (node->FindMessage(P_C_NODE_DATA,&data) == B_OK) {
		while (data.GetInfo(B_MESSAGE_TYPE,i,(char **)&name,&type,&count) == B_OK) {
			BMessage	*payload	= new BMessage();
			payload->AddString("subgroup",P_C_NODE_DATA);
			payload->AddString("name",name);
			payload->AddInt32("index",count-1);
			deleteMenu->AddItem(new BMenuItem(name,payload));
			i++;
		}
	}
	i = 0;
	while (node->GetInfo(B_ANY_TYPE,i,(char **)&name,&type,&count) == B_OK) {
		// B_MESSAGE_TYPE isn't excluded here the way it is above - Data/
		// Font/Pattern are already kept out by name via
		// NavIsStructuralField(), and a user-added nested Message field
		// (see NavAddField()) needs to be deletable like anything else.
		if ((type != B_POINTER_TYPE) && !NavIsStructuralField(name)) {
			BMessage	*payload	= new BMessage();
			payload->AddString("name",name);
			payload->AddInt32("index",count-1);
			deleteMenu->AddItem(new BMenuItem(name,payload));
		}
		i++;
	}
}

void NavShowNodeContextMenu(PDocument *doc, BMessage *node, bool isChildList,
	BView *owner, BPoint screenPoint)
{
	BPopUpMenu	*menu		= new BPopUpMenu("nodeContext",false,false);

	BMenu		*addFieldMenu	= new BMenu(B_TRANSLATE("Add field"));
	BMenuItem	*addBool	= new BMenuItem(B_TRANSLATE("Boolean"),NULL);
	BMenuItem	*addInt		= new BMenuItem(B_TRANSLATE("Integer"),NULL);
	BMenuItem	*addFloat	= new BMenuItem(B_TRANSLATE("Float"),NULL);
	BMenuItem	*addText	= new BMenuItem(B_TRANSLATE("Text"),NULL);
	BMenuItem	*addRect	= new BMenuItem(B_TRANSLATE("Rectangle"),NULL);
	BMenuItem	*addMsg		= new BMenuItem(B_TRANSLATE("Message (nested fields)"),NULL);
	addFieldMenu->AddItem(addBool);
	addFieldMenu->AddItem(addInt);
	addFieldMenu->AddItem(addFloat);
	addFieldMenu->AddItem(addText);
	addFieldMenu->AddItem(addRect);
	addFieldMenu->AddItem(addMsg);
	menu->AddItem(addFieldMenu);

	BMenu	*deleteFieldMenu	= new BMenu(B_TRANSLATE("Delete field"));
	NavAddDeleteFieldItems(deleteFieldMenu,node);
	deleteFieldMenu->SetEnabled(deleteFieldMenu->CountItems() > 0);
	menu->AddItem(deleteFieldMenu);

	menu->AddSeparatorItem();
	BMenuItem	*addChild	= NULL;
	if (isChildList) {
		addChild	= new BMenuItem(B_TRANSLATE("Add child node"),NULL);
		menu->AddItem(addChild);
	}
	BMenuItem	*deleteNode	= new BMenuItem(B_TRANSLATE("Delete node"),NULL);
	menu->AddItem(deleteNode);

	menu->SetTargetForItems(owner);
	BMenuItem	*chosen	= menu->Go(screenPoint,true,true);
	if (chosen == NULL)
		return;
	if (chosen == addBool)
		NavAddField(doc,node,B_BOOL_TYPE);
	else if (chosen == addInt)
		NavAddField(doc,node,B_INT32_TYPE);
	else if (chosen == addFloat)
		NavAddField(doc,node,B_FLOAT_TYPE);
	else if (chosen == addText)
		NavAddField(doc,node,B_STRING_TYPE);
	else if (chosen == addRect)
		NavAddField(doc,node,B_RECT_TYPE);
	else if (chosen == addMsg)
		NavAddField(doc,node,B_MESSAGE_TYPE);
	else if (chosen == addChild)
		NavInsertNode(doc,node);
	else if (chosen == deleteNode)
		NavDeleteNode(doc,node);
	else if (chosen->Message() != NULL) {
		const char	*subgroup	= NULL;
		const char	*name		= NULL;
		int32		index		= 0;
		chosen->Message()->FindString("subgroup",&subgroup);
		chosen->Message()->FindString("name",&name);
		chosen->Message()->FindInt32("index",&index);
		if (name != NULL)
			NavDeleteField(doc,node,subgroup,name,index);
	}
}

void NavShowEmptyContextMenu(PDocument *doc, BMessage *parentNode,
	BView *owner, BPoint screenPoint)
{
	BPopUpMenu	*menu		= new BPopUpMenu("emptyContext",false,false);
	BMenuItem	*addNode	= new BMenuItem(
		parentNode ? B_TRANSLATE("Add child node") : B_TRANSLATE("Add node"),NULL);
	menu->AddItem(addNode);
	menu->SetTargetForItems(owner);
	BMenuItem	*chosen	= menu->Go(screenPoint,true,true);
	if (chosen == addNode)
		NavInsertNode(doc,parentNode);
}

void NavSetFocusedList(NavigatorEditor *editor, BListView *list)
{
	editor->SetFocusedList(list);
}

void NavToolbarAddNode(PDocument *doc, BListView *focusedList)
{
	if (focusedList == NULL)
		return;
	BListItem	*item	= focusedList->ItemAt(focusedList->CurrentSelection(0));
	NodeItem	*node	= dynamic_cast<NodeItem *>(item);
	if (node != NULL)
		NavInsertNode(doc,node->GetNode());
	else if (item == NULL)
		// nothing selected - a top-level node, same as the empty-space
		// right-click menu's default.
		NavInsertNode(doc,NULL);
}

// The index (position among possibly-several same-name/same-type
// values - see AddAttribute::DoAddAttribute()'s own "lastIndex" scan)
// of the *last* field on "container" matching name+type. Needed because
// a plain field row (unlike the ones in the "Delete field" submenu,
// which already knows its own index from when it was built) only knows
// its own name and type, not its position.
static bool NavFindFieldIndex(BMessage *container, const char *name,
	type_code type, int32 *outIndex)
{
	char		*tmpName;
	uint32		tmpType;
	int32		count;
	int32		i			= 0;
	int32		lastIndex	= -1;
	while (container->GetInfo(B_ANY_TYPE,i,(char **)&tmpName,&tmpType,&count) == B_OK) {
		if ((strcmp(tmpName,name) == 0) && (tmpType == type))
			lastIndex	= count-1;
		i++;
	}
	if (lastIndex < 0)
		return false;
	*outIndex	= lastIndex;
	return true;
}

void NavToolbarDelete(PDocument *doc, BListView *focusedList)
{
	if (focusedList == NULL)
		return;
	BListItem	*item	= focusedList->ItemAt(focusedList->CurrentSelection(0));
	if (item == NULL)
		return;

	NodeItem	*nodeItem	= dynamic_cast<NodeItem *>(item);
	if (nodeItem != NULL) {
		NavDeleteNode(doc,nodeItem->GetNode());
		return;
	}

	// A plain field row (Bool/String/Float/Rect/Int32/ColorItem) -
	// only ones at the top level of their own column are handled
	// directly here; a nested, GraphEditor-style wrapped attribute
	// still goes through the right-click "Delete field" submenu, which
	// already tracks its own subgroup/index from when it built that
	// menu, rather than trying to reconstruct an arbitrary subgroup
	// chain here from just the selected row.
	BaseListItem		*base		= dynamic_cast<BaseListItem *>(item);
	BOutlineListView	*outline	= dynamic_cast<BOutlineListView *>(focusedList);
	MessageListView		*column		= dynamic_cast<MessageListView *>(focusedList);
	if ((base == NULL) || (base->GetLabel() == NULL)
			|| (outline == NULL) || (outline->Superitem(item) != NULL)
			|| (column == NULL))
		return;

	int32	index;
	if (NavFindFieldIndex(column->GetContainer(),base->GetLabel(),
			base->GetSupportedType(),&index))
		NavDeleteField(doc,column->GetContainer(),NULL,base->GetLabel(),index);
}
