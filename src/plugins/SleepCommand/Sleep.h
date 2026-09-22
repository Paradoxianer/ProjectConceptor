#ifndef SLEEP_H
#define SLEEP_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "PCommand.h"
#include "PDocument.h"

/**
 * @class Sleep
 * @brief Pauses macro playback for "milliseconds" - a deliberate, macro-
 * author-placed pause between two other steps, e.g. to let whoever's
 * watching a macro play actually see what a preceding step did before the
 * next one runs, or to give a long-running automated macro a natural point
 * to pause at. "milliseconds" is a normal, $variable-bindable field like
 * any other command's (see PCommandManager::ResolveBindings()) - nothing
 * Sleep-specific needed for that.
 *
 * Do() releases the document lock for the actual wait, not just sits idle
 * while holding it - PCommandManager::Execute() holds that lock for its
 * whole Do() call, and PlayMacro()'s own existing comment already
 * documents that a step's own redraw broadcast needs that same lock to be
 * processed elsewhere (GraphEditor::ValueChanged()). A Sleep that held the
 * lock while waiting would keep the exact redraw it exists to make visible
 * from ever happening during the pause.
 */
class Sleep : public PCommand
{

public:
							Sleep();

	//++++++++++++++++PCommand
	virtual	void			Undo(PDocument *doc,BMessage *undo);
	virtual	BMessage*		Do(PDocument *doc, BMessage *settings);
	virtual	char*			Name(void){return "Sleep";};
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);
	virtual	const property_info	*PropertyInfo(int32 *count);

protected:
private:
	//----------------PCommand
};
#endif
