#include "ProjectConceptor.h"
#include "ProjectConceptorDefs.h"
#include "PWindow.h"
#include "About/AboutWindow.h"

#include <app/Roster.h>
#include <interface/Alert.h>
#include <interface/Rect.h>

#include <storage/Directory.h>
#include <storage/Entry.h>
#include <storage/FindDirectory.h>
#include <storage/Path.h>
#include <translation/TranslationUtils.h>

#include <string.h>


ProjektConceptor::ProjektConceptor():BApplication(APP_SIGNATURE) {
	TRACE();
	RegisterMime();
	documentManager = new PDocumentManager();
	openPanel		= new BFilePanel();
}

ProjektConceptor::~ProjektConceptor() {
	TRACE();
	delete documentManager;
	delete openPanel;
}

void ProjektConceptor::ReadyToRun() {
	TRACE();
	//creat settingsfolder
	BPath		settings;
	status_t	err				= B_OK;
	BDirectory	*settingsDir	= NULL;
	
	find_directory(B_USER_SETTINGS_DIRECTORY, &settings, true);
	settingsDir = new BDirectory(settings.Path());
	err = settingsDir->CreateDirectory("ProjectConceptor", NULL);
	err = settingsDir->SetTo(settingsDir, "ProjectConceptor");
	settings.SetTo(settingsDir, "GeneralSettings");
	ConfigManager		*configManager = new ConfigManager((char *)settings.Path());
	configManager->LoadConfig();
	err = settingsDir->CreateDirectory("AutoSave", NULL);
}

/**
 * @todo request the Quit .. don´t simply quit all without asking (so that there is chance to save or abort because the document has changed)
 */
bool ProjektConceptor::QuitRequested() {
	TRACE();
	bool quit	= true;
	for (int32 i=0;i<documentManager->CountPDocuments();i++) {
		PDocument * doc=documentManager->PDocumentAt(i);
		quit = quit | doc->QuitRequested();
	}
	return quit;
}

void ProjektConceptor::MessageReceived(BMessage *message) {
	TRACE();
	switch(message->what) {
		case MENU_FILE_OPEN: {
/*			Documenter *tester;
			documentPlugins->FindPointer("plugins",(void **)&tester);
			PWindow *prjWindow=(PWindow *)WindowAt(0);
			tester->SetRenderPlugins(pluginManager->GetPluginsByKindAndType(tester->GetName(),P_C_RENDERER));
			prjWindow->MakeNewDocument(tester);*/
			openPanel->Show();		// Show the file panel
			break;
		}
		case MENU_FILE_NEW: {
			documentManager->CreateDocument();
			break;
		}
		default:
			BApplication::MessageReceived(message);
			break;
	}
}

void ProjektConceptor::RefsReceived(BMessage *msg) {
	TRACE();
	uint32 		type;
	int32 		count;
	BEntry		*entry=new BEntry();
	entry_ref	ref;

	msg->GetInfo("refs", &type, &count);

	// not a entry_ref?
	if (type != B_REF_TYPE) {
		delete entry;
		return;
	}

	if (msg->FindRef("refs", 0, &ref) == B_OK)
		if (entry->SetTo(&ref,true)==B_OK) {
			PDocument *doc=documentManager->PDocumentAt(0);
			doc->SetEntry(&ref);
			doc->Load();
		}
	delete entry;
}

void ProjektConceptor::AboutRequested() {
	TRACE();
	AboutWindow *aboutWindow = new AboutWindow();
	aboutWindow->Show();
}

void ProjektConceptor::ArgvReceived(int32 argc, char **argv) {
	if (argc>1) {
		BEntry ref(argv[1]);
		if (ref.Exists()){
				PDocument *doc=documentManager->PDocumentAt(0);
				if (doc ==NULL)
					doc = documentManager->CreateDocument();
				if (doc !=NULL){
					entry_ref entry=entry_ref();
					ref.GetRef(&entry);
					doc->SetEntry(&entry);
					doc->Load();
				}
				else
					printf("Error creating the document");
		}
		else
			printf("Could not load %s",argv[1]);
	}
}

void ProjektConceptor::RegisterMime(void) {
	bool			valid = false;
	BMimeType		mime;
	BMessage		info;
	mime.SetType(P_C_DOCUMENT_MIMETYPE);
	if (!mime.IsInstalled()) {
		if (mime.GetAttrInfo(&info) == B_NO_ERROR)  {
			int32	mimeVersion;
			info.FindInt32("version",&mimeVersion);
			if (mimeVersion<P_C_VERSION);
				valid = false;
		}
		if (!valid)
			mime.Delete();
	}
	if (!valid) {
		BBitmap *kLargeIcon= BTranslationUtils::GetBitmap(B_LARGE_ICON,"BEOS:L:STD_ICON");
		BBitmap *kSmallIcon= BTranslationUtils::GetBitmap(B_MINI_ICON,"BEOS:M:STD_ICON");

		mime.Install();
		mime.SetShortDescription("ProjectConceptor Document");
		mime.SetLongDescription("Documentfile for the ProjectConceptor");
		if (kLargeIcon)
			mime.SetIcon(kLargeIcon, B_LARGE_ICON);
		if (kLargeIcon)
			mime.SetIcon(kSmallIcon, B_MINI_ICON);
		mime.SetPreferredApp(APP_SIGNATURE);
		BMessage msg;
		msg.AddInt32("version",P_C_VERSION);
		mime.SetAttrInfo(&msg);
	}
}

int main()
{

	new ProjektConceptor();
	/*	freopen ("/boot/var/log/ProjectConceptor.log","w",stdout);
	freopen ("/boot/var/log/ProjectConceptor.log","a+",stderr);*/
	be_app->Run();
	delete be_app;
/*	fflush (stdout);
	fflush (stderr);
	fclose (stdout);
	fclose (stderr);*/
	return 0;
}
