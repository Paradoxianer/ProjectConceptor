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
	configMessage=_configMessage;
        mainConfigView->SetConfigMessage(configMessage);
}

void ConfigWindow::Quit(){
	TRACE();
	BWindow::Quit();
}

void ConfigWindow::CreateViews(){
    BGroupLayout *groupLayout =new BGroupLayout(B_VERTICAL);
    SetLayout(groupLayout);

    //Build mainConfigView
    containerView       = new BTabView("containerView");
    BTab    *mainSettingsTab=new BTab();
    mainSettings->SetLabel("Main Settings");
    mainConfigView      = new ConfigView(configMessage);
    containerView->AddTab(mainConfigView, mainSettingsTab);

    //Build PluginConfigView
    BTab    *pluginSettingsTab=new BTab();
    pluginSettingsTab->SetLabel("Plugin Settings");
    pluginConfigView    = new PluginView();
    containerView->AddTab(mainConfigView,pluginSettingsTab );

    groupLayout->AddView(containerView);
}
