#include "ForEach.h"
#include "PCommandManager.h"
#include "ProjectConceptorDefs.h"


ForEach::ForEach():PCommand()
{
}

static const property_info kForEachProperties[] = {
	{ "ForEach", { B_EXECUTE_PROPERTY, 0 }, { B_DIRECT_SPECIFIER, 0 },
		"Runs its subPCommand children once per currently selected node. "
		"nodeVariable (optional): publishes the current node under this "
		"name in the macro's value context, for a child field bound to "
		"\"$<nodeVariable>\".", 0, {0},
		{ { { {"nodeVariable", B_STRING_TYPE} } } } },
};

const property_info* ForEach::PropertyInfo(int32 *count)
{
	*count	= 1;
	return kForEachProperties;
}

BMessage* ForEach::Do(PDocument *doc, BMessage *settings)
{
	BString	nodeVariable;
	bool	hasNodeVariable	= (settings->FindString("nodeVariable",&nodeVariable) == B_OK);

	// a snapshot, not doc->GetSelected() itself - a child command run
	// below could change the selection (or delete a node outright), and
	// this loop's own cardinality has to stay fixed at "however many
	// nodes were selected when this began", not shrink/grow mid-run.
	BList	snapshot;
	BList	*selected	= doc->GetSelected();
	for (int32 i=0;i<selected->CountItems();i++)
		snapshot.AddItem(selected->ItemAt(i));

	BMessage	*executed	= new BMessage();
	for (int32 n=0; n<snapshot.CountItems(); n++) {
		BMessage	*node	= (BMessage*)snapshot.ItemAt(n);
		if (hasNodeVariable && (manager->GetValueContext() != NULL)) {
			manager->GetValueContext()->RemoveName(nodeVariable.String());
			manager->GetValueContext()->AddPointer(nodeVariable.String(),node);
		}
		BMessage	*iterationRecord	= RunSubCommandsOnce(doc,settings);
		executed->AddMessage("iteration",iterationRecord);
	}

	settings->RemoveName("ForEach::executed");
	settings->AddMessage("ForEach::executed",executed);
	doc->SetModified();
	return settings;
}

void ForEach::Undo(PDocument *doc,BMessage *undo)
{
	// see Repeat::Undo()'s own comment for why PCommand::Undo(doc,undo)
	// is deliberately not called here - the same reasoning applies:
	// "PCommand::subPCommand" here is the loop body template, never run
	// directly itself, only fresh per-iteration copies were.
	BMessage	executed;
	if (undo->FindMessage("ForEach::executed",&executed) != B_OK)
		return;
	// reverse order - see Repeat::Undo()'s own comment for why
	type_code	infoType;
	int32		iterationCount	= 0;
	executed.GetInfo("iteration",&infoType,&iterationCount);
	for (int32 i = iterationCount-1; i >= 0; i--) {
		BMessage	iteration;
		if (executed.FindMessage("iteration",i,&iteration) != B_OK)
			continue;
		int32	subCount	= 0;
		iteration.GetInfo("PCommand::subPCommand",&infoType,&subCount);
		for (int32 j = subCount-1; j >= 0; j--) {
			BMessage	subEntry;
			if (iteration.FindMessage("PCommand::subPCommand",j,&subEntry) != B_OK)
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



void ForEach::AttachedToManager(void)
{
}

void ForEach::DetachedFromManager(void)
{
}
