#ifndef PDOCLOADER_H
#define PDOCLOADER_H

#include <app/Message.h>
#include <storage/Entry.h>
#include <storage/File.h>
#include <support/KeyedVector.h>

#include "Commands/PCommandManager.h"
#include "Document/PEditorManager.h"
#include "Document/WindowManager.h"

/**
 * @class PDocLoader
 *
 * @brief  PDocLoader is a class to load ProjectConceptor files
 * @warning it assums that the Nodes and Connections are BMEssages!!
 * but it automatically restores pointers from Connections to the Nodes (BMessage Objects) 
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2005/10/04
 * @Contact: mail@projectconceptor.de
 *
 *
 */

class PDocLoader
{

public:
								PDocLoader(BEntry *openEntry);
	virtual						~PDocLoader(void);


	virtual	BList*				GetAllNodes(void);
	virtual	BList*				GetAllConnections(void);
	virtual	BList*				GetSelectedNodes(void);

	virtual	BMessage*			GetPrinterSetting(void);
	virtual	BMessage*			GetSettings(void);

	virtual	WindowManager*		GetWindowManager(void);
	virtual	PEditorManager*		GetEditorManager(void);
	virtual	BMessage*			GetCommandManagerMessage(void){return commandManagerMessage;};

protected:
	virtual void				Init(void);
	virtual	void				Load(void);
	//spread sorts all BList stuff out of the nodes... ;-)
	virtual BList*				Spread(BMessage *allNodeMessage);
	//ReIndex takes the stored Connections and ReAssing the Connections to real existing pointers 
	virtual BList*				ReIndexConnections(BMessage *allConnectionsMessage);
	virtual BList*				ReIndexSelected(BMessage *allConnectionsMessage);
	virtual void				ReIndexUndo(BMessage *reIndexUndo);
	virtual void				ReIndexMacro(BMessage *reIndexUndo);
	virtual void				ReIndexCommand(BMessage *commandMessage);
	
		BKeyedVector<int32,BMessage*>*	sorter;
		
		BList*					allNodes;
		BList*					selectedNodes;
		BList*					allConnections;

		BMessage*				settings;
		
		BMessage*				printerSettings;


		BMessage*				commandManagerMessage;

		PEditorManager*			editorManager;
		WindowManager*			windowManager;
		
		BFile*					toLoad;
		BMessage*				loadedStuff;

private:

};
#endif
