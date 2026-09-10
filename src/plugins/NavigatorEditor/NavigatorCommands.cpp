#include "NavigatorCommands.h"

#include <app/Messenger.h>
#include <interface/GraphicsDefs.h>
#include <interface/MenuItem.h>
#include <interface/PopUpMenu.h>
#include <support/List.h>
#include <support/TypeConstants.h>

#include <Catalog.h>
#include <stdlib.h>
#include <string.h>

#include "PDocument.h"
#include "ProjectConceptorDefs.h"
#include "InputRequest.h"

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

static void NavAddAttribute(PDocument *doc, BMessage *node, int32 type)
{
	InputRequest	*inputAlert	= new InputRequest(B_TRANSLATE("Input attribute name"),
		B_TRANSLATE("Name"),B_TRANSLATE("Attribute"),B_TRANSLATE("OK"),B_TRANSLATE("Cancel"));
	char			*input		= NULL;
	if (inputAlert->Go(&input) >= 1) {
		free(input);
		return;
	}

	BMessage	*addMessage		= new BMessage(P_C_EXECUTE_COMMAND);
	addMessage->AddString("Command::Name","AddAttribute");
	addMessage->AddPointer("node",(void *)node);
	BMessage	*valueContainer	= new BMessage();
	valueContainer->AddInt32("type",B_MESSAGE_TYPE);
	valueContainer->AddString("name",input);
	valueContainer->AddString("subgroup",P_C_NODE_DATA);
	BMessage	*newAttribute	= new BMessage(type);
	newAttribute->AddString("Name",input);
	if (type == B_STRING_TYPE)
		newAttribute->AddString("Value","");
	else if (type == B_BOOL_TYPE)
		newAttribute->AddBool("Value",true);
	valueContainer->AddMessage("newAttribute",newAttribute);
	addMessage->AddMessage("valueContainer",valueContainer);
	free(input);
	BMessenger(doc).SendMessage(addMessage);
}

static void NavDeleteAttribute(PDocument *doc, BMessage *node, const char *name, int32 index)
{
	BMessage	*removeMessage	= new BMessage(P_C_EXECUTE_COMMAND);
	removeMessage->AddString("Command::Name","RemoveAttribute");
	removeMessage->AddPointer("node",(void *)node);
	BMessage	*valueContainer	= new BMessage();
	valueContainer->AddString("subgroup",P_C_NODE_DATA);
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

// Every current attribute under this node's Node::Data - each is stored
// as its own B_MESSAGE_TYPE field there (Name/Value sub-fields), the same
// shape AddAttribute/GraphEditor's own toolbar build (see
// ClassRenderer::InsertAttribute()) - not the node's plain Node::name
// string, which never matches a B_MESSAGE_TYPE lookup here.
static void NavAddDeleteAttributeItems(BMenu *deleteAttrMenu, BMessage *node)
{
	BMessage	data;
	if (node->FindMessage(P_C_NODE_DATA,&data) != B_OK)
		return;
	char	*name;
	uint32	type;
	int32	count;
	int32	i	= 0;
	while (data.GetInfo(B_MESSAGE_TYPE,i,(char **)&name,&type,&count) == B_OK) {
		BMessage	*payload	= new BMessage();
		payload->AddString("name",name);
		payload->AddInt32("index",count-1);
		deleteAttrMenu->AddItem(new BMenuItem(name,payload));
		i++;
	}
}

void NavShowNodeContextMenu(PDocument *doc, BMessage *node, bool isChildList,
	BView *owner, BPoint screenPoint)
{
	BPopUpMenu	*menu		= new BPopUpMenu("nodeContext",false,false);
	BMenuItem	*addBool	= new BMenuItem(B_TRANSLATE("Add boolean attribute"),NULL);
	BMenuItem	*addText	= new BMenuItem(B_TRANSLATE("Add text attribute"),NULL);
	menu->AddItem(addBool);
	menu->AddItem(addText);

	BMenu	*deleteAttrMenu	= new BMenu(B_TRANSLATE("Delete attribute"));
	NavAddDeleteAttributeItems(deleteAttrMenu,node);
	deleteAttrMenu->SetEnabled(deleteAttrMenu->CountItems() > 0);
	menu->AddItem(deleteAttrMenu);

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
		NavAddAttribute(doc,node,B_BOOL_TYPE);
	else if (chosen == addText)
		NavAddAttribute(doc,node,B_STRING_TYPE);
	else if (chosen == addChild)
		NavInsertNode(doc,node);
	else if (chosen == deleteNode)
		NavDeleteNode(doc,node);
	else if (chosen->Message() != NULL) {
		const char	*name	= NULL;
		int32		index	= 0;
		chosen->Message()->FindString("name",&name);
		chosen->Message()->FindInt32("index",&index);
		if (name != NULL)
			NavDeleteAttribute(doc,node,name,index);
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
