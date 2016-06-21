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
const uint32			G_E_PEN_SIZE_CHANGED	= 'gePS';
const uint32			G_E_ADD_ATTRIBUTE		= 'geAA';
//*order to Insert and new a Node and to connect it to all current selected Nodes*/
const uint32			G_E_INSERT_NODE 		= 'geIN';
//*order to Insert and new a Node directly as a sibling to the last selected Node*/
const uint32            G_E_INSERT_SIBLING      = 'geIS';

extern const char		*G_E_TOOL_BAR;		//	= "G_E_TOOL_BAR";

const float		triangleHeight	= 7;
const float		gridWidth		= 50;
const float		circleSize		= 3.0;

class Renderer;

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


protected:
			void			Init(void);
			void			InsertObject(BPoint where,bool deselect);
			void			InsertRenderObject(BMessage *node);
			
			void			DeleteFromList(Renderer *wichRenderer);
			void			AddToList(Renderer *wichRenderer, int32 pos);
			void			UpdateScrollBars(void);



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
			BList			*renderer;
			float			scale;

			bool			gridEnabled;
			image_id 		pluginID;

			BScrollView		*myScrollParent;
			
			uint32			oldEventMask;

private:
};
#endif
