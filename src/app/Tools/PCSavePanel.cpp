#include <interface/Window.h>
#include <Catalog.h>

#include "PCSavePanel.h"
#include "BasePlugin.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "SavePanel"



PCSavePanel::PCSavePanel(BMessage *msg,  BMessenger* target): BFilePanel(B_SAVE_PANEL,target,NULL,B_FILE_NODE,false,msg,NULL,false,true)
{
	float 	height		= 74;
	Window()->Lock();
	BView 	*background = Window()->ChildAt(0);
	BView	*textView;
	Window()->SetTitle(B_TRANSLATE("Save as"));


	BRect limit 				= background->Bounds();
	limit.bottom				= height;
	BMenu		*format_menu	= BuildFormatsMenu();
	format_menu->ItemAt(0)->SetMarked(true);
	format_menu->SetLabelFromMarked(true);
	BRect rect;
	textView=Window()->FindView("text view");
	if(textView!=NULL) {
		rect=textView->ConvertToParent(textView->Bounds());
	}
	else {
		//use view coordinates to set base for drop-down menu
		BRect taille=background->Bounds();
		float x_center=taille.right-100;
		rect.Set(x_center,   taille.top+20, x_center+126,    taille.top+20+12  );
		rect.OffsetBy(-20,2);
	}

	formatMenu = new BMenuField(rect,"",NULL,format_menu,B_FOLLOW_BOTTOM | B_FOLLOW_LEFT ,B_WILL_DRAW);
	background->AddChild(formatMenu);
	textView->MoveTo(formatMenu->Frame().left,formatMenu->Frame().bottom+15);
	textView->SetResizingMode(B_FOLLOW_NONE);
	formatMenu->SetResizingMode(B_FOLLOW_NONE);
	background->FindView("PoseView")->SetResizingMode(B_FOLLOW_NONE);
	background->FindView("CountVw")->SetResizingMode(B_FOLLOW_NONE);
	background->FindView("HScrollBar")->SetResizingMode(B_FOLLOW_NONE);
	Window()->ResizeTo(background->Bounds().Width(),textView->Frame().bottom+10);
	textView->SetResizingMode(B_FOLLOW_LEFT|B_FOLLOW_BOTTOM);
	formatMenu->SetResizingMode(B_FOLLOW_BOTTOM | B_FOLLOW_LEFT );
	background->FindView("PoseView")->SetResizingMode(B_FOLLOW_ALL_SIDES);
	
	BRect sRect=background->FindView("cancel button")->Frame();
	sRect.OffsetTo(textView->Frame().right+10,sRect.top);
	settings	= new BButton(sRect,"Settings",B_TRANSLATE("Settings"),NULL);
	background->AddChild(settings);
	Window()->Unlock();
}

BMenu *PCSavePanel::BuildFormatsMenu(void)
{
	// Builds a menu with all the possible file types
	BTranslatorRoster			*roster				= BTranslatorRoster::Default();
	int32						num_translators		= 0;
	int32						i,q					= 0;
	int32						outNum				= 0;
	int32						inNum				= 0;
	translator_id				*translators		= NULL;
	const	translation_format	*oFormats;
	const	translation_format	*iFormats;
	translation_format			*allFormats			= NULL;
	BMessage					*message			= NULL;
	BMenu						*menu				= new BMenu(B_TRANSLATE("File format") );
	BMenuItem					*item				= NULL;
	roster->GetAllTranslators(&translators, &num_translators);
	allFormats	=	new translation_format[num_translators];
	for (i=0;i<num_translators;i++)  {
		roster->GetInputFormats(translators[i], &iFormats, &inNum);
		for (q=0;q<inNum;q++)  {
			if (iFormats[q].type == P_C_DOCUMENT_RAW_TYPE) {
				roster->GetOutputFormats(translators[i], &oFormats, &outNum);
				for (int j=0; j<outNum; j++) {
					//	and take the first output format that isn't P_C_DOCUMENT_RAW_TYPE
					if( oFormats[j].type != P_C_DOCUMENT_RAW_TYPE) {
						message	= new BMessage();
						message->AddInt32("translator_id",translators[i]);
						message->AddString("format::name",oFormats[j].name);
						message->AddString("format::MIME",oFormats[j].MIME);
						message->AddInt32("format::type",oFormats[j].type);
						message->AddInt32("format::group",oFormats[j].group);
						message->AddFloat("format::quality",oFormats[j].quality);
						message->AddFloat("format::capability",oFormats[j].capability);
						item = new BMenuItem(oFormats[j].name, message);
						menu->AddItem( item );
					}
				}
			}
		}
	}
	delete [] translators; 
	if (menu->CountItems() == 0)
		menu->AddItem(new BMenuItem(B_TRANSLATE("Could not find plugins"),NULL));
	return menu;
}

void	PCSavePanel::SendMessage(const BMessenger* messenger, BMessage *message)
{
	BMenuItem *selected = formatMenu->Menu()->FindMarked();
	if ( (selected) && (selected->Message()) ) {
		message->AddMessage("saveSettings",selected->Message());
	}
	BFilePanel::SendMessage(messenger,message);
}
