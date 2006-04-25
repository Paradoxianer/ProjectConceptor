#ifndef SAMPLE_EDITOR_H
#define SAMPLE_EDITOR_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <app/Message.h>
#include <interface/View.h>
#include <support/List.h>

#include "PEditor.h"
#include "BasePlugin.h"
#include "PDocument.h"
#include "PluginManager.h"

#include "PatternToolItem.h"
#include "ColorToolItem.h"
#include "FloatToolItem.h"

const float 	max_entfernung			= 50.0;
const uint32	B_E_RENDERER			= 'beRr';
const uint32	B_E_CONNECTING			= 'beCG';
const uint32	B_E_CONNECTED			= 'beCD';
const uint32	B_E_NEW_SCALE			= 'beNS';



const uint32	B_E_PATTERN_CHANGED		= 'bePC';
const uint32	B_E_COLOR_CHANGED		= 'beCC';
const uint32	B_E_PEN_SIZE_CHANGED	= 'bePS';


const float		triangleHeight	= 7;


class Renderer;

class SampleEditor : public PEditor, public BView
{

public:
							SampleEditor();

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
	
	virtual void			AddRenderer(Renderer* newRenderer);
	virtual void			RemoveRenderer(Renderer* wichRenderer);
	
	virtual Renderer*		FindRenderer(BPoint where);
	virtual Renderer*		FindNodeRenderer(BPoint where);
	virtual Renderer*		FindConnectionRenderer(BPoint where);
	
			PCommand*		MoveCommand(void){return moveCommand;};
			PCommand*		ResizeCommand(void){return resizeCommand;};
			PCommand*		SelectCommand(void){return selectCommand;};
			PCommand*		ChangeValueCommand(void){return changeValueCommand;};
	
	virtual void			BringToFront(Renderer *wichRenderer);
	virtual void			SendToBack(Renderer *wichRenderer);
	
	virtual float			Scale(void){return scale;};
protected:
			void			Init(void);
			void			InsertObject(BPoint where,bool deselect);
			void			InsertRenderObject(BMessage *node);
			void			DeleteRenderObject(BMessage *node);

	static	bool			ProceedRegion(void *arg,void *region);
	static	bool			DrawRenderer(void *arg,void *editor);
			
			int32			id;
			char*			renderString;
			BMenu			*scaleMenu;

			ColorToolItem	*colorItem;
			
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
			
		
			PCommand		*insertCommand;
			PCommand		*selectCommand;
			PCommand		*deleteCommand;
			PCommand		*connectCommand;

			//support for the renderer
			PCommand		*moveCommand;
			PCommand		*resizeCommand;
			PCommand		*changeValueCommand;
			
			BMessenger		*sentTo;

			BRegion			*rendersensitv;
			Renderer		*activRenderer;
			Renderer		*mouseReciver;
			BList			*renderer;
			float			scale;
private:
};
#endif
