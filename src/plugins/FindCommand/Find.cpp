#include <String.h>

#include "Find.h"
#include "FindWindow.h"
#include "ProjectConceptorDefs.h"


Find::Find():PCommand()
{
}

// scope: which list(s) FindNodes() searches. "nodes" (default, matches the
// original, connections-blind behavior) / "connections" / "both".
static const char* const kFindScopeNodes			= "nodes";
static const char* const kFindScopeConnections		= "connections";
static const char* const kFindScopeBoth			= "both";

// setOperation: how the search result combines with whatever was already
// selected. "replace" (default, matches the original behavior - old
// selection is simply gone) / "add" (union) / "subtract" (remove matches
// from the current selection) / "intersect" (keep only what's in both) -
// #133: without this a multi-step selection ("all with X, but not Y") isn't
// expressible at all, every Find just throws the previous one away.
static const char* const kFindOpReplace			= "replace";
static const char* const kFindOpAdd				= "add";
static const char* const kFindOpSubtract			= "subtract";
static const char* const kFindOpIntersect			= "intersect";

static const property_info kFindProperties[] = {
	{ "Find", { B_EXECUTE_PROPERTY, 0 }, { B_DIRECT_SPECIFIER, 0 },
		"Selects nodes/connections containing searchString in a string "
		"field. scope: nodes/connections/both (default nodes). "
		"setOperation: replace/add/subtract/intersect against the current "
		"selection (default replace).", 0, {0},
		{ { { {"searchString", B_STRING_TYPE}, {"scope", B_STRING_TYPE},
			  {"setOperation", B_STRING_TYPE} } } } },
};

const property_info* Find::PropertyInfo(int32 *count)
{
	*count	= 1;
	return kFindProperties;
}

void Find::Undo(PDocument *doc,BMessage *undo)
{
	// was reading "node" pointers directly off `undo` (this command's own
	// top-level settings message) - Do() below never actually stored any
	// there (see its own history note), so this never restored anything;
	// the deselect loop that follows also never rebuilt doc->GetSelected()'s
	// own membership or notified GetChangedNodes(), just flipped the bool
	// flag on whatever was still sitting in the list. Undo() has therefore
	// been a no-op-that-looks-like-something-happened for as long as this
	// function has looked like this - fixed together with #133's rewrite of
	// Do(), which is what actually needed a working, restorable snapshot of
	// "what was selected before this ran" in the first place (setOperation
	// modes besides "replace" are meaningless to undo without one).
	PCommand::Undo(doc,undo);
	BMessage		undoMessage;
	undo->FindMessage("Find::Undo",&undoMessage);
	int32 			i					= 0;
	BMessage		*currentContainer	= NULL;
	BList			*selected			= doc->GetSelected();
	set<BMessage*>	*changed			= doc->GetChangedNodes();

	while (selected->CountItems()>0) {
		currentContainer	= (BMessage *)selected->RemoveItem((int32)0);
		if (currentContainer != NULL) {
			currentContainer->ReplaceBool(P_C_NODE_SELECTED,0,false);
			changed->insert(currentContainer);
		}
	}
	while (undoMessage.FindPointer("node",i,(void **)&currentContainer) == B_OK) {
		if (currentContainer != NULL) {
			currentContainer->ReplaceBool(P_C_NODE_SELECTED,0,true);
			selected->AddItem(currentContainer);
			changed->insert(currentContainer);
		}
		i++;
	}
	doc->SetModified();
}

BMessage* Find::Do(PDocument *doc, BMessage *settings)
{
	BString			findString;
	BList			*selected			= doc->GetSelected();
	set<BMessage*>	*changed			= doc->GetChangedNodes();
	BMessage		*undoMessage		= new BMessage();

	//if there is no findString we will show a window wich will generate a proper "searchString command on its own :)
	if (settings->FindString("searchString",&findString) != B_OK) {
		FindWindow *findWindow = new FindWindow(doc);
		findWindow->Show();
		return NULL;
	}

	// BMessage::FindString(name,BString*) clobbers its output to "" on
	// failure too (confirmed against Haiku's own Message.cpp - "Find*()
	// clobbers the object even on failure"), unlike FindBool()/FindInt32();
	// pre-setting these to their default and letting a failed Find leave
	// them alone (the pattern every other optional field in this codebase
	// uses) silently threw the default away and left scope/setOperation
	// empty - matching neither this nor any of the real option strings, so
	// a settings message with no "scope" field ended up searching nothing
	// at all rather than defaulting to nodes.
	BString	scope(kFindScopeNodes);
	if (settings->FindString("scope",&scope) != B_OK)
		scope = kFindScopeNodes;
	BString	setOperation(kFindOpReplace);
	if (settings->FindString("setOperation",&setOperation) != B_OK)
		setOperation = kFindOpReplace;

	// snapshotted before anything below mutates doc->GetSelected() -
	// Undo() needs the exact prior selection to restore, and every
	// setOperation below (add/subtract/intersect) needs it as the set to
	// combine the search result with, not the partially-drained list this
	// function is about to leave behind as it walks it.
	set<BMessage*>	previouslySelected;
	for (int32 i=0;i<selected->CountItems();i++)
		previouslySelected.insert((BMessage*)selected->ItemAt(i));

	BList	*foundList	= FindNodes(doc,&findString,scope);
	set<BMessage*>	found;
	for (int32 i=0;i<foundList->CountItems();i++)
		found.insert((BMessage*)foundList->ItemAt(i));
	delete foundList;

	set<BMessage*>	newSelection;
	if (setOperation == kFindOpAdd) {
		newSelection	= previouslySelected;
		newSelection.insert(found.begin(),found.end());
	} else if (setOperation == kFindOpSubtract) {
		newSelection	= previouslySelected;
		for (set<BMessage*>::iterator it=found.begin();it!=found.end();it++)
			newSelection.erase(*it);
	} else if (setOperation == kFindOpIntersect) {
		for (set<BMessage*>::iterator it=previouslySelected.begin();it!=previouslySelected.end();it++)
			if (found.find(*it) != found.end())
				newSelection.insert(*it);
	} else {
		// "replace", and the fallback for an unrecognized value - matches
		// the original, only-ever-supported behavior
		newSelection	= found;
	}

	for (set<BMessage*>::iterator it=previouslySelected.begin();it!=previouslySelected.end();it++)
		undoMessage->AddPointer("node",*it);

	selected->MakeEmpty();
	for (set<BMessage*>::iterator it=newSelection.begin();it!=newSelection.end();it++) {
		BMessage	*node	= *it;
		if (previouslySelected.find(node) == previouslySelected.end())
			changed->insert(node);
		node->ReplaceBool(P_C_NODE_SELECTED,0,true);
		selected->AddItem(node);
	}
	for (set<BMessage*>::iterator it=previouslySelected.begin();it!=previouslySelected.end();it++) {
		BMessage	*node	= *it;
		if (newSelection.find(node) == newSelection.end()) {
			node->ReplaceBool(P_C_NODE_SELECTED,0,false);
			changed->insert(node);
		}
	}

	settings->RemoveName("Find::Undo");
	settings->AddMessage("Find::Undo",undoMessage);
	settings	= PCommand::Do(doc,settings);
	doc->SetModified();
	return settings;
}

