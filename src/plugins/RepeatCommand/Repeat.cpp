#include "Repeat.h"
#include "PCommandManager.h"
#include "ProjectConceptorDefs.h"


Repeat::Repeat():PCommand()
{
}

static const property_info kRepeatProperties[] = {
	{ "Repeat", { B_EXECUTE_PROPERTY, 0 }, { B_DIRECT_SPECIFIER, 0 },
		"Runs its subPCommand children \"count\" times. counterVariable "
		"(optional): publishes the current 0-based iteration index under "
		"this name in the macro's value context, for a child field bound "
		"to \"$<counterVariable>\".", 0, {0},
		{ { { {"count", B_INT32_TYPE}, {"counterVariable", B_STRING_TYPE} } } } },
};

const property_info* Repeat::PropertyInfo(int32 *count)
{
	*count	= 1;
	return kRepeatProperties;
}

BMessage* Repeat::Do(PDocument *doc, BMessage *settings)
{
	int32	count	= 0;
	if (settings->FindInt32("count",&count) != B_OK)
		count	= 0;
	BString	counterVariable;
	bool	hasCounterVariable	= (settings->FindString("counterVariable",&counterVariable) == B_OK);

	BMessage	*executed	= new BMessage();
	for (int32 iteration=0; iteration<count; iteration++) {
		if (hasCounterVariable && (manager->GetValueContext() != NULL)) {
			manager->GetValueContext()->RemoveName(counterVariable.String());
			manager->GetValueContext()->AddInt32(counterVariable.String(),iteration);
		}
		BMessage	*iterationRecord	= RunSubCommandsOnce(doc,settings);
		executed->AddMessage("iteration",iterationRecord);
	}

	settings->RemoveName("Repeat::executed");
	settings->AddMessage("Repeat::executed",executed);
	doc->SetModified();
	return settings;
}

void Repeat::Undo(PDocument *doc,BMessage *undo)
{
	// deliberately does NOT call PCommand::Undo(doc,undo) - that base
	// implementation would try to undo undo's own "PCommand::subPCommand"
	// entries as if they were this command's own already-executed
	// children, but those are the loop body TEMPLATE (see Do()'s own
	// comment) - never run directly themselves, so they carry no real
	// undo data (each run was a fresh copy of the template instead, and
	// only those copies' post-Do() state, stored under "Repeat::executed",
	// actually has any). Walking the template here would either no-op
	// uselessly or, worse, misapply stale undo data left over from some
	// unrelated earlier use of the same template object.
	BMessage	executed;
	if (undo->FindMessage("Repeat::executed",&executed) != B_OK)
		return;
	// reverse order, both across iterations and within each one's own
	// subPCommand children - later iterations' own recorded "oldFrame"-
	// style undo state was captured relative to what the *previous*
	// iteration left behind (a real dependency chain, unlike independent
	// sibling subPCommands - see PCommand::Undo()'s own, deliberately
	// forward-order loop for that different case), so undoing anything
	// but last-run-first re-derives the wrong intermediate state. Same
	// principle PCommandManager::Undo() itself already uses for undoList.
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



void Repeat::AttachedToManager(void)
{
}

void Repeat::DetachedFromManager(void)
{
}
