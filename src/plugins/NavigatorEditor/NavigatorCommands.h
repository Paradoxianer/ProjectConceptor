#ifndef NAVIGATOR_COMMANDS_H
#define NAVIGATOR_COMMANDS_H
/*
 * Shared right-click context-menu actions for NavigatorEditor's list
 * views (MessageListView, NodeListView) - both show the same NodeItems
 * and need the same add/delete actions (#56), so this is one place
 * instead of duplicating menu-building and command-sending logic
 * between them. Reuses the existing Insert/Delete/AddAttribute/
 * RemoveAttribute PCommand plugins exactly as GraphEditor does - no new
 * commands needed, just a different UI driving them.
 */

#include <app/Message.h>
#include <interface/ListView.h>
#include <interface/Point.h>
#include <interface/View.h>

class PDocument;
class NavigatorEditor;

/** Context menu for a node row: add attribute (bool/text), delete
 * attribute (submenu listing the node's current ones), delete node,
 * and - only when isChildList is true, i.e. this node lives in a
 * group's own P_C_NODE_ALLNODES list - add a child node. */
void NavShowNodeContextMenu(PDocument *doc, BMessage *node, bool isChildList,
	BView *owner, BPoint screenPoint);

/** Context menu for empty list space: add a new node - top-level if
 * parentNode is NULL, a child of parentNode otherwise. */
void NavShowEmptyContextMenu(PDocument *doc, BMessage *parentNode,
	BView *owner, BPoint screenPoint);

/** Called from NodeListView/MessageListView::MouseDown so the toolbar
 * (which has no click position of its own to work out its target from)
 * knows which list the user last interacted with. */
void NavSetFocusedList(NavigatorEditor *editor, BListView *list);

/** Toolbar actions - same commands as the context menu, but the target
 * node is whichever NodeItem is currently selected in focusedList
 * (add-node/add-attribute act on it like "add child"/"add attribute" in
 * the context menu; with nothing selected, add-node falls back to a
 * top-level node, same as the empty-space context menu). */
void NavToolbarAddNode(PDocument *doc, BListView *focusedList);
void NavToolbarAddAttribute(PDocument *doc, BListView *focusedList,
	BView *owner, BPoint screenPoint);
void NavToolbarDeleteNode(PDocument *doc, BListView *focusedList);

#endif
