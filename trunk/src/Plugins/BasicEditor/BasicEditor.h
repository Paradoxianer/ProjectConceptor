#ifndef BASIC_EDITOR_H
#define BASIC_EDITOR_H
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

const float 	max_entfernung	= 3.0;
const uint32	B_E_RENDERER	= 'beRr';
const uint32	B_E_CONNECTING	= 'beCG';
const uint32	B_E_CONNECTED	= 'beCD';
const float		triangleHeight	= 7;


class BasicEditor : public PEditor, public BView
{

public:
							BasicEditor();

	//++++++++++++++++PEditor
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);

	virtual	BView*			GetView(void){return this;};
	virtual	BHandler*		GetHandler(void){return this;};
	virtual	BList*			GetPCommandList(void);

	virtual	void			ValueChanged(void);
	virtual	void			SetDirty(BRegion *region);

	virtual	bool			IsFocus(void) const;
	virtual	void			MakeFocus(bool focus = true);
	//----------------PEditor	
	
	//++++++++++++++++BView
		
	virtual	void			Draw(BRect updateRect);
	virtual void			DrawAfterChildren(BRect updateRect);
	
	virtual	void			MouseDown(BPoint where);
	virtual	void			MouseMoved(	BPoint where, uint32 code, const BMessage *a_message);
	virtual	void			MouseUp(BPoint where);
	
	virtual	void			KeyDown(const char *bytes, int32 numBytes);
	virtual	void			KeyUp(const char *bytes, int32 numBytes);

	virtual	void			MessageReceived(BMessage *msg);
	
	//----------------BView	
	

protected:
			void			Init(void);
			void			InsertObject(BPoint where,bool deselect);
			void			InsertRenderObject(BMessage *node);
			void			DeleteRenderObject(BMessage *node);
	
			int32			id;
			char*			renderString;
			BList*			renderPlugins;
			
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
			BMessenger		*sentTo;

			BRegion			*rendersensitv;
			
private:
};
#endif
