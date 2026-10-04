#include "ProjectConceptorDefs.h"
#include "Insert.h"

#include <support/TypeConstants.h>

#include <map>

#include "PCommandManager.h"

Insert::Insert():PCommand() {
}

static const property_info kInsertProperties[] = {
	// included_node: Indexer::IndexCommand() embeds the full node content
	// here the first time a "node" pointer is seen while recording a
	// macro (true for every command with a "node" field, not just
	// Insert - but Insert's node is typically brand new, so it almost
	// always carries one). See MacroText.h for how this round-trips.
	{ "Insert", { B_EXECUTE_PROPERTY, 0 }, { B_DIRECT_SPECIFIER, 0 },
		"Inserts one or more nodes (repeated \"node\" pointers) - "
		"resultVariable (optional) saves the inserted node(s) under that "
		"name in the macro's value context, e.g. to reposition or select "
		"them right after in the same Repeat/ForEach iteration.", 0, {0},
		{ { { {"node", B_POINTER_TYPE}, {"included_node", B_MESSAGE_TYPE},
			  {"resultVariable", B_STRING_TYPE} } } } },
};

const property_info* Insert::PropertyInfo(int32 *count) {
	*count	= 1;
	return kInsertProperties;
}

void Insert::Undo(PDocument *doc,BMessage *undo) {
	BList			*allConnectinos		= doc->GetAllConnections();
	BList			*allNodes			= doc->GetAllNodes();
	set<BMessage*>		*changed			= doc->GetChangedNodes();
	BMessage		*node				= new BMessage();
	int32			i					= 0;
	PCommand::Undo(doc,undo);
	// mirrors Do(): a node's parent lives on the node itself
	// (P_C_NODE_PARENT), not on the command wrapper - see there.
	while (undo->FindPointer("node",i,(void **)&node) == B_OK){
		if (node!=NULL) {
			if (node->what != P_C_CONNECTION_TYPE){
				allNodes->RemoveItem(node);
				BMessage	*parentNode			= NULL;
				BList		*parentAllNodes		= NULL;
				if ((node->FindPointer(P_C_NODE_PARENT, (void **)&parentNode) == B_OK) && (parentNode != NULL)) {
					if ((parentNode->FindPointer(P_C_NODE_ALLNODES, (void **)&parentAllNodes) == B_OK) && (parentAllNodes))
						parentAllNodes->RemoveItem(node);
				}
			}
			else
				allConnectinos->RemoveItem(node);
			changed->insert(node);
		}
		i++;
	}
	doc->SetModified();
}

void Insert::ReplaceInterpolatedNodes(BMessage *settings) {
	if ((manager == NULL) || (manager->GetValueContext() == NULL))
		return;
	// a node with "${...}" placeholders goes in as a filled-in copy, never
	// the template itself - inside a Repeat the template is reused for the
	// next pass and must keep its placeholders
	std::map<BMessage*,BMessage*>	replaced;
	BMessage	*node	= NULL;
	for (int32 i = 0; settings->FindPointer("node",i,(void **)&node) == B_OK; i++) {
		if ((node->what == P_C_CONNECTION_TYPE) || !manager->HasInterpolation(node))
			continue;
		BMessage	*copy	= new BMessage(*node);
		manager->InterpolateStrings(copy);
		settings->ReplacePointer("node",i,copy);
		manager->RepointReplayNode(node,copy);
		replaced[node]	= copy;
	}
	// a connection in this same Insert must point at the copies, not at
	// templates that never reach the document - and so becomes a copy too
	for (int32 i = 0; settings->FindPointer("node",i,(void **)&node) == B_OK; i++) {
		if (node->what != P_C_CONNECTION_TYPE)
			continue;
		BMessage	*from	= NULL;
		BMessage	*to		= NULL;
		node->FindPointer(P_C_NODE_CONNECTION_FROM,(void **)&from);
		node->FindPointer(P_C_NODE_CONNECTION_TO,(void **)&to);
		std::map<BMessage*,BMessage*>::iterator	newFrom	= replaced.find(from);
		std::map<BMessage*,BMessage*>::iterator	newTo	= replaced.find(to);
		bool	hasText	= manager->HasInterpolation(node);
		if ((newFrom == replaced.end()) && (newTo == replaced.end()) && !hasText)
			continue;
		BMessage	*copy	= new BMessage(*node);
		if (newFrom != replaced.end())
			copy->ReplacePointer(P_C_NODE_CONNECTION_FROM,newFrom->second);
		if (newTo != replaced.end())
			copy->ReplacePointer(P_C_NODE_CONNECTION_TO,newTo->second);
		if (hasText)
			manager->InterpolateStrings(copy);
		settings->ReplacePointer("node",i,copy);
		manager->RepointReplayNode(node,copy);
	}
}

