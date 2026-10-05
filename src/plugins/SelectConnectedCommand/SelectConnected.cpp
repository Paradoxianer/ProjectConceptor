#include "SelectConnected.h"
#include "PCommandManager.h"
#include "ProjectConceptorDefs.h"

#include <Catalog.h>

#include <set>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "SelectConnected"


SelectConnected::SelectConnected():PCommand()
{
}

static const property_info kSelectConnectedProperties[] = {
	{ "SelectConnected", { B_EXECUTE_PROPERTY, 0 }, { B_DIRECT_SPECIFIER, 0 },
		"Adds every node reachable from the current selection over "
		"connections. direction: both (default), outgoing or incoming. "
		"depth: number of hops, 0 (default) = all.", 0, {0},
		{ { { {"direction", B_STRING_TYPE}, {"depth", B_INT32_TYPE} } } } },
};

const property_info* SelectConnected::PropertyInfo(int32 *count)
{
	*count	= 1;
	return kSelectConnectedProperties;
}

BMessage* SelectConnected::Do(PDocument *doc, BMessage *settings)
{
	// FindString() empties the BString even when the field is missing
	const char	*directionText	= NULL;
	BString		direction((settings->FindString("direction",&directionText) == B_OK)
					? directionText : "both");
	int32		depth	= 0;
	settings->FindInt32("depth",&depth);
	bool	outgoing	= (direction == "both") || (direction == "outgoing");
	bool	incoming	= (direction == "both") || (direction == "incoming");
	if (!outgoing && !incoming) {
		BString	error;
		error.SetToFormat(B_TRANSLATE("SelectConnected: unknown direction \"%s\" (both, outgoing, incoming)"),
			direction.String());
		manager->AddPlaybackError(error);
		return PCommand::Do(doc,settings);
	}

	BList				*selected		= doc->GetSelected();
	BList				*connections	= doc->GetAllConnections();
	BMessage			undoMessage;
	std::set<BMessage*>	reached;
	BList				frontier;
	for (int32 i = 0; i < selected->CountItems(); i++) {
		BMessage	*node	= (BMessage*)selected->ItemAt(i);
		undoMessage.AddPointer("node",node);
		if ((node != NULL) && (node->what != P_C_CONNECTION_TYPE)) {
			reached.insert(node);
			frontier.AddItem(node);
		}
	}

	// breadth-first, one hop per pass; "reached" makes a cycle harmless
	for (int32 hop = 0; (frontier.CountItems() > 0) && ((depth <= 0) || (hop < depth)); hop++) {
		BList	next;
		for (int32 c = 0; c < connections->CountItems(); c++) {
			BMessage	*connection	= (BMessage*)connections->ItemAt(c);
			BMessage	*from		= NULL;
			BMessage	*to			= NULL;
			connection->FindPointer(P_C_NODE_CONNECTION_FROM,(void**)&from);
			connection->FindPointer(P_C_NODE_CONNECTION_TO,(void**)&to);
			if (outgoing && (to != NULL) && frontier.HasItem(from) && (reached.count(to) == 0)) {
				reached.insert(to);
				next.AddItem(to);
			}
			if (incoming && (from != NULL) && frontier.HasItem(to) && (reached.count(from) == 0)) {
				reached.insert(from);
				next.AddItem(from);
			}
		}
		for (int32 i = 0; i < next.CountItems(); i++)
			MarkSelected(doc,(BMessage*)next.ItemAt(i));
		frontier.MakeEmpty();
		frontier.AddList(&next);
	}

	settings->RemoveName("SelectConnected::Undo");
	settings->AddMessage("SelectConnected::Undo",&undoMessage);
	settings	= PCommand::Do(doc,settings);
	doc->SetModified();
	return settings;
}

void SelectConnected::Undo(PDocument *doc,BMessage *undo)
{
	PCommand::Undo(doc,undo);
	BList		*selected	= doc->GetSelected();
	BMessage	undoMessage;
	undo->FindMessage("SelectConnected::Undo",&undoMessage);
	while (selected->CountItems() > 0) {
		BMessage	*node	= (BMessage*)selected->RemoveItem((int32)0);
		node->ReplaceBool(P_C_NODE_SELECTED,0,false);
		doc->GetChangedNodes()->insert(node);
	}
	BMessage	*node	= NULL;
	for (int32 i = 0; undoMessage.FindPointer("node",i,(void**)&node) == B_OK; i++)
		MarkSelected(doc,node);
	doc->SetModified();
}

// same as Select::DoSelect(): a node without the flag yet gets it added
void SelectConnected::MarkSelected(PDocument *doc, BMessage *node)
{
	bool	wasSelected	= false;
	if (node->FindBool(P_C_NODE_SELECTED,&wasSelected) == B_OK)
		node->ReplaceBool(P_C_NODE_SELECTED,0,true);
	else
		node->AddBool(P_C_NODE_SELECTED,true);
	if (!doc->GetSelected()->HasItem(node))
		doc->GetSelected()->AddItem(node);
	doc->GetChangedNodes()->insert(node);
}

void SelectConnected::AttachedToManager(void)
{
}

void SelectConnected::DetachedFromManager(void)
{
}
