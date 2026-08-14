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
#include <String.h>
#include <stdlib.h>


#include "ConfigManager.h"
#include "ConfigView.h"
#include "ProjectConceptor.h"
#include "ProjectConceptorDefs.h"

//#include "MessageItem.h"

ConfigView::ConfigView(BMessage *_configMessage):BGridView(10,10){
        TRACE();
        configMessage=_configMessage;
        Init();
}

void ConfigView::Init(){
    BGridLayout     *gridLayout = GridLayout();
    autoSaveCheckBox        = new BCheckBox(NULL,new BMessage(AUTOSAVE_TOGGLED));
    autoSaveIntervalControl = new BTextControl("AutoSave","Autosaveintervall", "5", new BMessage(AUTOSAVE_CHANGED));
    rememberWindowFrameCheckBox = new BCheckBox(NULL,new BMessage(REMEMBER_WINDOW_FRAME_TOGGLED));

    // row 1
   gridLayout->AddView(new BStringView("CheckBoxLabel","Autosave enabled"), 0, 0);
   gridLayout->AddView(autoSaveCheckBox, 1, 0);

   //row 2
   gridLayout->AddItem(autoSaveIntervalControl->CreateLabelLayoutItem(), 0, 1);
   gridLayout->AddItem(autoSaveIntervalControl->CreateTextViewLayoutItem(), 1, 1);

   //row 3
   gridLayout->AddView(new BStringView("RememberWindowLabel","Remember window size/position"), 0, 2);
   gridLayout->AddView(rememberWindowFrameCheckBox, 1, 2);

   gridLayout->AddItem(BSpaceLayoutItem::CreateGlue(),0,3);
}

void ConfigView::AttachedToWindow(void){
	BGridView::AttachedToWindow();
	// controls default their invocation target to the window; route them
	// to this view instead so MessageReceived() below actually sees them
	autoSaveCheckBox->SetTarget(this);
	autoSaveIntervalControl->SetTarget(this);
	rememberWindowFrameCheckBox->SetTarget(this);
}

void ConfigView::ChangeLanguage(){
	TRACE();
}

void ConfigView::SetConfigMessage(BMessage *configureMessage, BMessenger docTarget){
	TRACE();
	configMessage	= configureMessage;
	documentTarget	= docTarget;
	ValueChanged();
}

void ConfigView::ValueChanged(void){
	TRACE();
	// app-wide, independent of configMessage (which is per-document) -
	// refreshed here too since this runs whenever the settings window is
	// shown, same as the per-document fields below
	ConfigManager	*configManager		= ((ProjektConceptor*)be_app)->GetConfigManager();
	BMessage		*frameConfig		= configManager->GetConfigMessage(P_C_CONFIG_WINDOW_FRAME_FIELD);
	bool			rememberEnabled		= true;
	if (frameConfig != NULL) {
		frameConfig->FindBool(P_C_WINDOW_FRAME_REMEMBER_FIELD,&rememberEnabled);
		delete frameConfig;
	}
	rememberWindowFrameCheckBox->SetValue(rememberEnabled ? B_CONTROL_ON : B_CONTROL_OFF);

	if (configMessage == NULL)
		return;
	bool	enabled		= true;
	int32	interval	= 300;
	configMessage->FindBool(P_C_DOC_AUTOSAVE_ENABLED,&enabled);
	configMessage->FindInt32(P_C_DOC_AUTOSAVE_INTERVAL,&interval);
	autoSaveCheckBox->SetValue(enabled ? B_CONTROL_ON : B_CONTROL_OFF);
	autoSaveIntervalControl->SetEnabled(enabled);
	BString intervalText;
	intervalText << interval;
	autoSaveIntervalControl->SetText(intervalText.String());
}


void ConfigView::BuildConfigList(BMessage *confMessage, BListItem *parentItem){

}

void ConfigView::MessageReceived(BMessage *msg){
	switch (msg->what) {
		case AUTOSAVE_TOGGLED: {
			bool enabled = (autoSaveCheckBox->Value() == B_CONTROL_ON);
			if (configMessage->ReplaceBool(P_C_DOC_AUTOSAVE_ENABLED,enabled) != B_OK)
				configMessage->AddBool(P_C_DOC_AUTOSAVE_ENABLED,enabled);
			autoSaveIntervalControl->SetEnabled(enabled);
			documentTarget.SendMessage(P_C_DOC_SETTINGS_CHANGED);
			break;
		}
		case REMEMBER_WINDOW_FRAME_TOGGLED: {
			bool			enabled			= (rememberWindowFrameCheckBox->Value() == B_CONTROL_ON);
			ConfigManager	*configManager	= ((ProjektConceptor*)be_app)->GetConfigManager();
			BMessage		*frameConfig	= configManager->GetConfigMessage(P_C_CONFIG_WINDOW_FRAME_FIELD);
			BMessage		newFrameConfig;
			if (frameConfig != NULL) {
				newFrameConfig	= *frameConfig;
				delete frameConfig;
			}
			if (newFrameConfig.ReplaceBool(P_C_WINDOW_FRAME_REMEMBER_FIELD,enabled) != B_OK)
				newFrameConfig.AddBool(P_C_WINDOW_FRAME_REMEMBER_FIELD,enabled);
			configManager->SetConfigMessage(P_C_CONFIG_WINDOW_FRAME_FIELD,&newFrameConfig);
			configManager->SaveConfig();
			break;
		}
		case AUTOSAVE_CHANGED: {
			int32 interval = atoi(autoSaveIntervalControl->Text());
			if (interval <= 0)
				interval = 300;
			if (configMessage->ReplaceInt32(P_C_DOC_AUTOSAVE_INTERVAL,interval) != B_OK)
				configMessage->AddInt32(P_C_DOC_AUTOSAVE_INTERVAL,interval);
			documentTarget.SendMessage(P_C_DOC_SETTINGS_CHANGED);
			break;
		}
		case MESSAGE_SELECTED: {
/*			MessageItem	*selectedItem= dynamic_cast<MessageItem*> (configList->ItemAt(configList->CurrentSelection()));
			if (selectedItem != NULL)
				showMessage->SetConfigMessage(selectedItem->Message());*/
			break;
		}
		default:
			BGridView::MessageReceived(msg);
			break;
	}
}