BMessage* Insert::Do(PDocument *doc, BMessage *settings) {
	TRACE();
	BMessage		*node				= NULL;
	set<BMessage*>	*changed			= doc->GetChangedNodes();
	BList			*allConnections		= doc->GetAllConnections();
	BList			*allNodes			= doc->GetAllNodes();
	int32			i					= 0;
	status_t		err					= B_OK;
	BString			resultVariable;
	bool			hasResultVariable	= (settings->FindString("resultVariable",&resultVariable) == B_OK)
		&& (resultVariable.Length() > 0);
	if (hasResultVariable && (manager->GetValueContext() != NULL))
		manager->GetValueContext()->RemoveName(resultVariable.String());
	ReplaceInterpolatedNodes(settings);
	while ((err=settings->FindPointer("node",i,(void **)&node)) == B_OK) {
		if ((node->what != P_C_CONNECTION_TYPE) && allNodes->HasItem(node)) {
			// the same command run again (Repeat/ForEach around an Insert)
			// carries the same node object - inserting it twice would put
			// one node in the graph twice instead of creating a new one
			BMessage	*copy	= new BMessage(*node);
			settings->ReplacePointer("node",i,copy);
			node	= copy;
		}
		if (node->what != P_C_CONNECTION_TYPE) {
			allNodes->AddItem(node);
			// a node's intended parent (e.g. set by GroupRenderer when
			// double-clicking a group to insert a child) lives on the node
			// itself, not on this command's wrapper message - each inserted
			// node can have its own parent, so this has to be looked up per
			// node, not once for the whole batch (was issue #36: the old
			// lookup on `settings` never matched anything a caller actually
			// set, so new children silently ended up as top-level siblings
			// instead of being registered in their parent's node list)
			BMessage	*parentNode			= NULL;
			BList		*parentAllNodes		= NULL;
			if ((node->FindPointer(P_C_NODE_PARENT, (void **)&parentNode) == B_OK) && (parentNode != NULL)) {
				if (parentNode->FindPointer(P_C_NODE_ALLNODES, (void **)&parentAllNodes) != B_OK) {
					parentAllNodes = new BList();
					parentNode->AddPointer(P_C_NODE_ALLNODES, parentAllNodes);
				}
				parentAllNodes->AddItem(node);
			}
		}
		else
			allConnections->AddItem(node);
		//recalc size
		//**check if there is a passed "docRect"
		BRect	insertFrame		= BRect(0,0,0,0);
		if (node->FindRect(P_C_NODE_FRAME,&insertFrame)==B_OK) {
			BRect	docRect			= doc->Bounds();
			if (insertFrame.bottom >= docRect.Height())
				docRect.bottom= insertFrame.bottom+20;
			if (insertFrame.right >= docRect.Width())
				docRect.right = insertFrame.right+20;
			if (docRect != doc->Bounds())
				doc->Resize(docRect.right,docRect.bottom);
		}
		i++;
		changed->insert(node);
		if (hasResultVariable && (manager->GetValueContext() != NULL))
			manager->GetValueContext()->AddPointer(resultVariable.String(),node);
	}
	doc->SetModified();
	settings = PCommand::Do(doc,settings);
	return settings;
}



void Insert::AttachedToManager(void) {
}

void Insert::DetachedFromManager(void) {
}
