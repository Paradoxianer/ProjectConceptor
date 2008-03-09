#ifndef NODE_EDITOR_H
#define NODE_EDITOR_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <app/Message.h>
#include <interface/View.h>
#include <interface/ScrollView.h>
#include <support/List.h>
#ifdef B_ZETA_VERSION_1_0_0
	#include <locale/Locale.h>
	#include <locale/LanguageNotifier.h>
#else
	#define _T(a) a
#endif


#include "PDocument.h"


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

const float				circleSize				= 2;
const float				gridWidth				= 50;

class Renderer;
class GraphEditor;

class NodeEditor :  public BView
{

public:
							NodeEditor(GraphEditor *_graphEditor,BRect rect);

	//++++++++++++++++PEditor
	//virtual	void			AttachedToManager(void);
	//virtual	void			DetachedFromManager(void);

	virtual	void			ValueChanged(void);
	virtual	void			InitAll(void);

	virtual void			PreprocessBeforSave(BMessage *container);
	virtual void			PreprocessAfterLoad(BMessage *container);

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
			char*			RenderString(void){return renderString;};
			void			SendMessage(BMessage* msg){sentToMe->SendMessage(msg);};
			
			image_id		PluginID(void);
			PDocument		*BelongTo(void);

	
			BMessage		*GetStandartPattern(void){return patternMessage;};

protected:
			void			Init(void);
			void			InsertObject(BPoint where,bool deselect);
			void			InsertRenderObject(BMessage *node);
			BMessage        *GenerateInsertCommand(uint32 newWhat);
			
			void			DeleteFromList(Renderer *wichRenderer);
			void			AddToList(Renderer *wichRenderer, int32 pos);
			void			UpdateScrollBars(void);



	static	bool			DrawRenderer(void *arg,void *editor);

			GraphEditor		*graphEditor;
			char*			renderString;

			BRect			*printRect;

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
			Renderer		*activRenderer;
			Renderer		*mouseReciver;
			BList			*renderer;
			float			scale;

			bool			gridEnabled;

private:
};
#endif
