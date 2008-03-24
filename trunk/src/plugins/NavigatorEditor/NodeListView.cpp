#include "NodeListView.h"
#include "NodeItem.h"


NodeListView::NodeListView(BRect rect, BList *forNodeList):BListView(rect,"NodeListView")
{
	nodes=forNodeList;
	ValueChanged();
}


void NodeListView::ValueChanged()
{
	BMessage	*messageNode	= NULL;
	MakeEmpty();
	nodes->DoForEach(AddNodes,this);
}

bool NodeListView::AddNodes(void *node,void *list)
{
	BMessage *nodeMessage=(BMessage *)node;
	if (nodeMessage!=NULL)
		((BListView *)list)->AddItem(new NodeItem(nodeMessage));
	return false;	
}
