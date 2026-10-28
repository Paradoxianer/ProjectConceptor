#include <interface/Box.h>
#include <interface/Button.h>
#include <interface/LayoutBuilder.h>
#include <interface/MenuItem.h>
#include <interface/PopUpMenu.h>
#include <Catalog.h>

#include <set>

#include "FindWindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "FindWindow"

static const uint32	kOK				= 'ok  ';
static const uint32	kCancel			= 'cl  ';
static const uint32	kTextChanged	= 'live';
static const uint32	kCriterionChanged	= 'crit';

/** operator codes of NodeSearch.h and how the menu shows them */
static const char* const kOperators[][2] = {
	{ "contains",	B_TRANSLATE_MARK("contains") },
	{ "==",			"=" },
	{ "!=",			"≠" },
	{ "<",			"<" },
	{ "<=",			"≤" },
	{ ">",			">" },
	{ ">=",			"≥" },
};


FindWindow::FindWindow(PDocument *tmpDoc)
	: BWindow(BRect(50,50,500,200),B_TRANSLATE("Find"),B_FLOATING_WINDOW_LOOK,
		B_FLOATING_APP_WINDOW_FEEL,B_AUTO_UPDATE_SIZE_LIMITS | B_NOT_ZOOMABLE),
	useCriterion(false),
	doc(tmpDoc)
{
	TRACE();
	CreateViews();
}


void FindWindow::CollectAttributeNames(BMessage *names)
{
	std::set<BString>	found;
	BList	*nodes	= doc->GetAllNodes();
	for (int32 n = 0; n < nodes->CountItems(); n++) {
		BMessage	data;
		if (((BMessage*)nodes->ItemAt(n))->FindMessage(P_C_NODE_DATA,&data) != B_OK)
			continue;
		char		*key	= NULL;
		type_code	type	= B_ANY_TYPE;
		int32		count	= 0;
		for (int32 i = 0; data.GetInfo(B_MESSAGE_TYPE,i,&key,&type,&count) == B_OK; i++) {
			for (int32 entry = 0; entry < count; entry++) {
				BMessage	attribute;
				const char	*name	= NULL;
				if ((data.FindMessage(key,entry,&attribute) == B_OK)
						&& (attribute.FindString(P_C_ATTRIBUTE_NAME,&name) == B_OK))
					found.insert(BString(name));
			}
		}
	}
	for (std::set<BString>::iterator it = found.begin(); it != found.end(); it++)
		names->AddString("name",*it);
}


void FindWindow::CreateViews()
{
	TRACE();
	searchText	= new BTextControl("searchText",B_TRANSLATE("Search"),"",NULL);
	searchText->SetModificationMessage(new BMessage(kTextChanged));

	BPopUpMenu	*attributes	= new BPopUpMenu("attributes");
	BMessage	*any		= new BMessage(kCriterionChanged);
	any->AddString(P_C_FIND_ATTRIBUTE,"");
	attributes->AddItem(new BMenuItem(B_TRANSLATE("Any attribute"),any));
	BMessage	*nodeName	= new BMessage(kCriterionChanged);
	nodeName->AddString(P_C_FIND_ATTRIBUTE,"name");
	attributes->AddItem(new BMenuItem(B_TRANSLATE("Node name"),nodeName));
	BMessage	names;
	CollectAttributeNames(&names);
	const char	*name	= NULL;
	if (names.FindString("name",&name) == B_OK)
		attributes->AddSeparatorItem();
	for (int32 i = 0; names.FindString("name",i,&name) == B_OK; i++) {
		BMessage	*pick	= new BMessage(kCriterionChanged);
		pick->AddString(P_C_FIND_ATTRIBUTE,name);
		attributes->AddItem(new BMenuItem(name,pick));
	}
	attributes->ItemAt(0)->SetMarked(true);
	attributeField	= new BMenuField("attribute",NULL,attributes);

	BPopUpMenu	*operators	= new BPopUpMenu("operators");
	for (uint32 i = 0; i < sizeof(kOperators) / sizeof(kOperators[0]); i++) {
		BMessage	*pick	= new BMessage(kCriterionChanged);
		pick->AddString(P_C_FIND_OPERATOR,kOperators[i][0]);
		operators->AddItem(new BMenuItem(B_TRANSLATE_NOCOLLECT(kOperators[i][1]),pick));
	}
	operators->ItemAt(0)->SetMarked(true);
	operatorField	= new BMenuField("operator",NULL,operators);

	valueText	= new BTextControl("value",NULL,"",NULL);
	valueText->SetModificationMessage(new BMessage(kCriterionChanged));

	BBox	*compare	= new BBox("compare");
	compare->SetLabel(B_TRANSLATE("Or compare an attribute"));
	BGroupView	*row	= new BGroupView(B_HORIZONTAL,B_USE_HALF_ITEM_SPACING);
	BLayoutBuilder::Group<>(row)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.Add(attributeField)
		.Add(operatorField)
		.Add(valueText,2.0f);
	compare->AddChild(row);

	BButton	*ok	= new BButton("okButton",B_TRANSLATE("OK"),new BMessage(kOK));
	BLayoutBuilder::Group<>(this,B_VERTICAL)
		.SetInsets(B_USE_WINDOW_SPACING)
		.Add(searchText)
		.Add(compare)
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(new BButton("cancelButton",B_TRANSLATE("Cancel"),new BMessage(kCancel)))
			.Add(ok)
		.End();
	SetDefaultButton(ok);
	searchText->MakeFocus(true);
}


BString FindWindow::MarkedValue(BMenuField *field, const char *name)
{
	BMenuItem	*marked	= field->Menu()->FindMarked();
	const char	*value	= "";
	if ((marked != NULL) && (marked->Message() != NULL))
		marked->Message()->FindString(name,&value);
	return BString(value);
}


void FindWindow::SendSearch(bool shadow)
{
	//**Use a small trick an tell the PDocument that the menu was invoked but with a given searchstring
	BMessage	searchMessage(MENU_SEARCH_FIND);
	if (useCriterion) {
		searchMessage.AddString(P_C_FIND_ATTRIBUTE,MarkedValue(attributeField,P_C_FIND_ATTRIBUTE));
		searchMessage.AddString(P_C_FIND_OPERATOR,MarkedValue(operatorField,P_C_FIND_OPERATOR));
		searchMessage.AddString(P_C_FIND_VALUE,valueText->Text());
	} else
		searchMessage.AddString("searchString",searchText->Text());
	if (shadow)
		searchMessage.AddBool("shadow",true);
	BMessenger(doc).SendMessage(&searchMessage);
}


void FindWindow::MessageReceived(BMessage *message)
{
	TRACE();
	switch (message->what) {
		case kOK:
			SendSearch(false);
			Quit();
			break;
		case kCancel:
			Quit();
			break;
		case kTextChanged:
			useCriterion	= false;
			SendSearch(true);
			break;
		case kCriterionChanged:
			useCriterion	= true;
			SendSearch(true);
			break;
		default:
			BWindow::MessageReceived(message);
			break;
	}
}
