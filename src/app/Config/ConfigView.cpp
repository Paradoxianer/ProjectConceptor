#include <CheckBox.h>
#include <GroupLayoutBuilder.h>
#include <GridLayout.h>
#include <GridLayoutBuilder.h>
#include <GridView.h>
#include <TextControl.h>
#include <SpaceLayoutItem.h>
#include <ScrollView.h>
#include <Slider.h>
#include <StringView.h>


#include "ConfigView.h"
#include "ProjectConceptorDefs.h"

//#include "MessageItem.h"

ConfigView::ConfigView(BMessage *_configMessage):BGridView(10,10){
        TRACE();
        configMessage=_configMessage;
        Init();
}

void ConfigView::Init(){
    BGridLayout     *gridLayout = GridLayout();
    BCheckBox       *checkBox   = new BCheckBox(NULL,new BMessage(AUTOSAVE_TOGGLED));
    BTextControl    *textControl = new BTextControl("AutoSave","Autosaveintervall", "5", new BMessage(AUTOSAVE_CHANGED));
 //   AddChild(textControl);

    // row 1
   gridLayout->AddView(new BStringView("CheckBoxLabel","Autosave enabled"), 0, 0);
   gridLayout->AddView(checkBox, 1, 0);

   //row 2
   gridLayout->AddItem(textControl->CreateLabelLayoutItem(), 0, 1);
   gridLayout->AddItem(textControl->CreateTextViewLayoutItem(), 1, 1);
   gridLayout->SetInsets(10,10,10,10);
}

void ConfigView::ChangeLanguage(){
	TRACE();
}

void ConfigView::SetConfigMessage(BMessage *configureMessage){
	TRACE();
	configMessage=configureMessage;
	ValueChanged();
}

void ConfigView::ValueChanged(void){
	TRACE();
}


void ConfigView::BuildConfigList(BMessage *confMessage, BListItem *parentItem){

}

void ConfigView::MessageReceived(BMessage *msg){
	if (msg->what == MESSAGE_SELECTED){
/*		MessageItem	*selectedItem= dynamic_cast<MessageItem*> (configList->ItemAt(configList->CurrentSelection()));
		if (selectedItem != NULL)
			showMessage->SetConfigMessage(selectedItem->Message());*/
	}
}
