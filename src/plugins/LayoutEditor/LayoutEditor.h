#ifndef LAYOUT_EDITOR_H
#define LAYOUT_EDITOR_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <app/Handler.h>
#include <app/Message.h>
#include <app/Messenger.h>
#include <support/List.h>

#include "PDocument.h"
#include "PEditor.h"
#include "PLayouter.h"

class ToolBar;

const uint32	L_E_APPLY_LAYOUT	= 'lEAL';
const uint32	L_E_SET_DIRECTION	= 'lESD';
const uint32	L_E_SET_ENGINE		= 'lESE';

/**
 * @class LayoutEditor
 *
 * @brief View-less PEditor (#105, part of #54): applies an automatic
 * layout on explicit trigger only, never from ValueChanged() (avoids
 * fighting manual repositioning / the #88 recompute-loop trap).
 *
 * GetView() returns NULL (tab-less, see #102). GetHandler() returns
 * `this` (a plain BHandler, not a BView), so it lives on doc's looper,
 * not a window's - see AttachedToManager().
 *
 * Layout math is behind PLayouter (#103); this class only decides when
 * to run it and turns the result into one undoable command.
 */
class LayoutEditor : public PEditor, public BHandler
{

public:
	/** newId is the plugin's own image_id, needed to load its icon
	 * resource in AttachedToManager(); -1 (default) skips icon loading. */
							LayoutEditor(image_id newId = -1);
	virtual					~LayoutEditor(void);

	//++++++++++++++++PEditor
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);

	virtual void			PreprocessBeforSave(BMessage *container) {};
	virtual void			PreprocessAfterLoad(BMessage *container) {};

	virtual	BView*			GetView(void) {return NULL;};
	virtual BHandler*		GetHandler(void) {return this;};

	virtual	BMessage*		GetConfiguration(void) {return configMessage;};
	virtual	void			SetConfiguration(BMessage *message)
								{delete configMessage;configMessage=message;};

	virtual	void			ValueChanged(void);

	virtual	void			SetShortCutFilter(ShortCutFilter *_shortCutFilter);
	//----------------PEditor

	//++++++++++++++++BHandler
	virtual	void			MessageReceived(BMessage *message);
	//----------------BHandler

	/** Defaults to DotLayouter in AttachedToManager(); swappable and
	 * test-injectable. */
			void			SetLayouter(PLayouter *newLayouter);
			PLayouter*		GetLayouter(void) {return layouter;};

	/** Builds one "Batch" wrapper with one "ChangeValue" subPCommand per
	 * node/frame pair in `positions`. Pure message-building, no side
	 * effects. Returns NULL if `positions` had no usable pairs.
	 */
			BMessage*		BuildLayoutCommand(BMessage *positions);

	/** Shifts every "frame" entry in `positions` so the new layout's
	 * bounding-box center lands on the old bounding-box center (from
	 * `nodes`' current P_C_NODE_FRAME) instead of wherever the layouter's
	 * own coordinate space happens to put it - a fresh dot layout starts
	 * near its own origin, which reads as "graph jammed into a corner"
	 * once applied. No-op if either bounding box is empty.
	 */
			void			CenterOnOldBounds(const BList *nodes, BMessage *positions);

protected:
			void			ApplyLayout(void);
			/** Forward to the current layouter's own Graphviz-specific
			 * settings, if it is a DotLayouter (dynamic_cast) - a no-op
			 * for any other PLayouter, since rankdir/engine aren't part
			 * of the generic PLayouter interface.
			 */
			void			SetRankDir(const char *rankdir);
			void			SetEngine(const char *engine);

			BMessage		*configMessage;
			PLayouter		*layouter;
			ToolBar			*toolBar;
			/** guards against reacting to change notifications caused by
			 * this editor's own command; not a concurrency lock. */
			bool			applyingLayout;
			image_id		pluginID;
};
#endif
