#ifndef GRAPH_EDITOR_H
#define GRAPH_EDITOR_H
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

#include "PEditor.h"
#include "BasePlugin.h"
#include "PDocument.h"
#include "PluginManager.h"

#include "PatternToolItem.h"
#include "ColorToolItem.h"
#include "FloatToolItem.h"

#include "MessageListView.h"
#include "NodeEditor.h"

#include "BViewSplitter.h"

/*const float			 	max_entfernung			= 50.0;
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
//const uint32			G_E_INSERT_NODE 		= 'geIN';
//*order to Insert and new a Node directly as a sibling to the last selected Node*/
//const uint32            G_E_INSERT_SIBLING      = 'geIS';

const uint32            G_E_INVOKATION      	= 'geIN';

extern const char		*G_E_TOOL_BAR;		//	= "G_E_TOOL_BAR";

const uint32			MESSAGE_VIEW_WIDTH		= 200;

/*const float		triangleHeight	= 7;
const float		gridWidth		= 50;*/

class Renderer;

class GraphEditor : public PEditor, public BViewSplitter
{

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

	virtual	void			MessageReceived(BMessage *msg);
	//----------------BView

			image_id		PluginID(void){return pluginID;};
			
			BMessage		*GetStandartPattern(void){return patternMessage;};
			rgb_color		GetColor(void){return colorItem->GetColor();};
			float			GetPenSize(void){penSize->GetValue();};

protected:
			void			Init(void);

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


			BMessage		*nodeMessage;
			BMessage		*fontMessage;
			BMessage		*patternMessage;
			BMessage		*configMessage;

			image_id 		pluginID;
			
			MessageListView	*showMessage;
			NodeEditor		*nodeEditor;
			BScrollView		*myScrollParent;

private:
};
#endif
