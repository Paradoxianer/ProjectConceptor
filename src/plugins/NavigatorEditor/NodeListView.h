#ifndef NODE_LISTVIEW_H
#define NODE_LISTVIEW_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <app/Message.h>
#include <interface/ListView.h>
#include <interface/Point.h>
#include <interface/Rect.h>

class PDocument;
class NavigatorEditor;

class NodeListView : public BListView
{

public:
						NodeListView(BRect rect, BList *forNodeList, PDocument *document = NULL, NavigatorEditor *forEditor = NULL);
	virtual	void		ValueChanged(void);
	virtual	void		MouseDown(BPoint point);
protected:
			PDocument	*doc;
			NavigatorEditor	*editor;

	static	bool		AddNodes(void *node,void *list);
			BList		*nodes;
private:
};
#endif
