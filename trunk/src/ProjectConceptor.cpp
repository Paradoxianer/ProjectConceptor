#include <app/Roster.h>
#include <interface/Alert.h>
#include <interface/Rect.h>

#include <storage/Entry.h>
#include <storage/Path.h>
#include <translation/TranslationUtils.h>

#include <string.h>

#include "ProjectConceptor.h"
#include "ProjectConceptorDefs.h"
#include "PWindow.h"
#include "AboutWindow.h"




const char		*APP_SIGNATURE		= "application/ProjectConceptor";

const char*		P_MENU_APP_HELP					= "Help";
const char*		P_MENU_APP_ABOUT				= "About";
const char*		P_MENU_APP_SETTINGS				= "Settings";
const char*		P_MENU_APP_QUIT					= "Quit";



const char*		P_M_MENU_BAR					= "P_M_MENU_BAR";
/**string with wich you can find the correspondenting BMenubar wich is used for Status menus and information 
 *@see BMenuBar
 */
const char*		P_M_STATUS_BAR					= "P_M_STATUS_BAR";
/**string with wich you can find the correspondenting ToolBar for all this things like open, save... 
 *@see ToolBar
 */
const char*		P_M_STANDART_TOOL_BAR			= "P_M_STANDART_TOOL_BAR";
/**string with wich you can find the correspondenting ToolBar for all formating stuff 
 *@see ToolBar
 */
const char*		P_M_FORMAT_TOOL_BAR				= "P_M_FORMAT_TOOL_BAR";
/**string with wich you can find the correspondenting ToolBar for Editor related ToolItems
 *@see ToolBar
 */
const char*		P_M_EDITOR_TOOL_BAR				= "P_M_EDITOR_TOOL_BAR";


/**value to find the BMenu "File" over the PMenuAcces Interface
 *@see PMenuAcces
 */	
const char*		P_MENU_FILE						= "File";

/**value to find the BMenu Item "File->New" over the PMenuAcces Interface
 *@see PMenuAcces
 */	
const char*		P_MENU_FILE_NEW					= "New";
const char*		P_MENU_FILE_NEW_TAB				= "New tab";
/**value to find the BMenu Item "File->Open" over the PMenuAcces Interface
 *@see PMenuAcces
 */	
const char*		P_MENU_FILE_OPEN				= "Open";
/**value to find the BMenu Item "File->Close" over the PMenuAcces Interface
 *@see PMenuAcces
 */	
const char*		P_MENU_FILE_CLOSE				= "Close";
/**value to find the BMenu Item "File->Save" over the PMenuAcces Interface
 *@see PMenuAcces
 */	
const char*		P_MENU_FILE_SAVE				= "Save";
/**value to find the BMenu Item "File->Save As" over the PMenuAcces Interface
 *@see PMenuAcces
 */	
const char*		P_MENU_FILE_SAVE_AS				= "Save as";
/**value to find the BMenu Item "File->PageSetup" over the PMenuAcces Interface
 *@see PMenuAcces
 */	
 
const char*		P_MENU_FILE_EXPORT				= "Export";

const char*		P_MENU_FILE_PAGESETUP			= "Page setup";
/**value to find the BMenu Item "File->Print" over the PMenuAcces Interface
 *@see PMenuAcces
 */	
const char*		P_MENU_FILE_PRINT				= "Print";
/**value to find the BMenu Item "File->Quit" over the PMenuAcces Interface
 *@see PMenuAcces
 */	
const char*		P_MENU_FILE_QUIT				= "Quit";
/**value to find the BMenu "Edit" over the PMenuAcces Interface
 *@see PMenuAcces
 */	

const char*		P_MENU_EDIT						= "Edit";
const char*		P_MENU_EDIT_UNDO				= "Undo";
const char*		P_MENU_EDIT_REDO				= "Redo";

const char*		P_MENU_EDIT_CUT					= "Cut";
const char*		P_MENU_EDIT_COPY				= "Copy";
const char*		P_MENU_EDIT_PASTE				= "Paste";

const char*		P_MENU_EDIT_CLEAR				= "Clear";
const char*		P_MENU_EDIT_SELECT_ALL			= "Select all";

const char*		P_MENU_EDIT_PROJECT_SETTINGS	= "Project settings";

const char*		P_MENU_SEARCH					= "Search";
const char*		P_MENU_SEARCH_FIND				= "Find";
const char*		P_MENU_SEARCH_FIND_NEXT			= "Find next";
const char*		P_MENU_SEARCH_REPLACE			= "Replace";
const char*		P_MENU_SEARCH_REPLACE_AND_FIND	= "Find & replace";
const char*		P_MENU_SEARCH_REPLACE_ALL		= "Replace all";

