#include "ConfigWindow.h"


ConfigWindow::ConfigWindow(BMessage *_configMessage):BWindow(BRect(50,50,600,400),_T("Settings"),B_TITLED_WINDOW,0){
	TRACE();
	configMessage=_configMessage;
	configView	= new ConfigView(Bounds(),configMessage);
	AddChild(configView);

}


void ConfigWindow::ChangeLanguage(){
	TRACE();

}

void ConfigWindow::SetConfigMessage(BMessage *_configMessage){
	configMessage=_configMessage;
	configView->SetConfigMessage(configMessage);
}

void ConfigWindow::Quit(){
	TRACE();
	BWindow::Quit();
}

void ConfigWindow::CreateViews(){
    BOutlineListView    *chooser    = new BOutlineListView(BRect(1,1,Bounds().Width()/3,Bounds().Height()-2),"PC_Select_Categorie",B_SINGLE_SELECTION_LIST,B_FOLLOW_TOP_BOTTOM | B_FOLLOW_LEFT);
    AddChild(chooser);
}
