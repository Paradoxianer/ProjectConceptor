#include "NodeSearch.h"


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
