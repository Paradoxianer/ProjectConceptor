#ifndef COMMAND_MANAGER_H
#define COMMAND_MANAGER_H

#include "PCommand.h"
#include "BasePlugin.h"
#include "Indexer.h"

#include <app/Message.h>
#include <app/PropertyInfo.h>
#include <support/List.h>
#include <support/String.h>

#include <map>
using namespace std;

class PDocument;


/**
 * @class PCommandManager
 * @brief  a Class wich manage all PCommands in a Document
 * 
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2005/10/31
 * @Contact: mail@projectconceptor.de
 *
 * Created on: Wed Jun 05  2005
 *
 *
 */

class PCommandManager 
{

public:
						PCommandManager(PDocument *initDoc);
	virtual				~PCommandManager(void);

			void		StartMacro(void);
			void		StopMacro(void);
			void		PlayMacro(BMessage *makro);
			/** looks up a macro by its "Name" field in macroList and plays
			 * it if found; logs and does nothing otherwise - a document not
			 * having a macro under an app-wide shortcut's name is a normal
			 * case, not an error
			 */
			void		PlayMacroByName(const char *name);
	virtual	status_t	RegisterPCommand(BasePlugin *commandPlugin);
	virtual	void		UnregisterPCommand(char *name);

	virtual status_t	Archive(BMessage *archive, bool deep = true);
	virtual	status_t	SetMacroList(BList *newMacroList);
	virtual	status_t	SetUndoList(BList *newUndoList);

	virtual void		SetUndoIndex(uint32 newIndex){undoStatus=newIndex;};
	virtual	PCommand*	GetPCommand(char* name);
	virtual BList*		GetUndoList(void){return undoList;};
	virtual BList*		GetMacroList(void){return macroList;};
	virtual int32		GetUndoIndex(void){return undoStatus;};

	virtual	void		Undo(BMessage *undo);
	virtual	void		Redo(BMessage *redo);

	virtual	status_t	Execute(BMessage *settings);
	
	virtual	int32		CountPCommand(void){return commandMap.size();};
	virtual	PCommand*	PCommandAt(int32 index);
	/**
	 * Concatenates every registered command's own PropertyInfo() entries
	 * into one BPropertyInfo, for #55's scripting suite (PDocument) and
	 * the MacroEditor's DSL field validation - the single canonical
	 * schema source, not duplicated between the two.
	 *
	 * Caller owns and must `delete` the returned BPropertyInfo, but must
	 * NOT try to free the underlying property_info array itself - it's
	 * kept alive on this object (fPropertyInfoArray, rebuilt on every
	 * call) since every command's name/usage/field-name strings inside
	 * it are pointers into that command's own `static const` array
	 * (string literals), not individually heap-allocated. The returned
	 * BPropertyInfo is therefore constructed with freeOnDelete=false -
	 * BPropertyInfo's freeOnDelete=true calls plain free() on every one
	 * of those string pointers individually (matching how Unflatten()
	 * malloc()s them), which crashes on a string literal's address.
	 */
	virtual	BPropertyInfo	*BuildPropertyInfo(void);
	
	virtual PDocument*	BelongTo(void){return doc;};

protected:
	virtual void		Init(void);

			BList		*undoList;
			BList		*macroList;
			/** owned by this object, not by any BPropertyInfo wrapper -
			 * see BuildPropertyInfo(). Freed and rebuilt on every
			 * BuildPropertyInfo() call, freed once more in ~PCommandManager(). */
			property_info	*fPropertyInfoArray;
			int32		undoStatus;
			map<BString, PCommand*>	 commandMap;
			PDocument	*doc;
			BMessage	*recording;
			Indexer		*macroIndexer;
private:

};
#endif
