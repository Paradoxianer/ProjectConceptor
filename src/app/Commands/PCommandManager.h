#ifndef COMMAND_MANAGER_H
#define COMMAND_MANAGER_H

#include "PCommand.h"
#include "BasePlugin.h"
#include "Indexer.h"

#include <app/Message.h>
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
	
	virtual PDocument*	BelongTo(void){return doc;};

protected:
	virtual void		Init(void);

			BList		*undoList;
			BList		*macroList;
			int32		undoStatus;
			map<BString, PCommand*>	 commandMap;
			PDocument	*doc;
			BMessage	*recording;
			Indexer		*macroIndexer;
private:

};
#endif
