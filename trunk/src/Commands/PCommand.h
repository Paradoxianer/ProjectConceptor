#ifndef COMMAND_H
#define COMMAND_H

#include "ObjectHelp.h"

#include <app/Message.h>
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
			void				SetManager(PCommandManager *newManager);
			PCommandManager*	Manager(void){return manager;};
protected:

			PCommandManager*	manager;
//			BList*				subPCommands;
private:

};
#endif
