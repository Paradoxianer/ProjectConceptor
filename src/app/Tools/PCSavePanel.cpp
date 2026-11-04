#include <interface/TextControl.h>
#include <interface/Window.h>
#include <storage/MimeType.h>
#include <translation/TranslatorFormats.h>
#include <support/Autolock.h>
#include <Catalog.h>
#include <string.h>
#include <strings.h>

#include "PCSavePanel.h"
#include "ProjectConceptorDefs.h"
//#include "BasePlugin.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "SavePanel"

// sent by the format menu's items
static const uint32	kFormatChanged	= 'PCfc';


/** the panel's window handler for the format menu: BFilePanel is no
 * BHandler itself */
class FormatChangedHandler : public BHandler
{
public:
					FormatChangedHandler(PCSavePanel *panel)
						: BHandler("FormatChangedHandler"), panel(panel) {}
	virtual	void	MessageReceived(BMessage *message)
	{
		if (message->what == kFormatChanged)
			panel->FormatChanged();
		else
			BHandler::MessageReceived(message);
	}
private:
	PCSavePanel		*panel;
};


PCSavePanel::PCSavePanel(BMessage *msg,  BMessenger* target, bool exporting)
	: BFilePanel(B_SAVE_PANEL,target,NULL,B_FILE_NODE,false,msg,NULL,false,true),
	exporting(exporting)
{
	float 	height		= 74;
	Window()->Lock();
	BView 	*background = Window()->ChildAt(0);
	BView	*textView;
	Window()->SetTitle(exporting ? B_TRANSLATE("Export") : B_TRANSLATE("Save as"));
	if (exporting)
		SetButtonLabel(B_DEFAULT_BUTTON, B_TRANSLATE("Export"));
	formatHandler	= new FormatChangedHandler(this);
	Window()->AddHandler(formatHandler);


	BRect limit 				= background->Bounds();
	limit.bottom				= height;
	BMenu		*format_menu	= BuildFormatsMenu();
	// Default to the native ProjectConceptor format, not whatever translator
	// GetAllTranslators() happens to enumerate first (e.g. FreeMind) - saving
	// without touching the format menu should never silently export into an
	// unrelated format.
	BMenuItem	*defaultItem	= NULL;
	for (int32 i=0; i<format_menu->CountItems(); i++) {
		BMenuItem	*item	= format_menu->ItemAt(i);
		int32		type	= 0;
		if ( (item->Message() != NULL)
			&& (item->Message()->FindInt32(P_C_FORMAT_TYPE,&type) == B_OK)
			&& (type == P_C_DOCUMENT_TYPE) ) {
			defaultItem = item;
			break;
		}
	}
	if (defaultItem == NULL)
		defaultItem = format_menu->ItemAt(0);
	if (defaultItem != NULL)
		defaultItem->SetMarked(true);
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


PCSavePanel::~PCSavePanel()
{
	if (Window()->Lock()) {
		Window()->RemoveHandler(formatHandler);
		Window()->Unlock();
	}
	delete formatHandler;
}


// the file name extension the translator gives for type, or ""
static BString FormatExtension(BTranslatorRoster *roster, translator_id id,
	uint32 type)
{
	BMessage	config;
	if (roster->GetConfigurationMessage(id, &config) != B_OK)
		return BString();
	int32		configType;
	const char	*extension;
	for (int32 i = 0; config.FindInt32(P_C_FORMAT_TYPE, i, &configType) == B_OK; i++) {
		if (((uint32)configType == type)
			&& (config.FindString(P_C_FORMAT_EXTENSION, i, &extension) == B_OK))
			return BString(extension);
	}
	return BString();
}


std::vector<BMessage> PCSavePanel::Formats(bool exporting)
{
	BTranslatorRoster			*roster				= BTranslatorRoster::Default();
	int32						num_translators		= 0;
	int32						i,q					= 0;
	int32						outNum				= 0;
	int32						inNum				= 0;
	translator_id				*translators		= NULL;
	const	translation_format	*oFormats;
	const	translation_format	*iFormats;
	std::vector<BMessage>		formats;
	roster->GetAllTranslators(&translators, &num_translators);
	for (i=0;i<num_translators;i++)  {
		roster->GetInputFormats(translators[i], &iFormats, &inNum);
		bool	readsDocument	= false;
		for (q=0;q<inNum;q++)
			readsDocument |= (iFormats[q].type == P_C_DOCUMENT_RAW_TYPE);
		if (!readsDocument)
			continue;
		roster->GetOutputFormats(translators[i], &oFormats, &outNum);
		for (int j=0; j<outNum; j++) {
			if (oFormats[j].type == P_C_DOCUMENT_RAW_TYPE)
				continue;
			// a document is saved in its own formats only, which keep
			// everything; the others are exports, opened as imports
			bool	native	= (oFormats[j].type == P_C_DOCUMENT_TYPE)
				|| (oFormats[j].type == P_C_DOCUMENT_TEXT_TYPE);
			if (native == exporting)
				continue;
			BMessage	format(kFormatChanged);
			format.AddString(P_C_FORMAT_NAME,oFormats[j].name);
			format.AddString(P_C_FORMAT_MIME,oFormats[j].MIME);
			format.AddInt32(P_C_FORMAT_TYPE,oFormats[j].type);
			format.AddString(P_C_FORMAT_EXTENSION,
				FormatExtension(roster, translators[i], oFormats[j].type));
			formats.push_back(format);
		}
	}
	// an export can also be a picture: every format the system's bitmap
	// translators write
	for (i=0; exporting && (i<num_translators); i++) {
		roster->GetInputFormats(translators[i], &iFormats, &inNum);
		bool	readsBitmaps	= false;
		for (q=0;q<inNum;q++)
			readsBitmaps |= (iFormats[q].type == B_TRANSLATOR_BITMAP);
		if (!readsBitmaps)
			continue;
		roster->GetOutputFormats(translators[i], &oFormats, &outNum);
		for (int j=0; j<outNum; j++) {
			if ((oFormats[j].type == B_TRANSLATOR_BITMAP)
				|| (oFormats[j].group != B_TRANSLATOR_BITMAP))
				continue;
			BMessage	extensions;
			const char	*extension	= "";
			BMimeType	mime(oFormats[j].MIME);
			if (mime.GetFileExtensions(&extensions) == B_OK)
				extension	= extensions.GetString("extensions", "");
			BMessage	format(kFormatChanged);
			format.AddString(P_C_FORMAT_NAME,oFormats[j].name);
			format.AddString(P_C_FORMAT_MIME,oFormats[j].MIME);
			format.AddInt32(P_C_FORMAT_TYPE,oFormats[j].type);
			format.AddString(P_C_FORMAT_EXTENSION,extension);
			format.AddBool(P_C_FORMAT_BITMAP,true);
			formats.push_back(format);
		}
	}
	delete [] translators;
	return formats;
}


status_t PCSavePanel::FindFormat(const char *extension, BMessage *format)
{
	for (int32 exporting = 0; exporting < 2; exporting++) {
		std::vector<BMessage>	formats	= Formats(exporting != 0);
		for (size_t i = 0; i < formats.size(); i++) {
			if (strcasecmp(formats[i].GetString(P_C_FORMAT_EXTENSION,""),extension) == 0) {
				*format	= formats[i];
				return B_OK;
			}
		}
	}
	return B_NAME_NOT_FOUND;
}


BMenu *PCSavePanel::BuildFormatsMenu(void)
{
	BMenu					*menu		= new BMenu(B_TRANSLATE("File format"));
	std::vector<BMessage>	formats		= Formats(exporting);
	bool					bitmaps		= false;
	for (size_t i = 0; i < formats.size(); i++) {
		if (formats[i].GetBool(P_C_FORMAT_BITMAP,false) && !bitmaps) {
			menu->AddSeparatorItem();
			bitmaps	= true;
		}
		BMenuItem	*item	= new BMenuItem(formats[i].GetString(P_C_FORMAT_NAME,""),
			new BMessage(formats[i]));
		item->SetTarget(formatHandler);
		menu->AddItem(item);
	}
	if (menu->CountItems() == 0)
		menu->AddItem(new BMenuItem(B_TRANSLATE("Could not find plugins"),NULL));
	return menu;
}


void PCSavePanel::FormatChanged(void)
{
	BAutolock	lock(Window());
	BMenuItem	*marked		= formatMenu->Menu()->FindMarked();
	const char	*extension	= NULL;
	BTextControl	*nameControl	= dynamic_cast<BTextControl*>(
		Window()->FindView("text view"));
	if ((marked == NULL) || (marked->Message() == NULL) || (nameControl == NULL)
		|| (marked->Message()->FindString(P_C_FORMAT_EXTENSION,&extension) != B_OK))
		return;
	// the name keeps what was typed, only the format's extension changes
	BString	name(nameControl->Text());
	int32	dot	= name.FindLast('.');
	if ((dot > 0) && (lastExtension.Length() > 0)
		&& (strcmp(name.String() + dot + 1, lastExtension.String()) == 0))
		name.Truncate(dot);
	if (extension[0] != '\0')
		name << "." << extension;
	lastExtension	= extension;
	SetSaveText(name.String());
	// the panel enables it only on a key or click (TFilePanel::AdjustButton)
	BButton	*button	= dynamic_cast<BButton*>(Window()->FindView("default button"));
	if (button != NULL)
		button->SetEnabled(name.Length() > 0);
}


void	PCSavePanel::SendMessage(const BMessenger* messenger, BMessage *message)
{
	BMenuItem *selected = formatMenu->Menu()->FindMarked();
	if ( (selected) && (selected->Message()) ) {
		message->AddMessage("saveSettings",selected->Message());
	}
	BFilePanel::SendMessage(messenger,message);
}
