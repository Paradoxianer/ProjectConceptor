#include <GroupLayout.h>
#include <Button.h>
#include <SpaceLayoutItem.h>
#include "ConfigWindow.h"


ConfigWindow::ConfigWindow(BMessage *_configMessage):BWindow(BRect(50,50,600,400),_T("Settings"),B_TITLED_WINDOW,B_AUTO_UPDATE_SIZE_LIMITS){
	TRACE();
        configMessage=_configMessage;
        CreateViews();
}


void ConfigWindow::ChangeLanguage(){
	TRACE();

}

void ConfigWindow::SetConfigMessage(BMessage *_configMessage){
TRACE();
	configMessage=_configMessage;
        mainConfigView->SetConfigMessage(configMessage);
}

void ConfigWindow::Quit(){
	TRACE();
	BWindow::Quit();
}

void ConfigWindow::CreateViews(){
TRACE();
    BGroupLayout *groupLayout =new BGroupLayout(B_VERTICAL);
    SetLayout(groupLayout);

    //Build mainConfigView
    containerView       = new BTabView("containerView");
    BTab    *mainSettingsTab=new BTab();
    mainConfigView      = new ConfigView(configMessage);
    containerView->AddTab(mainConfigView, mainSettingsTab);
    mainSettingsTab->SetLabel("Main Settings");

    //Build PluginConfigView
    BTab    *pluginSettingsTab=new BTab();
    pluginConfigView    = new PluginView();
    containerView->AddTab(pluginConfigView,pluginSettingsTab );
    pluginSettingsTab->SetLabel("Plugin Settings");

    groupLayout->AddView(containerView);
}
