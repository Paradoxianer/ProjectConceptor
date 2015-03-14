#ifndef NODE_LISTVIEW_H
#define NODE_LISTVIEW_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <app/Message.h>
#include <interface/ListView.h>
#include <interface/Point.h>
#include <interface/Rect.h>

class NodeListView : public BListView
{

public:
						NodeListView(BRect rect, BList *forNodeList);
	virtual	void		ValueChanged(void);
protected:

	static	bool		AddNodes(void *node,void *list);
			BList		*nodes;
private:
};
#endif
