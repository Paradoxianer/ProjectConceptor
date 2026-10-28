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

/** A typed search (#134): an attribute of a node compared to a value.
 * attribute is an attribute's name, "name" for the node's own name, or
 * empty for any of them; case doesn't matter. op is one of ==, !=, <,
 * <=, >, >= or "contains". Numbers - a number attribute, or text that is a
 * number on both sides - compare as numbers, yes/no as true/false (also
 * yes/no/1/0), everything else as text without regard to case, so dates
 * (YYYY-MM-DD) compare in time. "contains" looks for value in the text. */
struct SearchCriterion {
	BString		attribute;
	BString		op;
	BString		value;
};

/** the criterion in a Find or If command's settings; B_NAME_NOT_FOUND
 * without an operator, B_BAD_VALUE for an unknown one */
status_t	ReadSearchCriterion(const BMessage &settings, SearchCriterion *criterion);

bool		NodeMatchesCriterion(BMessage *node, const SearchCriterion &criterion);

/** like FindMatchingNodes(), for a criterion */
BList*		FindNodesMatchingCriterion(BList *all, const SearchCriterion &criterion);

#endif
