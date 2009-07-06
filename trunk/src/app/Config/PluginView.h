#ifndef PLUGINVIEW_H
#define PLUGINVIEW_H

#include <app/Message.h>
#include <interface/Window.h>


#include "ProjectConceptorDefs.h"
#include "PluginManager)"





class PluginView: public BView
{
public:

    PluginView(PluginManager *_pManager,BMessage *forMessage, uint32 resizingMode = B_FOLLOW_ALL_SIDES, uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP);


//					~PluginView();

//	virtual	void			ChangeLanguage(void);
        virtual void 			ValueChanged(void);
        virtual void			MessageReceived(BMessage *msg);

protected:

private:
                void			Init();
        virtual void                    BuildPluginList();
        BListView                       *pluginListView;
        BView                           *configContainer;
};


#endif // PLUGINVIEW_H
