#ifndef GRAPH_EDITOR_H
#define GRAPH_EDITOR_H
/*
 * @author Paradoxon powered by Jesus Christ
 *
 * For God so loved the world that he gave
 * his one and only Son, that whoever believes
 * in him shall not perish but have eternal life.
 * Joh. 3, 16
 */
#include <app/Message.h>
#include <interface/View.h>
#include <interface/ScrollView.h>
#include <support/List.h>

#include "PEditor.h"
#include "BasePlugin.h"
#include "PDocument.h"
#include "PluginManager.h"

#include "PatternToolItem.h"
#include "ColorToolItem.h"
#include "FloatToolItem.h"

const float			 	max_entfernung			= 50.0;
const uint32			G_E_RENDERER			= 'geRr';
const uint32			G_E_CONNECTING			= 'geCG';
const uint32			G_E_CONNECTED			= 'geCD';
const uint32			G_E_GROUP				= 'geGR';

const uint32			G_E_NEW_SCALE			= 'geNS';
const uint32			G_E_INVALIDATE			= 'geIV';
const uint32			G_E_GRID_CHANGED		= 'geGC';

const uint32			G_E_PATTERN_CHANGED		= 'gePC';
const uint32			G_E_COLOR_CHANGED		= 'geCC';
// live preview only, while a color picker is still open - updates the
// selected nodes'/connections' renderers directly (see Renderer::
// SetPreviewFillColor()), never touches document data or undo history.
// G_E_COLOR_CHANGED above is still the one real, undo-worthy commit,
// sent once when the picker closes - see docs/notes.md.
const uint32			G_E_COLOR_PREVIEW		= 'geCP';
const uint32			G_E_PEN_SIZE_CHANGED	= 'gePS';
const uint32			G_E_ADD_ATTRIBUTE		= 'geAA';
//*order to Insert and new a Node and to connect it to all current selected Nodes*/
const uint32			G_E_INSERT_NODE 		= 'geIN';
//*order to Insert and new a Node directly as a sibling to the last selected Node*/
const uint32            G_E_INSERT_SIBLING      = 'geIS';
// drives Renderer::AnimationStep() for every renderer in animatingRenderers,
// see StartAnimating(); not sent by anything outside GraphEditor itself.
const uint32			G_E_ANIMATION_TICK		= 'geAT';

extern const char		*G_E_TOOL_BAR;		//	= "G_E_TOOL_BAR";

const float		triangleHeight	= 7;
const float		gridWidth		= 50;
const float		circleSize		= 3.0;

class Renderer;
class BMessageRunner;

class GraphEditor : public PEditor, public BView {

public:
							GraphEditor(image_id newId);

	//++++++++++++++++PEditor
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);

	virtual	BView*			GetView(void);
	virtual BHandler*		GetHandler(void){return this;};
	virtual	BList*			GetPCommandList(void);

	virtual	void			ValueChanged(void);
	virtual	void			InitAll(void);

	virtual	void			SetDirty(BRegion *region);
	virtual	BMessage*		GetConfiguration(void){return configMessage;};
	virtual	void			SetConfiguration(BMessage *message){delete configMessage;configMessage=message;};

	virtual void			PreprocessBeforSave(BMessage *container);
	virtual void			PreprocessAfterLoad(BMessage *container);
	virtual	void			SetShortCutFilter(ShortCutFilter *_shortCutFilter);

	//----------------PEditor

	//++++++++++++++++BView
	virtual void			AttachedToWindow(void);
	virtual void			DetachedFromWindow(void);

	virtual	void			Draw(BRect updateRect);

	virtual	void			MouseDown(BPoint where);
	virtual	void			MouseMoved(	BPoint where, uint32 code, const BMessage *a_message);
	virtual	void			MouseUp(BPoint where);

	virtual	void			MessageReceived(BMessage *msg);

	virtual void			FrameResized(float width, float height);
	//----------------BView

			Renderer*		CreateRendererFor(BMessage *node);
			void			AddRenderer(Renderer* newRenderer);
			void			RemoveRenderer(Renderer* wichRenderer);

			bool			GridEnabled(void){return gridEnabled;};
			float			GridWidth(void){return gridWidth;};

			Renderer*		FindRenderer(BPoint where);
			Renderer*		FindNodeRenderer(BPoint where);
			Renderer*		FindConnectionRenderer(BPoint where);
			Renderer*		FindRenderer(BMessage *container);

			void			BringToFront(Renderer *wichRenderer);
			void			SendToBack(Renderer *wichRenderer);

			float			Scale(void){return scale;};
			BList*			RenderList(void){return renderer;};
			image_id		PluginID(void){return pluginID;};
			char*			RenderString(void){return renderString;};
			void			SendMessage(BMessage* msg){sentToMe->SendMessage(msg);};
			void			SendMessageToDoc(BMessage* msg){sentTo->SendMessage(msg);};
			BMessage		*GetStandartPattern(void){return patternMessage;};
			BMessage        *GenerateInsertCommand(uint32 newWhat, bool connected = false);

			/** Registers wichRenderer for per-frame AnimationStep() calls
			 * (lazily starts the shared tick runner); the renderer removes
			 * itself once AnimationStep() reports it has settled. */
			void			StartAnimating(Renderer *wichRenderer);


