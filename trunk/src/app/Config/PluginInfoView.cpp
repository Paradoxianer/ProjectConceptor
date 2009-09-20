#include "PluginInfoView.h"
#include <StringView.h>
#include <TextView.h>
#include <GridLayout.h>
#include "ProjectConceptorDefs.h"

 map<uint32, BString>  	PluginInfoView::pcTypes;

PluginInfoView::PluginInfoView(BasePlugin *_plugin):BGridView(5,5){
     if (pcTypes.size() == 0){
	pcTypes[P_C_EDITOR_PLUGIN_TYPE	]="Editor";
	pcTypes[P_C_COMMANDO_PLUGIN_TYPE]="Command";
	pcTypes[P_C_ITEM_PLUGIN_TYPE]="Item";
	pcTypes[P_C_ITEM_INPORT_EXPORT_TYPE]="Import/Export";
    }
    BGridLayout *layout = (BGridLayout* )this->GetLayout();
    layout->AddView(new BStringView("Name","Name:"),0,0);
    layout->AddView(new BStringView("NameData",_plugin->GetName()),1,0);
    layout->AddView(new BStringView("Type","Type:"),0,2);
    layout->AddView(new BStringView("TypeData",pcTypes[_plugin->GetType()]),1,2);

    layout->AddView(new BStringView("Autor","Autor:"),0,3);
    layout->AddView(new BStringView("AutorData", _plugin->GetAutor()),1,3);
    layout->AddView(new BStringView("Version","Version:"),0,4);
    layout->AddView(new BStringView("VersionData",_plugin->GetVersionsString()),1,4);

    layout->AddView(new BStringView("Description","Description:"),0,5);
    BTextView *textView=new BTextView("DescriptionData");
    textView->SetText(_plugin->GetDescription());
    textView->MakeEditable(false);
    layout->AddView(textView,1,5,1,5);
}
