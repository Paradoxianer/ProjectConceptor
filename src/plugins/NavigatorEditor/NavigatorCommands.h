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

/** Context menu for a node row: "Add field" (a submenu of BMessage
 * types - boolean/integer/float/text/rect - each added as a plain field
 * directly on the node, not wrapped in GraphEditor's own Name/Value
 * attribute shape, so NavigatorEditor can manipulate the BMessage
 * itself rather than being limited to what GraphEditor knows how to
 * render), "Delete field" (submenu listing both those attribute-shaped
 * fields and any plain top-level ones - not the node's structural
 * fields like Frame/Font/Pattern, deleting those would just break
 * rendering elsewhere for no benefit), delete node, and - only when
 * isChildList is true, i.e. this node lives in a group's own
 * P_C_NODE_ALLNODES list - add a child node. */
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

/** Toolbar actions - each one fires immediately, no submenu: a toolbar
 * icon should do the thing, not open a chooser (that's what the
 * right-click menu above is for). Picking a field *type* inherently
 * needs a choice, so field creation stays right-click-only; the
 * toolbar only covers the two operations that don't need one. Both act
 * on whichever NodeItem (or, for delete, plain field row) is currently
 * selected in focusedList. */
void NavToolbarAddNode(PDocument *doc, BListView *focusedList);
void NavToolbarDelete(PDocument *doc, BListView *focusedList);

#endif
