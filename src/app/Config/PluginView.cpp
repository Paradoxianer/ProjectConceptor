#include <ScrollView.h>

#include "PluginView.h"
//#include "BasePlugin.h"
#include "ProjectConceptor.h"
#include "PDocumentManager.h"
#include "PluginItem.h"
#include "PluginInfoView.h"
#include "StringItem.h"


PluginView::PluginView():BSplitView(B_HORIZONTAL,5.0){
    TRACE();
    //    pluginManager=_pManager;
    Init();
}

void PluginView::ValueChanged(void){
TRACE();

}

void PluginView::MessageReceived(BMessage *msg){
TRACE();
	cardLayout->SetVisibleItem(pluginListView->CurrentSelection());
	switch(msg->what) {
		case 'selC': {	
			break;
		}
		default:
			BSplitView::MessageReceived(msg);
		break;
	}
}

void PluginView::Init(){
 TRACE();
   pluginListView  = new BListView("pluginList",B_SINGLE_SELECTION_LIST) ;
   cardLayout = new BCardLayout();
   configContainer = new BView("configContinaer",0,cardLayout);
   PDocumentManager* tmpManager = ((ProjektConceptor*)be_app)->GetPDocumentManager();
   pluginManager = tmpManager->GetPluginManager();
   BuildPluginList();
   //BSplitView::AddChild(pluginListView,configContainer);
//    AddChild(new BScrollView("PluginListScroller",pluginListView));
   AddChild(pluginListView);
   AddChild(configContainer);
   pluginListView->SetDrawingMode(B_OP_OVER);
   pluginListView->Select(0);
   pluginListView->SetSelectionMessage(new BMessage('selC'));
   pluginListView->SetTarget(this);
   cardLayout->SetVisibleItem((int32)0);
}

void PluginView::BuildPluginList(){
 TRACE();
   BList* allPlugins	= pluginManager->GetAllPlugins();
    for (int i=0; i<allPlugins->CountItems(); i++){
		BasePlugin* plugin = (BasePlugin *)allPlugins->ItemAt(i);
		pluginListView->AddItem(new PluginItem(plugin,pluginManager));
	//	pluginListView->AddItem(new BStringItem(plugin->GetName()));
		if (plugin->GetConfigView()!=NULL){
		    BSplitView *splitView = new BSplitView(B_VERTICAL);
		    splitView->AddChild(new BScrollView("PluginConfigScroller",plugin->GetConfigView()));
		    splitView->AddChild(new BScrollView("PluginInfoScroller",new PluginInfoView(plugin)));
		    configContainer->AddChild(splitView);
		}
		else {	
		//	configContainer->AddChild(new BScrollView("PluginInfoScroller",new PluginInfoView(plugin));
			configContainer->AddChild(new PluginInfoView(plugin));
		}
	//configContainer->AddChild(new PluginInfoView(plugin));
    }
}
