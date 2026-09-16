#ifndef LAYOUT_H
#define LAYOUT_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "PCommand.h"
#include "PDocument.h"

/**
 * @class Layout
 *
 * @brief Runs automatic layout as a normal PCommand (#55) - reuses the
 * exact algorithm/undo construction LayoutEditor's toolbar button already
 * used (see LayoutCommandBuilder.h), but as a registered command it's now
 * also scriptable (hey) and macro-recordable/DSL-editable, not just
 * reachable from that one toolbar button.
 *
 * Do() can run non-interactively (macro replay, a hey script) - on
 * failure it never pops a BAlert, it adds an "error" string field to its
 * own settings and returns with no subPCommand children (a safe no-op).
 * LayoutEditor's toolbar button is the interactive caller that still
 * shows that error to a user watching (see LayoutEditor::ApplyLayout()).
 */
class Layout : public PCommand
{

public:
							Layout();

	//++++++++++++++++PCommand
	virtual	BMessage*		Do(PDocument *doc, BMessage *settings);
	// Undo() intentionally not overridden - the appended ChangeValue
	// subPCommand children are undone generically by PCommand::Undo(),
	// same as Batch.
	virtual	char*			Name(void){return "Layout";};
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);
	virtual	const property_info	*PropertyInfo(int32 *count);

protected:
	//----------------PCommand
};
#endif
