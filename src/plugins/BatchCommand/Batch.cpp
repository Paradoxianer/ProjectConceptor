#include "Batch.h"


Batch::Batch():PCommand()
{
}

// No top-level fields of its own - its whole content is nested
// "PCommand::subPCommand" children, already handled generically by
// PCommand::Do()/Undo(). Still registered (not *count=0) so "Batch" stays
// executable/DSL-writable as a plain container.
static const property_info kBatchProperties[] = {
	{ "Batch", { B_EXECUTE_PROPERTY, 0 }, { B_DIRECT_SPECIFIER, 0 },
		"Runs its nested subPCommand children as one undo step.", 0, {0}, {} },
};

const property_info* Batch::PropertyInfo(int32 *count)
{
	*count	= 1;
	return kBatchProperties;
}

void Batch::AttachedToManager(void) {
}

void Batch::DetachedFromManager(void) {
}
