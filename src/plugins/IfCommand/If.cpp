#include "If.h"
#include "NodeSearch.h"
#include "PCommandManager.h"
#include "ProjectConceptorDefs.h"


If::If():PCommand()
{
}

static const char* const kIfScopeNodes			= "nodes";
static const char* const kIfScopeConnections	= "connections";
static const char* const kIfScopeBoth			= "both";

static const property_info kIfProperties[] = {
	{ "If", { B_EXECUTE_PROPERTY, 0 }, { B_DIRECT_SPECIFIER, 0 },
		"Runs its subPCommand children only if searchString is found - "
		"scope: nodes/connections/both (default nodes), same semantics as "
		"Find. Never changes the selection itself, unlike Find.", 0, {0},
		{ { { {"searchString", B_STRING_TYPE}, {"scope", B_STRING_TYPE} } } } },
};

const property_info* If::PropertyInfo(int32 *count)
{
	*count	= 1;
	return kIfProperties;
}

BMessage* If::Do(PDocument *doc, BMessage *settings)
{
	BString	searchString;
	settings->FindString("searchString",&searchString);
	// see Find.cpp's own note on this exact pattern: FindString(name,
	// BString*) clobbers its output to "" even on failure, so the default
	// has to be re-applied explicitly on failure, not just pre-set
	BString	scope(kIfScopeNodes);
	if (settings->FindString("scope",&scope) != B_OK)
		scope	= kIfScopeNodes;

	bool	matched	= false;
	if (searchString.Length() > 0) {
		if ((scope == kIfScopeNodes) || (scope == kIfScopeBoth)) {
			BList	*matches	= FindMatchingNodes(doc->GetAllNodes(),searchString);
			matched	= matched || (matches->CountItems() > 0);
			delete matches;
		}
		if (!matched && ((scope == kIfScopeConnections) || (scope == kIfScopeBoth))) {
			BList	*matches	= FindMatchingNodes(doc->GetAllConnections(),searchString);
			matched	= matched || (matches->CountItems() > 0);
			delete matches;
		}
	}

	BMessage	*executed	= new BMessage();
	if (matched) {
		BMessage	*record	= RunSubCommandsOnce(doc,settings);
		*executed	= *record;
		delete record;
	}
	settings->RemoveName("If::matched");
	settings->AddBool("If::matched",matched);
	settings->RemoveName("If::executed");
	settings->AddMessage("If::executed",executed);
	doc->SetModified();
	return settings;
}

void If::Undo(PDocument *doc,BMessage *undo)
{
	// see Repeat::Undo()'s own comment - not calling PCommand::Undo() here
	// for the same reason: "PCommand::subPCommand" is the child template,
	// only ever actually run (if at all) via RunSubCommandsOnce()'s own
	// fresh copy, recorded separately under "If::executed".
	bool	matched	= false;
	if ((undo->FindBool("If::matched",&matched) != B_OK) || !matched) {
		doc->SetModified();
		return;
	}
	BMessage	executed;
	if (undo->FindMessage("If::executed",&executed) == B_OK) {
		// reverse order - see Repeat::Undo()'s own comment for why (a
		// later child's own undo state can be captured relative to what
		// an earlier one left behind, same as Repeat's iterations)
		type_code	infoType;
		int32		subCount	= 0;
		executed.GetInfo("PCommand::subPCommand",&infoType,&subCount);
		for (int32 j = subCount-1; j >= 0; j--) {
			BMessage	subEntry;
			if (executed.FindMessage("PCommand::subPCommand",j,&subEntry) != B_OK)
				continue;
			char	*subName	= NULL;
			subEntry.FindString("Command::Name",(const char **)&subName);
			PCommand	*subCommand	= manager->GetPCommand(subName);
			if (subCommand != NULL)
				subCommand->Undo(doc,&subEntry);
		}
	}
	doc->SetModified();
}



void If::AttachedToManager(void)
{
}

void If::DetachedFromManager(void)
{
}
