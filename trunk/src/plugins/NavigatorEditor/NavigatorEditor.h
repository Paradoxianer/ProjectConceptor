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


class NavigatorEditor : public PEditor, public BView
{

public:
							NavigatorEditor();

	//++++++++++++++++PEditor
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);

	virtual void			PreprocessBeforSave(BMessage *container);
	virtual void			PreprocessAfterLoad(BMessage *container);

	virtual	BView*			GetView(void){return this;};
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
	
	virtual	void			KeyDown(const char *bytes, int32 numBytes);
	virtual	void			KeyUp(const char *bytes, int32 numBytes);

	virtual	void			MessageReceived(BMessage *msg);
	
	//----------------BView	
	
	
protected:
			void			Init(void);
			void			InitGraph();
			void			InsertNewList(BListView *source);
//			void			InsertRenderObject(BMessage *node);
			void			DeleteRenderObject(BMessage *node);

			int32			id;
			char*			renderString;

			BMessage		*configMessage;
			
			BMessenger		*sentTo;

			NodeListView	*root;

			BList			*viewLine;
private:
};
#endif