void Find::AttachedToManager(void)
{
}

void Find::DetachedFromManager(void)
{
}

BList* Find::FindNodes(PDocument *doc,BString *search,const BString &scope)
{
	BList		*nodesFound			= new BList();
	BMessage	*currentContainer	= NULL;
	int32		i					= 0;
	if ((scope == kFindScopeNodes) || (scope == kFindScopeBoth)) {
		BList	*all	= doc->GetAllNodes();
		for (i=0;i<all->CountItems();i++) {
			currentContainer =(BMessage *) all->ItemAt(i);
			if (FindInNode(currentContainer, search)== true)
				nodesFound->AddItem(currentContainer);
		}
	}
	if ((scope == kFindScopeConnections) || (scope == kFindScopeBoth)) {
		BList	*allConnections	= doc->GetAllConnections();
		for (i=0;i<allConnections->CountItems();i++) {
			currentContainer =(BMessage *) allConnections->ItemAt(i);
			if (FindInNode(currentContainer, search)== true)
				nodesFound->AddItem(currentContainer);
		}
	}
	return nodesFound;
}

bool Find::FindInNode(BMessage *node,BString *search)
{
	char		*attribName		= NULL;
	BMessage	*attribMessage	= new BMessage();
	BString		*dataString		= new BString();	
	uint32		type			= B_ANY_TYPE;
	int32		count			= 0;
	bool		found			= false;
	int32		i				= 0;
	//first iterate through all Strings
	while ((node->GetInfo(B_STRING_TYPE, i,(char **) &attribName, &type, &count) == B_OK) && !found)
	{
		if (node->FindString(attribName,count-1,dataString)==B_OK)
		{
			found = dataString->FindFirst(*search)!=B_ERROR;
		}
		i++;
	}
	//check all subnodes / sub bmessages
	i=0;
	while ((node->GetInfo(B_MESSAGE_TYPE, i,(char **) &attribName, &type, &count) == B_OK) && !found)
	{
		if ((node->FindMessage(attribName,count-1,attribMessage) == B_OK) && (attribMessage != NULL))
			found = FindInNode(attribMessage, search);
		i++;
	}
	return found;
}


/*
void Find::DoFind(PDocument *doc,BRect *rect)
{
	BList			*all				= doc->GetAllNodes();
	BList			*selected			= doc->GetSelected();
	set<BMessage*>		*changed			= doc->GetChangedNodes();
	BMessage		*currentContainer	= NULL;
	BRect			*frame				= new BRect(0,0,0,0);
	int32 			i					= 0;

	for (i=0;i<all->CountItems();i++)
	{
		currentContainer =(BMessage *) all->ItemAt(i);
		currentContainer->FindRect(P_C_NODE_FRAME,0,frame);
		if (rect->Contains(*frame))
		{
			currentContainer->ReplaceBool(P_C_NODE_SELECTED,0,true);
			selected->AddItem(currentContainer);
			changed->insert(currentContainer);
		}

	}
}

void Find::DoFind(PDocument *doc,BMessage *container)
{
	BList			*selected			= doc->GetSelected();
	set<BMessage*>		*changed			= doc->GetChangedNodes();
	bool			selectTester		= false;
	status_t		err					= B_OK;
	err= container->FindBool(P_C_NODE_SELECTED,&selectTester);
	if (err == B_OK)
		err = container->ReplaceBool(P_C_NODE_SELECTED,0,true);
	else
		err = container->AddBool(P_C_NODE_SELECTED,true);
	selected->AddItem(container);
	changed->insert(container);
	//container->(new BoolContainer(true));
}

void Find::DoFindAll(PDocument *doc)
{
	BList			*selected			= doc->GetSelected();
	BList			*all				= doc->GetAllNodes();
	set<BMessage*>		*changed			= doc->GetChangedNodes();

	BMessage		*currentContainer	= NULL;
	int32 			i					= 0;
	for (i=0;i<all->CountItems();i++)
	{
		currentContainer =(BMessage *) all->ItemAt(i);
		currentContainer->ReplaceBool(P_C_NODE_SELECTED,0,false);
		selected->AddItem(currentContainer);
		changed->insert(currentContainer);
	}
}
*/
