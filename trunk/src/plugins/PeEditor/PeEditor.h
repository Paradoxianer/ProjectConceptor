#ifndef PE_EDITOR_H
#define PE_EDITOR_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <app/Message.h>
#include <interface/View.h>
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


class GraphEditor : public PEditor, public BView
{

public:
							GraphEditor(image_id newId);

	//++++++++++++++++PEditor
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);

	virtual	BView*			GetView(void){return this;};
	virtual BHandler*		GetHandler(void){return this;};
	virtual	BList*			GetPCommandList(void);

	virtual	void			ValueChanged(void);
	virtual	void			InitAll(void);

	virtual	void			SetDirty(BRegion *region);
	virtual	BMessage*		GetConfiguration(void){return configMessage;};
	virtual	void			SetConfiguration(BMessage *message){delete configMessage;configMessage=message;};

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

	virtual	void			KeyDown(const char *bytes, int32 numBytes);
	virtual	void			KeyUp(const char *bytes, int32 numBytes);

	virtual	void			MessageReceived(BMessage *msg);

	virtual void			FrameResized(float width, float height);
	//----------------BView


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

private:
};
#endif
