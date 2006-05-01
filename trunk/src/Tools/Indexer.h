#ifndef INDEXER_H
#define INDEXER_H

#include <app/Message.h>
#include <storage/Entry.h>
#include <storage/File.h>
#include <support/KeyedVector.h>
#include <support/List.h>

class PDocument;
class PluginManager;
/**
 * @class Indexer
 *
 * @brief Indexer prepare Nodes or Connection to be able to "detached" from the running Programm
 * This meand it provide funktionality to convert known Pointer eg. in a Node BMessage so that the Node
 * coud be stored without losing it´s data.
 * and it´s Provides although the posiblity to revert this Convertion
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2006/05/01
 * @Contact: mail@projectconceptor.de
 *
 *
 */

class Indexer
{

public:
								Indexer(PDocument *documnent);
								~Indexer(void);

			BMessage*			IndexNode(BMessage *node);	
			BMessage*			IndexConnection(BMessage *connection,bool includeNodes=false);	
			BMessage*			IndexUndo(BMessage *undo,bool includeNodes=false);	
			BMessage*			IndexMacro(BMessage *macro,bool includeNodes=false);
			BMessage*			IndexCommand(BMessage *command,bool includeNodes=false);	

			
			BMessage*			DeIndexNode(BMessage *node);
			BMessage*			DeIndexConnection(BMessage *connection);	
			BMessage*			DeIndexUndo(BMessage *undo);	
			BMessage*			DeIndexMacro(BMessage *macro);	
			BMessage*			DeIndexCommand(BMessage *command);	

protected:
			void				Init(void);
	//spread sorts all BList stuff out of the nodes... ;-)
			BList*				Spread(BMessage *allNodeMessage);
	//ReIndex takes the stored Connections and ReAssing the Connections to real existing pointers 
			BList*				ReIndexConnections(BMessage *allConnectionsMessage);
			BList*				ReIndexSelected(BMessage *allConnectionsMessage);
			void				ReIndexUndo(BMessage *reIndexUndo);
			void				ReIndexMacro(BMessage *reIndexUndo);
			void				ReIndexCommand(BMessage *commandMessage);
			
			PDocument			*doc;
			BKeyedVector<int32,BMessage*>*	sorter;
			PluginManager		*pluginManager
/*		BList*					allNodes;
		BList*					selectedNodes;
		BList*					allConnections;

		BMessage*				settings;
		
		BMessage*				printerSettings;


		BMessage*				commandManagerMessage;

		PEditorManager*			editorManager;
		WindowManager*			windowManager;
		
		BFile*					toLoad;
		BMessage*				loadedStuff;*/
		

private:

};
#endif
