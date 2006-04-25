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


const uint32	N_A_RENDERER			= 'naRr';
const uint32	N_A_SELECTION_CHANGED	= 'naSC';
const uint32	N_A_INVOKATION			= 'naIK';
const uint32	N_A_VALUE_CHANGED		= 'naVC';


class NavigatorEditor : public PEditor, public BView
{

public:
							NavigatorEditor();

	//++++++++++++++++PEditor
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);

	virtual	BView*			GetView(void){return this;};
	virtual BHandler*		GetHandler(void){return this;};
	virtual	BList*			GetPCommandList(void);

	virtual	void			ValueChanged(void);
	virtual	void			SetDirty(BRegion *region);

	virtual	bool			IsFocus(void) const;
	virtual	void			MakeFocus(bool focus = true);
	//----------------PEditor	
	
	//++++++++++++++++BView
		
	
	virtual	void			KeyDown(const char *bytes, int32 numBytes);
	virtual	void			KeyUp(const char *bytes, int32 numBytes);

	virtual	void			MessageReceived(BMessage *msg);
	
	//----------------BView	
	
	
protected:
			void			Init(void);
			void			InsertNewList(BListView *source);
//			void			InsertRenderObject(BMessage *node);
			void			DeleteRenderObject(BMessage *node);

			int32			id;
			char*			renderString;
			

			PCommand		*insertCommand;
			PCommand		*selectCommand;
			PCommand		*deleteCommand;
			PCommand		*connectCommand;
			BMessenger		*sentTo;

			NodeListView	*root;

			BList			*viewLine;
private:
};
#endif
