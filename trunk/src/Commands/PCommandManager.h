#ifndef COMMAND_MANAGER_H
#define COMMAND_MANAGER_H

#include <app/Message.h>
#include <support/List.h>
#include <support/KeyedVector.h>
#include <support/String.h>

#include "PCommand.h"
#include "BasePlugin.h"
#include "Indexer.h"
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

	virtual	void		StartMacro(void);
	virtual	void		StopMacro(void);
	virtual void		PlayMacro(BMessage *makro);
	virtual	status_t	RegisterPCommand(BasePlugin *commandPlugin);
	virtual	void		UnregisterPCommand(char *name);

	virtual status_t	Archive(BMessage *archive, bool deep = true);
	virtual	status_t	LoadMacros(BMessage *archive);
	virtual	status_t	LoadUndo(BMessage *archive);

	virtual	PCommand*	GetPCommand(char* name);

//	virtual	BList*		GetPCommandPluginList(void){return commandList;};

	virtual	void		Undo(BMessage *undo);
	virtual	void		Redo(BMessage *redo);

	virtual	void		Execute(BMessage *settings);
	
	virtual	int32		CountPCommand(void){return commandVector->CountItems();};
	virtual	PCommand*	PCommandAt(int32 index){return (PCommand *)commandVector->ValueAt(index);};
	
	virtual PDocument* BelongTo(void){return doc;};

protected:
	virtual void		Init(void);

			BList		*undoList;
			BList		*macroList;
			int32		undoStatus;
			BKeyedVector<BString,PCommand *>	*commandVector;
			PDocument	*doc;
			BMessage	*recording;
			Indexer		*macroIndexer;
private:

};
#endif
