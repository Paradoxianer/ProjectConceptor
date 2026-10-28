#include "NodeSearch.h"

#include <string.h>

#include "AttributeText.h"
#include "ProjectConceptorDefs.h"


bool NodeMatchesSearch(BMessage *node, const BString &search)
{
	char		*attribName		= NULL;
	BMessage	attribMessage;
	BString		dataString;
	uint32		type			= B_ANY_TYPE;
	int32		count			= 0;
	bool		found			= false;
	int32		i				= 0;
	// first iterate through all Strings
	while ((node->GetInfo(B_STRING_TYPE,i,(char **)&attribName,&type,&count) == B_OK) && !found) {
		if (node->FindString(attribName,count-1,&dataString) == B_OK)
			found = dataString.FindFirst(search) != B_ERROR;
		i++;
	}
	// an attribute that isn't text is found by its value as text (#134)
	type_code	valueType	= B_ANY_TYPE;
	if (!found && (node->GetInfo(P_C_ATTRIBUTE_VALUE,&valueType) == B_OK)
			&& (valueType != B_STRING_TYPE)) {
		BString	text;
		if (AttributeValueText(*node,&text) == B_OK)
			found	= text.FindFirst(search) != B_ERROR;
	}
	// check all subnodes / sub bmessages
	i = 0;
	while ((node->GetInfo(B_MESSAGE_TYPE,i,(char **)&attribName,&type,&count) == B_OK) && !found) {
		if (node->FindMessage(attribName,count-1,&attribMessage) == B_OK)
			found = NodeMatchesSearch(&attribMessage,search);
		i++;
	}
	return found;
}


BList* FindMatchingNodes(BList *all, const BString &search)
{
	BList		*matches	= new BList();
	BMessage	*node		= NULL;
	int32		i			= 0;
	for (i=0; i<all->CountItems(); i++) {
		node = (BMessage *)all->ItemAt(i);
		if (NodeMatchesSearch(node,search))
			matches->AddItem(node);
	}
	return matches;
}


static const char* const kOperators[]	= { "==", "!=", "<", "<=", ">", ">=", "contains" };


status_t ReadSearchCriterion(const BMessage &settings, SearchCriterion *criterion)
{
	const char	*op	= NULL;
	if (settings.FindString(P_C_FIND_OPERATOR,&op) != B_OK)
		return B_NAME_NOT_FOUND;
	criterion->op	= op;
	const char	*attribute	= "";
	const char	*value		= "";
	settings.FindString(P_C_FIND_ATTRIBUTE,&attribute);
	settings.FindString(P_C_FIND_VALUE,&value);
	criterion->attribute	= attribute;
	criterion->value		= value;
	for (uint32 i = 0; i < sizeof(kOperators) / sizeof(kOperators[0]); i++) {
		if (criterion->op == kOperators[i])
			return B_OK;
	}
	return B_BAD_VALUE;
}


/** -1, 0 or 1 as a < b, a == b, a > b */
template<typename T>
static int Order(T a, T b)
{
	return (a < b) ? -1 : ((b < a) ? 1 : 0);
}


static bool ParseBool(const BString &text, bool *value)
{
	if ((text.ICompare("true") == 0) || (text.ICompare("yes") == 0) || (text == "1"))
		*value	= true;
	else if ((text.ICompare("false") == 0) || (text.ICompare("no") == 0) || (text == "0"))
		*value	= false;
	else
		return false;
	return true;
}


/** a number and nothing after it */
static bool ParsePlainNumber(const BString &text, float *number)
{
	BString	unit;
	return (ParseNumberWithUnit(text.String(),number,&unit) == B_OK)
		&& (unit.Length() == 0);
}


static bool ValueMatches(const BMessage &attribute, const SearchCriterion &criterion)
{
	BString	text;
	if (AttributeValueText(attribute,&text) != B_OK)
		return false;
	if (criterion.op == "contains")
		return text.IFindFirst(criterion.value) != B_ERROR;

	type_code	type	= B_ANY_TYPE;
	attribute.GetInfo(P_C_ATTRIBUTE_VALUE,&type);
	int		order	= 0;
	float	left	= 0;
	float	right	= 0;
	if (type == B_BOOL_TYPE) {
		bool	mine	= false;
		bool	wanted	= false;
		attribute.FindBool(P_C_ATTRIBUTE_VALUE,&mine);
		if (!ParseBool(criterion.value,&wanted))
			return false;
		order	= Order((int)mine,(int)wanted);
	} else if (type == B_FLOAT_TYPE) {
		attribute.FindFloat(P_C_ATTRIBUTE_VALUE,&left);
		if (!ParsePlainNumber(criterion.value,&right)) {
			// "25 €" asks for the number, the unit is what it is
			BString	unit;
			if (ParseNumberWithUnit(criterion.value.String(),&right,&unit) != B_OK)
				return false;
		}
		order	= Order(left,right);
	} else if (ParsePlainNumber(text,&left) && ParsePlainNumber(criterion.value,&right))
		order	= Order(left,right);
	else {
		int	compared	= text.ICompare(criterion.value);
		order	= (compared < 0) ? -1 : ((compared > 0) ? 1 : 0);
	}

	if (criterion.op == "==")	return order == 0;
	if (criterion.op == "!=")	return order != 0;
	if (criterion.op == "<")	return order < 0;
	if (criterion.op == "<=")	return order <= 0;
	if (criterion.op == ">")	return order > 0;
	if (criterion.op == ">=")	return order >= 0;
	return false;
}


bool NodeMatchesCriterion(BMessage *node, const SearchCriterion &criterion)
{
	BMessage	data;
	if (node->FindMessage(P_C_NODE_DATA,&data) != B_OK)
		return false;
	bool	any	= (criterion.attribute.Length() == 0);
	// the node's name, as if it were an attribute called "name"
	const char	*name	= NULL;
	if ((any || (criterion.attribute.ICompare("name") == 0))
			&& (data.FindString(P_C_NODE_NAME,&name) == B_OK)) {
		BMessage	asAttribute(B_STRING_TYPE);
		asAttribute.AddString(P_C_ATTRIBUTE_VALUE,name);
		if (ValueMatches(asAttribute,criterion))
			return true;
	}
	char		*key	= NULL;
	type_code	type	= B_ANY_TYPE;
	int32		count	= 0;
	for (int32 i = 0; data.GetInfo(B_MESSAGE_TYPE,i,&key,&type,&count) == B_OK; i++) {
		for (int32 entry = 0; entry < count; entry++) {
			BMessage	attribute;
			const char	*attributeName	= NULL;
			if ((data.FindMessage(key,entry,&attribute) != B_OK)
					|| (attribute.FindString(P_C_ATTRIBUTE_NAME,&attributeName) != B_OK))
				continue;
			if (!any && (criterion.attribute.ICompare(attributeName) != 0))
				continue;
			if (ValueMatches(attribute,criterion))
				return true;
		}
	}
	return false;
}


BList* FindNodesMatchingCriterion(BList *all, const SearchCriterion &criterion)
{
	BList	*matches	= new BList();
	for (int32 i = 0; i < all->CountItems(); i++) {
		BMessage	*node	= (BMessage *)all->ItemAt(i);
		if (NodeMatchesCriterion(node,criterion))
			matches->AddItem(node);
	}
	return matches;
}
