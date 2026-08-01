#ifndef INDEXER_H
#define INDEXER_H

#include <app/Message.h>
#include <storage/Entry.h>
#include <storage/File.h>
#include <support/List.h>
#include <map>

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
 * Identity on disk is a stable int32 id, assigned per Indexer instance (i.e.
 * per save) via IdFor() - not a persistent property of the live node/
 * connection BMessage. On load, RegisterDeIndexNode()/DeIndexConnection()
 * populate "sorter" (id -> freshly allocated BMessage*) so later references
 * to the same id resolve to the same new object.
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
			/**find-or-assign the stable id for a live node/connection BMessage
			 * for the duration of this Indexer instance (one save operation)
			 */
			int32				IdFor(BMessage *node);

			BMessage*			RegisterDeIndexNode(BMessage *node);

			BMessage*			DeIndexNode(BMessage *node);
			BMessage*			DeIndexConnection(BMessage *connection);
			BMessage*			DeIndexUndo(BMessage *undo);
//			BMessage*			DeIndexMacro(BMessage *macro);
			BMessage*			DeIndexMacroCommand(BMessage *macro){return DeIndexCommand(macro);};

			BMessage*			DeIndexCommand(BMessage *command);
			/**resolve a previously registered id to the BMessage it belongs to
			 * in this load. Returns false (and leaves *node untouched) if the
			 * id is unknown - callers must not treat that as "no reference",
			 * only as "could not resolve", and should log it.
			 */
			bool				ResolveId(int32 id,BMessage **node);

protected:
			void				Init(void);
			/** one PEditor instance per editor plugin, built on first use
			 * and reused for the rest of this Indexer's lifetime instead
			 * of constructing (and leaking - see issue #71) a fresh one on
			 * every single IndexNode/DeIndexNode/IndexConnection/
			 * DeIndexConnection call. Safe because PreprocessBeforSave()/
			 * PreprocessAfterLoad() only touch the BMessage passed to them,
			 * never the editor's own instance state.
			 */
			BList*				GetCachedEditors(void);


			PDocument			*doc;
			std::map<int32,BMessage*>	sorter;
			std::map<BMessage*,int32>	ids;
			int32				nextId;
			BList				*included;

			PluginManager		*pluginManager;
			BList				*cachedEditors;


private:

};
#endif
