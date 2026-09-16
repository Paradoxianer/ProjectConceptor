#include "ProjectConceptor.h"
#include "ProjectConceptorDefs.h"
#include "PWindow.h"
#include "About/AboutWindow.h"

#include <app/Messenger.h>
#include <app/PropertyInfo.h>
#include <app/Roster.h>
#include <interface/Alert.h>
#include <interface/Rect.h>

#include <storage/Directory.h>
#include <storage/Entry.h>
#include <storage/FindDirectory.h>
#include <storage/Path.h>
#include <storage/Resources.h>

#include <string.h>
#include <stdio.h>


ProjektConceptor::ProjektConceptor():BApplication(APP_SIGNATURE) {
	TRACE();
	RegisterMime();
	// configManager has to exist before the first document/editor does -
	// GraphEditor reads shortcut bindings from it while attaching, and the
	// very first document is created below, directly in this constructor,
	// long before ReadyToRun() would otherwise run.
	BPath		settings;
	status_t	err				= B_OK;
	BDirectory	*settingsDir	= NULL;

	find_directory(B_USER_SETTINGS_DIRECTORY, &settings, true);
	settingsDir = new BDirectory(settings.Path());
	err = settingsDir->CreateDirectory("ProjectConceptor", NULL);
	err = settingsDir->SetTo(settingsDir, "ProjectConceptor");
	settings.SetTo(settingsDir, "GeneralSettings");
	configManager = new ConfigManager((char *)settings.Path());
	err = settingsDir->CreateDirectory("AutoSave", NULL);

	documentManager = new PDocumentManager();
	openPanel		= new BFilePanel();
}

ProjektConceptor::~ProjektConceptor() {
	TRACE();
	delete documentManager;
	delete openPanel;
	delete configManager;
}


// Pure specifier hop, like BWindow's own "View" entry (Window.cpp) - no
// commands/specifiers of its own to advertise (empty arrays mean "accept
// any"), the actual index handling happens in ResolveSpecifier() below.
static const property_info kAppProperties[] = {
	{ "Document", {}, {}, "The open document at the given index.", 0, {0}, {} },
};


status_t ProjektConceptor::GetSupportedSuites(BMessage *data) {
	data->AddString("suites","suite/vnd.ProjectConceptor-application");
	BPropertyInfo	propertyInfo(const_cast<property_info*>(kAppProperties));
	data->AddFlat("messages",&propertyInfo);
	return BApplication::GetSupportedSuites(data);
}


BHandler* ProjektConceptor::ResolveSpecifier(BMessage *message, int32 index,
	BMessage *specifier, int32 what, const char *property) {
	BPropertyInfo	propertyInfo(const_cast<property_info*>(kAppProperties));
	if (propertyInfo.FindMatch(message,index,specifier,what,property) >= 0) {
		if (strcmp(property,"Document") == 0) {
			int32	docIndex	= -1;
			if (what == B_INDEX_SPECIFIER)
				specifier->FindInt32("index",&docIndex);
			if ((docIndex >= 0) && (docIndex < documentManager->CountPDocuments())) {
				// PDocument runs on its own BLooper thread, not be_app's -
				// returning its pointer here the way BWindow returns
				// fTopView (same-looper hop) doesn't work: DispatchMessage()
				// expects the returned handler to belong to the *current*
				// looper. Crossing loopers means forwarding the message
				// ourselves and returning NULL, exactly how BApplication's
				// own built-in "Window N" resolution does it
				// (Application.cpp: message->PopSpecifier();
				// BMessenger(window).SendMessage(message);) - not something
				// specific to PDocument, this is the general BeOS pattern
				// for a specifier hop into a different looper.
				message->PopSpecifier();
				BMessenger(documentManager->PDocumentAt(docIndex)).SendMessage(message);
				return NULL;
			}
			BMessage	replyMsg(B_MESSAGE_NOT_UNDERSTOOD);
			replyMsg.AddInt32("error",B_BAD_INDEX);
			replyMsg.AddString("message","No document at that index");
			message->SendReply(&replyMsg);
			return NULL;
		}
	}
	return BApplication::ResolveSpecifier(message,index,specifier,what,property);
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
			be_roster->AddToRecentDocuments(&ref,APP_SIGNATURE);
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
	if (mime.IsInstalled()) {
		int32	mimeVersion	= 0;
		if ( (mime.GetAttrInfo(&info) == B_OK)
			&& (info.FindInt32("version",&mimeVersion) == B_OK)
			&& (mimeVersion >= P_C_VERSION) )
			valid = true;
		if (!valid)
			mime.Delete();
	}
	if (!valid) {
		mime.Install();
		mime.SetShortDescription("ProjectConceptor Document");
		mime.SetLongDescription("Documentfile for the ProjectConceptor");
		// the app's own icon resources are the unnamed built-in large_icon/
		// mini_icon types (used for the app itself, not this document type) -
		// the document's own icon lives in the .rdef as a separate vector
		// resource, #'HVIF' id 1, "document" - load that instead of looking
		// up a "BEOS:L:STD_ICON"/"BEOS:M:STD_ICON" named resource that was
		// never actually declared anywhere, which silently failed before.
		size_t		iconSize	= 0;
		const void	*iconData	= BApplication::AppResources()->LoadResource('HVIF',1,&iconSize);
		if (iconData != NULL)
			mime.SetIcon((const uint8 *)iconData,iconSize);
		mime.SetPreferredApp(APP_SIGNATURE);
		BMessage msg;
		msg.AddInt32("version",P_C_VERSION);
		mime.SetAttrInfo(&msg);
	}
}

int main()
{
	// stdout/stderr default to fully-buffered when redirected to a file (not
	// a TTY) - debug.log then only gets what's been flushed, which for a
	// still-running, non-crashed process can lag far behind what's actually
	// happened. Line-buffered keeps it current for live debugging.
	setvbuf(stdout, NULL, _IOLBF, 0);
	setvbuf(stderr, NULL, _IOLBF, 0);

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
