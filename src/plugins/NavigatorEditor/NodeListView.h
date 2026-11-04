#ifndef NODE_LISTVIEW_H
#define NODE_LISTVIEW_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <app/Message.h>
#include <interface/ListView.h>
#include <interface/Point.h>
#include <interface/Rect.h>
#include <support/String.h>

class PDocument;
class NavigatorEditor;

class NodeListView : public BListView
{

public:
						NodeListView(BRect rect, BList *forNodeList, PDocument *document = NULL, NavigatorEditor *forEditor = NULL);
	virtual	void		ValueChanged(void);
	/** shows only the nodes whose name contains filter */
			void		SetFilter(const char *newFilter);
	virtual	void		MouseDown(BPoint point);
protected:
			PDocument	*doc;
			NavigatorEditor	*editor;

	static	bool		AddNodes(void *node,void *list);
			BList		*nodes;
			BString		filter;
private:
};
#endif
