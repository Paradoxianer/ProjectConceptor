#ifndef INDEXER_H
#define INDEXER_H

#include <app/Message.h>
#include <storage/Entry.h>
#include <storage/File.h>
#include <support/List.h>
#include <map>
using namespace std;

class PDocument;
class PluginManager;
typedef multimap<int32, BMessage*>::iterator iter;
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
//			BMessage*			IndexMacro(BMessage *macro,bool includeNodes=false);
			BMessage*			IndexMacroCommand(BMessage *macro);
			BMessage*			IndexCommand(BMessage *command,bool includeNodes=false);	
			BMessage*			PointerForIndex(int32 index){return sorter[index];};
			
			BMessage*			RegisterDeIndexNode(BMessage *node);

			BMessage*			DeIndexNode(BMessage *node);
			BMessage*			DeIndexConnection(BMessage *connection);	
			BMessage*			DeIndexUndo(BMessage *undo);	
//			BMessage*			DeIndexMacro(BMessage *macro);
			BMessage*			DeIndexMacroCommand(BMessage *macro){return DeIndexCommand(macro);};

			BMessage*			DeIndexCommand(BMessage *command);	

protected:
			void				Init(void);

			
			PDocument			*doc;
			map<int32,BMessage*>	sorter;
			BList				*included;

			PluginManager		*pluginManager;
		

private:

};
#endif
