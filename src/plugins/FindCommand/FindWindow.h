#ifndef FIND_WINDOW_H
#define FIND_WINDOW_H

#include "ProjectConceptorDefs.h"
#include "PDocument.h"

#include <app/Message.h>
#include <interface/MenuField.h>
#include <interface/TextControl.h>
#include <interface/Window.h>

/**
 * @class FindWindow
 * @brief Defines a Window to type in a search phrase
 *
 * @author Paradoxon
 *
 * Searches while typing: a text in any value, or an attribute compared
 * to a value (#134) - whichever was changed last. OK keeps the result as
 * one Find command (recorded in a macro), Cancel leaves the last preview.
 */
class FindWindow : public BWindow
{

public:
							FindWindow(PDocument *tmpDoc);
	virtual void			MessageReceived(BMessage *message);

private:
			void			CreateViews();
			/** the names of all attributes of the document's nodes */
			void			CollectAttributeNames(BMessage *names);
			/** a Find request for the document; shadow: only a preview */
			void			SendSearch(bool shadow);
			BString			MarkedValue(BMenuField *field, const char *name);

			BTextControl	*searchText;
			BMenuField		*attributeField;
			BMenuField		*operatorField;
			BTextControl	*valueText;
			/** the attribute row was changed after the search text */
			bool			useCriterion;
			PDocument		*doc;
};
#endif
