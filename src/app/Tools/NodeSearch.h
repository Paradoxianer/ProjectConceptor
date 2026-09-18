#ifndef NODE_SEARCH_H
#define NODE_SEARCH_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <app/Message.h>
#include <support/List.h>
#include <support/String.h>

/** True if `node` (or any nested BMessage field inside it, recursively) has
 * a string field whose value contains `search` as a substring. Extracted
 * from the original FindCommand plugin (Find::FindInNode()) into
 * libProjectConceptor.so so other plugins - currently the If command
 * (#135), which needs the exact same "does this match" check Find itself
 * uses without duplicating it or linking against Find's own plugin .so -
 * can reuse it directly. */
bool NodeMatchesSearch(BMessage *node, const BString &search);

/** Every entry in `all` for which NodeMatchesSearch() is true. Caller owns
 * the returned BList itself (not the BMessage entries it holds, which stay
 * owned by whatever `all` itself belongs to - same ownership shape as
 * Find::FindNodes()). */
BList* FindMatchingNodes(BList *all, const BString &search);

#endif
