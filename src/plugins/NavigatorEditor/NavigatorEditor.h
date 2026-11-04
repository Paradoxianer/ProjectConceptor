#ifndef BASIC_EDITOR_H
#define BASIC_EDITOR_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <app/Message.h>
#include <interface/View.h>
#include <support/List.h>

#include <vector>

#include "PEditor.h"
#include "PDocument.h"
#include "PluginManager.h"
#include "MessageListView.h"
#include "NodeListView.h"

#include "BaseListItem.h"

#define DEBUG 1

const uint32	N_A_RENDERER			= 'naRr';
const uint32	N_A_SELECTION_CHANGED	= 'naSC';
const uint32	N_A_INVOKATION			= 'naIK';
const uint32	N_A_VALUE_CHANGED		= 'naVC';
const uint32	N_A_ADD					= 'naAd';
const uint32	N_A_DELETE_NODE			= 'naDN';
/** pointer P_C_DOC_ENTRY: select that node, show it in the graph */
const uint32	N_A_SHOW_IN_GRAPH		= 'naSG';

class ToolBar;
class ToolItem;
class BScrollView;

class NavigatorEditor : public PEditor, public BView
{

public:
							NavigatorEditor();

	//++++++++++++++++PEditor
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);

	virtual void			PreprocessBeforSave(BMessage *container);
	virtual void			PreprocessAfterLoad(BMessage *container);

	// Wraps "this" in a horizontal-only BScrollView, lazily created on
	// first call - GraphEditor's GetView() follows the exact same shape
	// for the same reason (see GraphEditor::UpdateScrollBars()). Only
	// horizontal: each column already scrolls vertically on its own via
	// its own BScrollView (see InitGraph()/InsertNewList()), but nothing
	// previously let the *columns themselves* scroll into view once
	// drilling down made this view wider than the tab.
	virtual	BView*			GetView(void);
	virtual	const char*		TabLabel(void);
	virtual BHandler*		GetHandler(void){return this;};
	virtual	BList*			GetPCommandList(void);

	virtual	BMessage*		GetConfiguration(void){return configMessage;};
	virtual	void			SetConfiguration(BMessage *message){delete configMessage;configMessage=message;};

	virtual	void			ValueChanged(BMessage *changedNodes);

	virtual	bool			IsFocus(void) const;
	virtual	void			MakeFocus(bool focus = true);
	
	//virtual void			PreprocessBeforSave(BMessage *container);
	virtual	void			SetShortCutFilter(ShortCutFilter *_shortCutFilter);


	//----------------PEditor	
	
	//++++++++++++++++BView
		
	virtual void			AttachedToWindow(void);
	virtual void			DetachedFromWindow(void);
	virtual void			FrameResized(float width, float height);
	virtual	void			Draw(BRect updateRect);
	virtual	void			MouseDown(BPoint where);
	virtual	void			MouseMoved(BPoint where, uint32 transit,
								const BMessage *dragMessage);
	virtual	void			MouseUp(BPoint where);

	virtual	void			KeyDown(const char *bytes, int32 numBytes);
	virtual	void			KeyUp(const char *bytes, int32 numBytes);

	virtual	void			MessageReceived(BMessage *msg);

	//----------------BView

	/** Called by NodeListView/MessageListView on every click so the
	 * toolbar knows which list to act on - it has no click position of
	 * its own to work out a target from. */
			void			SetFocusedList(BListView *list){focusedList=list;};

protected:
			void			Init(void);
			void			InitGraph();
			void			InsertNewList(BListView *source);
//			void			InsertRenderObject(BMessage *node);
			void			DeleteRenderObject(BMessage *node);
			void			InitToolBar(void);
			void			ShowInGraph(BMessage *node);
			void			UpdateScrollBars(void);

			/** Places the columns side by side, each as wide as
			 * columnWidths says, a divider between them, and grows this
			 * view to fit them. */
			void			LayoutColumns(void);
			float			ColumnWidth(int32 index) const;
			/** The column whose divider (the gap right of it) is at
			 * where, or -1. */
			int32			DividerAt(BPoint where) const;

			int32			id;
			char*			renderString;

			BMessage		*configMessage;

			BMessenger		*sentTo;

			BScrollView		*myScrollParent;

			NodeListView	*root;
			BListView		*focusedList;

			ToolBar			*toolBar;
			ToolItem		*addItem;
			ToolItem		*deleteItem;

			BList			*viewLine;

			// widths the user dragged a column to, by its place from the
			// left; kept when drilling down replaces the columns
			std::vector<float>	columnWidths;
			int32			draggedDivider;
			float			dragStartX;
			float			dragStartWidth;
			bool			resizeCursor;
private:
};
#endif
