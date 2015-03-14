#ifndef PLUGINVIEW_H
#define PLUGINVIEW_H

#include <app/Message.h>
#include <interface/Window.h>
#include <ListView.h>
#include <SplitView.h>


#include "ProjectConceptorDefs.h"
#include "PluginManager.h"





class PluginView: public  BSplitView {
public:

					PluginView();


//					~PluginView();

//	virtual	void			ChangeLanguage(void);
        virtual void 			ValueChanged(void);
        virtual void			MessageReceived(BMessage *msg);

protected:

private:
                void			Init();
        virtual void                    BuildPluginList();
        PluginManager                   *pluginManager;
        BListView                       *pluginListView;
        BView                           *configContainer;
};


#endif // PLUGINVIEW_H
