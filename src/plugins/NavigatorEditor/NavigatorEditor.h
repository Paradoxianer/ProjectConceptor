#ifndef BASIC_EDITOR_H
#define BASIC_EDITOR_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <app/Message.h>
#include <interface/View.h>
#include <support/List.h>

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
	virtual BHandler*		GetHandler(void){return this;};
	virtual	BList*			GetPCommandList(void);

	virtual	BMessage*		GetConfiguration(void){return configMessage;};
	virtual	void			SetConfiguration(BMessage *message){delete configMessage;configMessage=message;};

	virtual	void			ValueChanged(void);

	virtual	bool			IsFocus(void) const;
	virtual	void			MakeFocus(bool focus = true);
	
	//virtual void			PreprocessBeforSave(BMessage *container);
	virtual	void			SetShortCutFilter(ShortCutFilter *_shortCutFilter);


	//----------------PEditor	
	
	//++++++++++++++++BView
		
	virtual void			AttachedToWindow(void);
	virtual void			DetachedFromWindow(void);
	virtual void			FrameResized(float width, float height);
	
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
			void			UpdateScrollBars(void);

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
private:
};
#endif
