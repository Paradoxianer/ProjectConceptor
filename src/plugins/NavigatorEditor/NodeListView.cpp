#include "NodeListView.h"
#include "NodeItem.h"
#include "NavigatorCommands.h"

#include <interface/Window.h>


NodeListView::NodeListView(BRect rect, BList *forNodeList, PDocument *document, NavigatorEditor *forEditor):BListView(rect,"NodeListView")
{
	nodes=forNodeList;
	doc=document;
	editor=forEditor;
	ValueChanged();
}

void NodeListView::MouseDown(BPoint point)
{
	BListView::MouseDown(point);

	if (editor != NULL)
		NavSetFocusedList(editor,this);

	BMessage	*current	= Window() ? Window()->CurrentMessage() : NULL;
	int32		buttons		= 0;
	if ((doc == NULL) || (current == NULL) || (current->FindInt32("buttons",&buttons) != B_OK)
			|| ((buttons & B_SECONDARY_MOUSE_BUTTON) == 0))
		return;

	int32	index	= IndexOf(point);
	ConvertToScreen(&point);
	if (index < 0) {
		// empty space - every node here is top-level, so a new one is too
		NavShowEmptyContextMenu(doc,NULL,this,point);
		return;
	}
	NodeItem	*item	= dynamic_cast<NodeItem *>(ItemAt(index));
	if (item != NULL)
		NavShowNodeContextMenu(doc,item->GetNode(),false,this,point);
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