/**value to find the BMenu "Window" over the PMenuAcces Interface
 *@see PMenuAcces
 */	

const char*		P_MENU_WINDOW					= "Window";
const char*		P_MENU_WINDOW_TITLE				= "Tile";
const char*		P_MENU_WINDOW_TITLE_VERTICAL	= "Tile vertical";
const char*		P_MENU_WINDOW_CASCADE			= "Cascade";


/**value to find the BMenu "Makrko" over the PMenuAcces Interface
 *@see PMenuAcces
 */	
const char*		P_MENU_MACRO					= "Macro";
const char*		P_MENU_MACRO_START_RECORDING	= "Start recording";
const char*		P_MENU_MACRO_STOP_RECORDING		= "Stop recording";
const char*		P_MENU_MACRO_PLAY				= "Play";
const char*		P_MENU_MACRO_OPEN				= "Open";
const char*		P_MENU_MACRO_SAVE				= "Save";



const char*		P_C_VERSION						= "0.01.1 Revision 82";

ProjektConceptor::ProjektConceptor():BApplication(APP_SIGNATURE)
{
	TRACE();
	RegisterMime();
	documentManager = new PDocumentManager();
	openPanel		= new BFilePanel();
}

ProjektConceptor::~ProjektConceptor()
{
	TRACE();
	delete documentManager;
	delete openPanel;
}

void ProjektConceptor::ReadyToRun()
{
	TRACE();
}

/**
 * @todo request the Quit .. don´t simply quit all without asking (so that there is chance to save or abort because the document has changed)
 */
bool ProjektConceptor::QuitRequested()
{
	TRACE();
	bool quit	= true;
	for (int32 i=0;i<documentManager->CountPDocuments();i++)
	{
		PDocument * doc=documentManager->PDocumentAt(i);
		quit= quit | doc->QuitRequested();
/*		doc->Lock();
		doc->Quit();*/
	}
	return quit;
}

void ProjektConceptor::MessageReceived(BMessage *message) 
{
	TRACE();
	switch(message->what)
	{
		case MENU_FILE_OPEN:
		{
/*			Documenter *tester;
			documentPlugins->FindPointer("plugins",(void **)&tester);
			PWindow *prjWindow=(PWindow *)WindowAt(0);
			tester->SetRenderPlugins(pluginManager->GetPluginsByKindAndType(tester->GetName(),P_C_RENDERER));
			prjWindow->MakeNewDocument(tester);*/
			openPanel->Show();		// Show the file panel
			break;
		}
		default:
			BApplication::MessageReceived(message);
			break;
	}
}

void ProjektConceptor::RefsReceived(BMessage *msg)
{
	TRACE();
	uint32 		type;
	int32 		count;
	BEntry		*entry=new BEntry();
	entry_ref	ref;

	msg->GetInfo("refs", &type, &count);
	
	// not a entry_ref?
	if (type != B_REF_TYPE)
	{
		delete entry;
		return;
	}
	
	if (msg->FindRef("refs", 0, &ref) == B_OK)
		if (entry->SetTo(&ref,true)==B_OK)
		{
			PDocument *doc=documentManager->PDocumentAt(0);
			doc->SetEntry(&ref);
			doc->Load();
			// entry is ok. use it here.
		}
	delete entry;
}

void ProjektConceptor::AboutRequested()
{
	TRACE();
//	BAlert *alert=new BAlert("about","ProjektConceptor v0.1\n\nCopyright by Author","Oh.");
	AboutWindow *aboutWindow = new AboutWindow();
	aboutWindow->Show();
}

void ProjektConceptor::ArgvReceived(int32 argc, char **argv)
{
	if (argc>1)
	{
		if (strcasecmp(argv[1],"-d") == B_OK)
			SET_DEBUG_ENABLED(true);
	}
	else
			SET_DEBUG_ENABLED(false);
}

void ProjektConceptor::RegisterMime(void)
{
	BMimeType		mime;
	mime.SetType(P_C_DOCUMENT_MIMETYPE);
	if (!mime.IsInstalled()) 
	{
		mime.Install();
		mime.SetShortDescription("ProjectConceptor Document");
		mime.SetLongDescription("Documentfile for the ProjectConceptor");
		mime.SetPreferredApp(APP_SIGNATURE);
	}

}

int main()
{
//	SET_DEBUG_ENABLED(false);
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