protected:
			void			Init(void);
			void			InsertObject(BPoint where,bool deselect);
			void			InsertRenderObject(BMessage *node);
			/** ValueChanged()'s per-node update/insert/remove logic, split out
			 * so it can run over allNodes/allConnections in two passes - see
			 * ValueChanged() for why the order matters.
			 */
			void			ProcessChangedNode(BMessage *node,BList *allNodes,BList *allConnections);

			void			DeleteFromList(Renderer *wichRenderer);
			void			AddToList(Renderer *wichRenderer, int32 pos);
			void			UpdateScrollBars(void);
			/** overlays the app-wide binding for the given action name
			 * (ConfigManager's Shortcuts config) onto *key/*modifiers,
			 * leaving them at their passed-in defaults if none is set
			 */
			void			ReadShortcutBinding(const char *action,int32 *key,int32 *modifiers);



	static	bool			ProceedRegion(void *arg,void *region);
	static	bool			DrawRenderer(void *arg,void *editor);


			int32			id;
			char*			renderString;
			BMenu			*scaleMenu;
			ToolBar			*toolBar;
			ToolItem		*grid;

			ToolItem		*addGroup;
			ToolItem		*addBool;
			ToolItem		*addText;


			FloatToolItem	*penSize;
			ColorToolItem	*colorItem;
			PatternToolItem	*patternItem;

			BRect			*printRect;
			bool			key_hold;

			BPoint			*startMouseDown;
			bool			connecting;
			BPoint			*fromPoint;
			BPoint			*toPoint;
			BRect			*selectRect;

			BMessage		*nodeMessage;
			BMessage		*fontMessage;
			BMessage		*patternMessage;
			BMessage		*configMessage;
			BMessage		*connectionMessage;
			BMessage		*groupMessage;

			BMessenger		*sentTo;
			BMessenger		*sentToMe;
			BRegion			*rendersensitv;
			Renderer		*activRenderer;
			Renderer		*mouseReciver;
			// "start editing this node's name once its renderer exists" is
			// pure GUI intent for the next InsertRenderObject() to act on -
			// keeping it as a GraphEditor-local pointer instead of a bool
			// flag on the node's own BMessage (the previous design) means it
			// can never leak into a saved file, no matter what order saves
			// and inserts happen to race in. See issue #75. Compared by
			// identity in CreateRendererFor(), so if the insert that set
			// this never results in a renderer (a failed/undone command,
			// say), the pointer goes stale until overwritten by the next
			// insert - accepted as a narrow, cosmetic risk (an unrelated
			// future node could in theory get an unwanted edit-focus if a
			// new BMessage happens to reuse that exact freed address) rather
			// than adding more state to close off what a real bool flag on
			// the node never risked in the first place, just differently.
			BMessage		*pendingStartEditNode;
			BList			*renderer;
			float			scale;

			/** renderers currently mid-AnimationStep(); drives the shared
			 * G_E_ANIMATION_TICK runner, see StartAnimating(). */
			BList			*animatingRenderers;
			BMessageRunner	*animationRunner;
			bigtime_t		animationLastTick;

			bool			gridEnabled;
			image_id 		pluginID;

			BScrollView		*myScrollParent;
			
			uint32			oldEventMask;

private:
};
#endif
