#ifndef COMMAND_H
#define COMMAND_H

#include "ObjectHelp.h"

#include <app/Message.h>
#include <app/PropertyInfo.h>
#include <support/List.h>

class PCommandManager;
class PDocument;
/**
 * @class PCommand
 *
 * @brief  PCommand is the BaseObject for all PCommandos wich are executet in The
 * ProjektManager.
 * 
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2005/10/04
 * @Contact: mail@projectconceptor.de
 *
 * Created on: Wed Oct 28  2005
 * @todo need something for MenuInsertion and for Shortcust so that every PCommand wich is loaded automatic inserted
 *
 * @see PCommandManager
 * @see PDocument
 */


class PCommand:  ObjectHelp
{

public:
								PCommand(void);
	virtual						~PCommand(void);
			void				Init(void);
	/**
	 * after adding to the Manager you are able to acces the DocumentClass
	 * and eg. add special Menusentry´s to all BWindows wich are Part of the Dokument
	 * This Name ist normally used to find and use this Command
	 */	
	virtual	void				AttachedToManager(void)		= 0;
	/**
	 * is called short befor the Command is removed from the Manager
	 * normally this only happens if the Document is closed ;-)
	 */	

	virtual void				DetachedFromManager(void)	= 0;
	/**
	 * should return the Name of the Command.
	 * This Name ist normally used to find and use this Command
	 */	
	virtual char*				Name(void)					= 0;
	virtual	BMessage*			Do(PDocument *doc, BMessage *settings);
	virtual	void				Undo(PDocument *doc,BMessage *undo);
	/**
	 * Scripting/macro-editor schema: the command's own property_info
	 * entries (name, commands/specifiers understood, usage, and a
	 * compound field list in ctypes[0].pairs[] describing the settings
	 * fields Do() reads). Defaults to none - a command that doesn't
	 * override this just isn't scriptable/DSL-checkable yet, nothing
	 * else breaks (see #55).
	 */
	virtual	const property_info	*PropertyInfo(int32 *count){*count=0;return NULL;};
			void				SetManager(PCommandManager *newManager);
			PCommandManager*	Manager(void){return manager;};
protected:
	/**
	 * Runs every "PCommand::subPCommand" entry found in `settings` once,
	 * in order, on a fresh copy of each (never the original - Do() may
	 * mutate it, e.g. writing its own undo data, and the original stays
	 * the untouched template) after resolving that copy's own bindings
	 * (see PCommandManager::ResolveBindings() - #135's "$variable"
	 * mechanism). Returns a new BMessage holding one "PCommand::
	 * subPCommand" entry per child actually run, each holding that
	 * child's own post-Do() state - never written back into `settings`
	 * itself here; the caller decides where that record belongs. The
	 * base Do() below uses this for its own (single, unconditional) pass
	 * and replaces settings' own subPCommand entries with the result in
	 * place, exactly as before this was extracted; Repeat/ForEach (#135)
	 * call this once per iteration instead and keep every iteration's
	 * record separately (their own settings' subPCommand list is a fixed
	 * loop-body template that Do() must never overwrite - seeded by an
	 * unrelated loop's iteration count would otherwise corrupt the
	 * recorded macro's own definition, not just this run's result); If
	 * (#135) calls it once, conditionally.
	 */
			BMessage*			RunSubCommandsOnce(PDocument *doc, BMessage *settings);

			PCommandManager*	manager;
//			BList*				subPCommands;
private:

};
#endif
