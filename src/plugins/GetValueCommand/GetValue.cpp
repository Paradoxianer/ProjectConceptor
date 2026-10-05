#include "GetValue.h"
#include "PCommandManager.h"
#include "ProjectConceptorDefs.h"

#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "GetValue"


GetValue::GetValue():PCommand()
{
}

static const property_info kGetValueProperties[] = {
	{ "GetValue", { B_EXECUTE_PROPERTY, 0 }, { B_DIRECT_SPECIFIER, 0 },
		"Reads one attribute of \"node\" (valueContainer: name, subgroup, "
		"index - like ChangeValue) into \"resultVariable\". Numbers become "
		"float, text string, bools bool. A missing attribute stores "
		"\"default\" if given, otherwise it's an error.", 0, {0},
		{ { { {"node", B_POINTER_TYPE}, {"valueContainer", B_MESSAGE_TYPE},
			  {"resultVariable", B_STRING_TYPE}, {"default", B_FLOAT_TYPE},
			  {"included_node", B_MESSAGE_TYPE} } } } },
};

const property_info* GetValue::PropertyInfo(int32 *count)
{
	*count	= 1;
	return kGetValueProperties;
}

static BString NodeLabel(BMessage *node)
{
	BMessage	data;
	const char	*name	= NULL;
	if ((node->FindMessage(P_C_NODE_DATA,&data) == B_OK)
			&& (data.FindString(P_C_NODE_NAME,&name) == B_OK))
		return BString(name);
	return BString("?");
}

BMessage* GetValue::Do(PDocument *doc, BMessage *settings)
{
	BMessage	*context	= manager->GetValueContext();
	BMessage	*node		= NULL;
	BMessage	valueContainer;
	BString		resultVariable;
	const char	*name		= NULL;
	int32		index		= 0;
	settings->FindPointer("node",(void**)&node);
	settings->FindMessage("valueContainer",&valueContainer);
	settings->FindString("resultVariable",&resultVariable);
	valueContainer.FindString("name",&name);
	valueContainer.FindInt32("index",&index);

	BString	error;
	if ((node == NULL) || (name == NULL) || (resultVariable.Length() == 0))
		error	= B_TRANSLATE("GetValue needs node, valueContainer/name and resultVariable");
	if ((error.Length() > 0) || (context == NULL)) {
		if (error.Length() > 0)
			manager->AddPlaybackError(error);
		return PCommand::Do(doc,settings);
	}
	context->RemoveName(resultVariable.String());

	// same subgroup walk as ChangeValue (e.g. "Node::Data")
	BMessage	holder(*node);
	const char	*subgroup	= NULL;
	bool		found		= true;
	for (int32 i = 0; found && (valueContainer.FindString("subgroup",i,&subgroup) == B_OK); i++) {
		BMessage	inner;
		found	= (holder.FindMessage(subgroup,&inner) == B_OK);
		if (found)
			holder	= inner;
	}

	type_code	type	= B_ANY_TYPE;
	int32		count	= 0;
	if (!found || (holder.GetInfo(name,&type,&count) != B_OK) || (index >= count)) {
		float	fallback	= 0;
		if (settings->FindFloat("default",&fallback) == B_OK)
			context->AddFloat(resultVariable.String(),fallback);
		else {
			error.SetToFormat(B_TRANSLATE("GetValue: node \"%s\" has no attribute \"%s\""),
				NodeLabel(node).String(),name);
			manager->AddPlaybackError(error);
		}
		return PCommand::Do(doc,settings);
	}

	// an attribute added in the editor (GraphEditor's G_E_ADD_ATTRIBUTE) is
	// a block {Name, Value} - read its Value, as the editor shows it
	BMessage	attribute;
	if ((type == B_MESSAGE_TYPE) && (holder.FindMessage(name,index,&attribute) == B_OK)
			&& (attribute.GetInfo("Value",&type,&count) == B_OK)) {
		holder	= attribute;
		name	= "Value";
		index	= 0;
	}

	float	number	= 0;
	bool	isNumber	= true;
	switch (type) {
		case B_INT8_TYPE:	{ int8 v; holder.FindInt8(name,index,&v); number = v; break; }
		case B_INT16_TYPE:	{ int16 v; holder.FindInt16(name,index,&v); number = v; break; }
		case B_INT32_TYPE:	{ int32 v; holder.FindInt32(name,index,&v); number = v; break; }
		case B_INT64_TYPE:	{ int64 v; holder.FindInt64(name,index,&v); number = v; break; }
		case B_FLOAT_TYPE:	{ holder.FindFloat(name,index,&number); break; }
		case B_DOUBLE_TYPE:	{ double v; holder.FindDouble(name,index,&v); number = v; break; }
		default:			isNumber	= false;
	}
	if (isNumber)
		context->AddFloat(resultVariable.String(),number);
	else if (type == B_STRING_TYPE) {
		const char	*text	= NULL;
		holder.FindString(name,index,&text);
		context->AddString(resultVariable.String(),text);
	} else if (type == B_BOOL_TYPE) {
		bool	value	= false;
		holder.FindBool(name,index,&value);
		context->AddBool(resultVariable.String(),value);
	} else {
		error.SetToFormat(B_TRANSLATE("GetValue: \"%s\" of node \"%s\" is not a number, text or bool"),
			name,NodeLabel(node).String());
		manager->AddPlaybackError(error);
	}
	return PCommand::Do(doc,settings);
}

void GetValue::Undo(PDocument *doc,BMessage *undo)
{
	// nothing to do - only the value context changed, see the class comment
}

void GetValue::AttachedToManager(void)
{
}

void GetValue::DetachedFromManager(void)
{
}
