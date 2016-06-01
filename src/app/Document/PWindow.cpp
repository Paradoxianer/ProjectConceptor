#include <interface/MenuItem.h>
#include <interface/ScrollBar.h>
#include <interface/ScrollView.h>
#include <stdio.h>
#include <translation/TranslationUtils.h>
#include <translation/TranslatorFormats.h>
#include <BeBuild.h>
#include <Catalog.h>

#include "BasePlugin.h"
#include "ProjectConceptorDefs.h"
#include "PWindow.h"
#include "PEditor.h"
#include "PEditorManager.h"
#include "ProjectConceptor.h"
#include "ToolItem.h"
#include "ToolMenu.h"
#include "MainView.h"




#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MainWindow"


PWindow::PWindow(BRect rect,PDocument *document):BWindow(rect,"ProjectConceptor",B_TITLED_WINDOW,0)
{
	TRACE();
	doc	= document;
	Init();
	Show();
}

PWindow::PWindow(BMessage *archive):BWindow(archive)
{
	TRACE();
}


PWindow::~PWindow(void)
{
	TRACE();
}

/**
 *@todo find the "standart" Editor and insert them as default
 */
void PWindow::Init(void)
{
	TRACE();
	oldShortcutMessage		= new BMessage();;
	mainView				= NULL;
	P_M_MAIN_VIEW_LEFT		= 0.0;
	P_M_MAIN_VIEW_TOP		= 0.0;
	P_M_MAIN_VIEW_BOTTOM	= Bounds().Height();
	P_M_MAIN_VIEW_RIGHT		= Bounds().Width();
	AddChild(menuBar		= MakeMenu());
	AddChild(statusBar		= MakeStatusBar());

	verticalToolbars		= new BList();
	horizontalToolbars		= new BList();
	MakeToolbars();

	SetSizeLimits(300,6000,150,4000);
	BRect containerRect		= BRect(P_M_MAIN_VIEW_LEFT+1,P_M_MAIN_VIEW_TOP+2,P_M_MAIN_VIEW_RIGHT-1,P_M_MAIN_VIEW_BOTTOM-1);
	mainView				= new MainView(doc,containerRect, "tabContainer");
	AddChild(mainView);
}

void PWindow::CreatEditorList(void)
{
	PluginManager	*pluginManager	= (doc->BelongTo())->GetPluginManager();
	BList 			*editorPlugins	= pluginManager->GetPluginsByType(P_C_EDITOR_PLUGIN_TYPE);
	BasePlugin		*editorPlg			= NULL;
	if (editorPlugins != NULL)
	{
		for (int32 i = 0; i<editorPlugins->CountItems(); i++)
		{
			editorPlg	= (BasePlugin*)editorPlugins->ItemAt(i);
			AddEditor(editorPlg->GetName(),(PEditor *)editorPlg->GetNewObject(NULL));
//			subMenu->AddItem(new BMenuItem(B_TRANSLATE(editorPlg->GetName()),editorAdd));
		}
	}
}

BMenuBar *PWindow::MakeMenu(void)
{
	TRACE();
	BRect menuFrame		= Bounds();
	localizeMenuItems	= new BMessage();

	BMenuBar	*tmpBar = new BMenuBar(menuFrame,P_M_MENU_BAR);
	font_height height;
	tmpBar->GetFontHeight(&height);
	//**
//	float left, top,right,bottom;
//	tmpBar->GetItemMargins(&left,&top,&right,&bottom);
	float top		= 2;
	float bottom	= 2;
	menuFrame.bottom	= menuFrame.top+(height.ascent+height.descent+height.leading+top+bottom);
	tmpBar->ResizeTo(menuFrame.Width(),menuFrame.Height());
	BMenuItem	*item;
	BMenu		*subMenu;
	BMenu 		*menu;

	// build Filemenu
	menu		= new BMenu(B_TRANSLATE("File"));
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("New"),new BMessage(MENU_FILE_NEW),'N'));
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_FILE_NEW);
	item->SetTarget(be_app);
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Open"),new BMessage(MENU_FILE_OPEN),'O'));
	item->SetTarget(be_app);
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_FILE_OPEN);
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Close"),new BMessage(MENU_FILE_CLOSE),'W'));
	menu->AddSeparatorItem();
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_FILE_CLOSE);
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Save"), new BMessage(MENU_FILE_SAVE), 'S'));
	item->SetEnabled(false);
	item->SetTarget(doc);
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_FILE_SAVE);
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Save As"),new BMessage(MENU_FILE_SAVEAS)));
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_FILE_SAVE_AS);
	item->SetTarget(doc);
	menu->AddSeparatorItem();
	/*menu->AddItem(item = new BMenuItem(B_TRANSLATE(P_MENU_FILE_EXPORT),new BMessage(MENU_FILE_EXPORT)));
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_FILE_EXPORT);
	item->SetTarget(doc);*/
	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Page Setup"),new BMessage(MENU_FILE_PAGESETUP)));
	item->SetEnabled(false);
	item->SetTarget(doc);
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_FILE_PAGESETUP);
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Print"),new BMessage(MENU_FILE_PRINT), 'P'));
//	item->SetEnabled(false);
	item->SetTarget(doc);
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_FILE_PRINT);
	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Quit"), new BMessage(MENU_FILE_QUIT), 'Q'));
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_FILE_QUIT);
	tmpBar->AddItem(menu);
	localizeMenuItems->AddPointer("item",(void *) menu->Superitem());
	localizeMenuItems->AddPointer("itemstring",P_MENU_FILE);

	menu=new BMenu(B_TRANSLATE("Edit"));
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Undo"),new BMessage(B_UNDO),'Z'));
	item->SetTarget(doc);
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_EDIT_UNDO);
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Redo"),new BMessage(B_REDO),'Z',B_SHIFT_KEY));
	item->SetTarget(doc);
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_EDIT_REDO);
	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Cut"),new BMessage(B_CUT),'X'));
	item->SetTarget(doc);
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_EDIT_CUT);
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Copy"),new BMessage(B_COPY),'C'));
	item->SetTarget(doc);
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_EDIT_COPY);
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Paste"),new BMessage(B_PASTE),'V'));
	item->SetTarget(doc);
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_EDIT_PASTE);
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Clear"),new BMessage(B_CLEAR)));
	item->SetTarget(doc);
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_EDIT_CLEAR);
	menu->AddSeparatorItem();

	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Select All"),new BMessage(B_SELECT_ALL),'A'));
	item->SetTarget(doc);
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_EDIT_SELECT_ALL);
	menu->AddSeparatorItem();

	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Project Settings"),new BMessage(MENU_APP_SETTINGS)));
	item->SetTarget(doc);
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_EDIT_PROJECT_SETTINGS);
	tmpBar->AddItem(menu);
	localizeMenuItems->AddPointer("item",(void *) menu->Superitem());
	localizeMenuItems->AddPointer("itemstring",P_MENU_EDIT);


	menu=new BMenu(B_TRANSLATE("Search"));
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Find"),new BMessage(MENU_SEARCH_FIND),'F'));
	item->SetTarget(doc);
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_SEARCH_FIND);
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Find Next"),new BMessage(MENU_SEARCH_FIND_NEXT),'G'));
	item->SetTarget(doc);
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_SEARCH_FIND_NEXT);
	menu->AddSeparatorItem();

	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Replace"),new BMessage(MENU_SEARCH_REPLACE),'R'));
	item->SetTarget(doc);
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_SEARCH_REPLACE);
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Replace & Find"),new BMessage(MENU_SEARCH_REPLACE_AND_FIND)));
	item->SetTarget(doc);
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_SEARCH_REPLACE_AND_FIND);
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Replace All"),new BMessage(MENU_SEARCH_REPLACE_ALL)));
	item->SetTarget(doc);
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_SEARCH_REPLACE_ALL);
	tmpBar->AddItem(menu);
	localizeMenuItems->AddPointer("item",(void *) menu->Superitem());
	localizeMenuItems->AddPointer("itemstring",P_MENU_SEARCH);

	menu=new BMenu(B_TRANSLATE("Window"));
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Title"),new BMessage(MENU_WINDOW_TITLE)));
	item->SetTarget(doc);
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_WINDOW_TITLE);
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Title Vertical"),new BMessage(MENU_WINDOW_TITLE_VERTICAL)));
	item->SetTarget(doc);
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_WINDOW_TITLE_VERTICAL);
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Cascade"),new BMessage(MENU_WINDOW_CASCADE)));
	item->SetTarget(doc);
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_WINDOW_CASCADE);
	menu->AddSeparatorItem();

	tmpBar->AddItem(menu);
	localizeMenuItems->AddPointer("item",(void *) menu->Superitem());
	localizeMenuItems->AddPointer("itemstring",P_MENU_WINDOW);


	menu=new BMenu(B_TRANSLATE("Macro"));
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Start Recording"),new BMessage(MENU_MACRO_START_RECORDING)));
	item->SetTarget(doc);
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_MACRO_START_RECORDING);
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Stop Recording"),new BMessage(MENU_MACRO_STOP_RECORDING)));
	item->SetTarget(doc);
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_MACRO_STOP_RECORDING);

	subMenu	= new BMenu(B_TRANSLATE("Play"));
	menu->AddItem(subMenu);
	localizeMenuItems->AddPointer("item",(void *) subMenu->Superitem());
	localizeMenuItems->AddPointer("itemstring",P_MENU_MACRO_PLAY);

	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Open"),new BMessage(MENU_MACRO_OPEN)));
	item->SetTarget(doc);
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_MACRO_OPEN);
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Save"),new BMessage(MENU_MACRO_SAVE)));
	item->SetTarget(doc);
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_MACRO_SAVE);

	tmpBar->AddItem(menu);
	localizeMenuItems->AddPointer("item",(void *) menu->Superitem());
	localizeMenuItems->AddPointer("itemstring",P_MENU_MACRO);

	menu=new BMenu(B_TRANSLATE("Help"));
	//menu->AddItem(item = new BMenuItem(B_TRANSLATE(P_MENU_HELP_ABOUT),new BMessage(MENU_HELP_ABOUT)));
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("About"),new BMessage(B_ABOUT_REQUESTED)));
	item->SetTarget(be_app);
	localizeMenuItems->AddPointer("item",(void *) item);
	localizeMenuItems->AddPointer("itemstring",P_MENU_HELP_ABOUT);

	tmpBar->AddItem(menu);
	localizeMenuItems->AddPointer("item",(void *) menu->Superitem());
	localizeMenuItems->AddPointer("itemstring",P_MENU_HELP);
	tmpBar->SetBorder(B_BORDER_CONTENTS);
		savemessage = NULL;
	P_M_MAIN_VIEW_TOP=(tmpBar->Frame()).bottom+1.0;
	return tmpBar;
}

BMenuBar *PWindow::MakeStatusBar(void)
{
	TRACE();
	BRect statusFrame=Bounds();

	BMenuBar *tmpBar=new BMenuBar(statusFrame,P_M_STATUS_BAR, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM);
	//use FontHigh;
	BFont *font=new BFont();
	tmpBar->GetFont(font);
	font->SetSize(font->Size()-3);
	tmpBar->SetFont(font);
	font_height height;
	font->GetHeight(&height);
	float top		= 2;
	float bottom	= 2;
	statusFrame.top=statusFrame.bottom-(height.ascent+height.descent+height.leading+top+bottom);
	tmpBar->ResizeTo(statusFrame.Width(),statusFrame.Height());
	tmpBar->MoveTo(0,statusFrame.top);

	tmpBar->SetBorder(B_BORDER_CONTENTS);
	P_M_MAIN_VIEW_BOTTOM=statusFrame.top-1.0;
	return tmpBar;
}


void PWindow::MakeToolbars()
{
	TRACE();
	ToolItem	*toolItem	= NULL;
	ToolBar		*tmpBar		= NULL;
	BBitmap		*tmpBitmap	= NULL;
	BRect statusFrame=Bounds();
	statusFrame.right=20;


	tmpBar		=new ToolBar(statusFrame,P_M_STANDART_TOOL_BAR,B_ITEMS_IN_ROW);

	tmpBitmap	= BTranslationUtils::GetBitmap(B_PNG_FORMAT,"new");

	toolItem	= new ToolItem("new",tmpBitmap,new BMessage(MENU_FILE_NEW));
	toolItem->SetTarget(be_app);
	tmpBar->AddItem(toolItem);
	tmpBar->AddSeperator();

	tmpBitmap=BTranslationUtils::GetBitmap(B_PNG_FORMAT,"open");
	toolItem = new ToolItem("open",tmpBitmap,new BMessage(MENU_FILE_OPEN));
	toolItem->SetTarget(be_app);
	tmpBar->AddItem(toolItem);

	tmpBitmap=BTranslationUtils::GetBitmap(B_PNG_FORMAT,"save");
	toolItem = new ToolItem("save",tmpBitmap,new BMessage(MENU_FILE_SAVE));
	tmpBar->AddItem(toolItem);
	toolItem->SetTarget(doc);
	tmpBitmap=BTranslationUtils::GetBitmap(B_PNG_FORMAT,"save as");
	toolItem = new ToolItem("save as",tmpBitmap,new BMessage(MENU_FILE_SAVE));
	tmpBar->AddItem(toolItem);
	toolItem->SetTarget(doc);

	tmpBitmap=BTranslationUtils::GetBitmap(B_PNG_FORMAT,"print");
	toolItem = new ToolItem("print",tmpBitmap,new BMessage(MENU_FILE_PRINT));
	toolItem->SetTarget(doc);
	tmpBar->AddItem(toolItem);


	tmpBar->AddSeperator();
	tmpBar->AddSeperator();
	tmpBitmap=BTranslationUtils::GetBitmap(B_PNG_FORMAT,"find");
	toolItem = new ToolItem("find",tmpBitmap,NULL);
	tmpBar->AddItem(toolItem);
	tmpBitmap=BTranslationUtils::GetBitmap(B_PNG_FORMAT,"trash");
	toolItem = new ToolItem("find",tmpBitmap,NULL);
	tmpBar->AddItem(toolItem);
	tmpBar->AddSeperator();
	tmpBitmap=BTranslationUtils::GetBitmap(B_PNG_FORMAT,"font");
	toolItem = new ToolItem("font",tmpBitmap,new BMessage('font'));
	tmpBar->AddItem(toolItem);

	AddToolBar(tmpBar);
}


bool PWindow::QuitRequested()
{
	TRACE();
	(new BMessenger(NULL,doc))->SendMessage(B_QUIT_REQUESTED);
	return false;
}
/**
 *@todo send SAVE and SAVE_AS MEssages to the PDocument
 */
void PWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
		case WINDOW_REGISTRY_ADDED:
			{
				char s[22];
				BRect rect;
				if (message->FindInt32("new_window_number", &window_id) == B_OK)
				{
					if (!savemessage)
					{		// if it's untitled
						sprintf(s, "Untitled %ld", window_id);
						SetTitle(s);
					}
				}
				if (message->FindRect("rect", &rect) == B_OK) {
					MoveTo(rect.LeftTop());
					ResizeTo(rect.Width(), rect.Height());
				}
				Minimize(false);
			}
			break;

		case MENU_FILE_NEW:
			{

			}
			break;
		case MENU_FILE_NEW_TAB:
			{
			}
			break;
		case MENU_FILE_CLOSE:
				(new BMessenger(NULL,doc))->SendMessage(B_QUIT_REQUESTED);
			break;
		case MENU_FILE_QUIT:
			// this is the old way, please use the new one
			// be_app->PostMessage(B_QUIT_REQUESTED);
			//be_app_messenger.SendMessage(B_QUIT_REQUESTED);
			(new BMessenger(NULL,doc))->SendMessage(B_QUIT_REQUESTED);
//				BWindow::MessageReceived(message);
			break;
		case MENU_FILE_SAVEAS:
			break;
		case MENU_FILE_SAVE:
			break;
		case B_SAVE_REQUESTED:
		//	Save(message);
			break;
		case P_C_INSERT_EDITOR:
		{
			BasePlugin	*plugin	= NULL;
			message->FindPointer("plugin",(void **)&plugin);
			AddEditor(plugin->GetName(),(PEditor *)plugin->GetNewObject((void *)message));
			break;
		}
		case P_C_REMOVE_EDITOR:
		{
			RemoveEditor();
		}
		default:
			BWindow::MessageReceived(message);
			break;
	}
}

void PWindow::AddEditor(const char *name,PEditor *editor)
{
	bool locked = LockLooper();
	BTab	*tab = new BTab();
	BRect	rect = mainView->Bounds();
	rect.InsetBy(5,5);
	rect.bottom -= mainView->TabHeight();
	mainView->AddTab(editor->GetView(), tab);
	tab->SetLabel(name);
	mainView->Select(tab);
	BMessage *configMessage	= editor->GetConfiguration();
	if (configMessage)
	{
		BMessage *docSettings	= doc->DocumentSettings();
		docSettings->AddMessage(name,configMessage);
	}
	editor->GetView()->MakeFocus(true);
	(editor->GetView())->ResizeTo(rect.Width()-B_V_SCROLL_BAR_WIDTH -2,rect.Height()-B_H_SCROLL_BAR_HEIGHT-2);
	(editor->GetView())->MoveTo(2,2);
	(doc->GetEditorManager())->RegisterPEditor(editor);
	if (locked)
		UnlockLooper();
}

void PWindow::RemoveEditor(void)
{
	BTab		*removed	= mainView->RemoveTab(mainView->Selection());
	BScrollView	*scroller	= NULL;
	if (removed != NULL)
	{
		scroller = (BScrollView*)removed->View();
		(doc->GetEditorManager())->UnregisterPEditor((PEditor *)scroller->Target());
	}
}

BMenu*	PWindow::GetMenu(const char *signature)
{
	TRACE();
	BMenuItem	*item		= NULL;
	const char	*string		= NULL;
	int32 		i			= 0;
	status_t err			= B_OK;
	bool	found			= false;
	while ((err == B_OK) && (!found))
	{
		err	= localizeMenuItems->FindPointer("itemstring",i,(void **)&string);
		err = err | localizeMenuItems->FindPointer("item",i,(void **)&item);
//		if ((err == B_OK)&&(string == signature))
		if (strcmp(signature,string) == B_OK)
			found=true;
		i++;
	}
	if (found)
		if (item->Submenu()!=NULL)
			return item->Submenu();
/*		if(item->Menu() != NULL)*/
		else
			return item->Menu();
	else
		return NULL;
}

BMenuItem*	PWindow::GetMenuItem(const char *menuitemSignature)
{
	TRACE();
	BMenuItem	*item		= NULL;
	const char	*string		= NULL;
	int32 		i			= 0;
	status_t err			= B_OK;
	bool	found			= false;
	while ((err==B_OK) && (!found))
	{
		err	= localizeMenuItems->FindPointer("itemstring",i,(void **)&string);
		err = err | localizeMenuItems->FindPointer("item",i,(void **)&item);
		if ( (err == B_OK) && (strcmp(string,menuitemSignature)==B_OK) )
			found=true;
		i++;
	}
	if (found)
		return item;
	else
		return NULL;
}

ToolBar* PWindow::GetToolBar(const char *signature)
{
	bool 		found		= false;
	int32		i			= 0;
	ToolBar		*tmpBar		= NULL;
	char		*tmpBarName	= NULL;
	while ( (!found) && (i<horizontalToolbars->CountItems()) )
	{
		tmpBar		= (ToolBar*) horizontalToolbars->ItemAt(i);
		tmpBarName	= (char *)tmpBar->Name();
		if ((tmpBar) && (tmpBarName) &&(strcmp(signature,tmpBarName) == B_OK))
			found=true;
		else
			i++;
	}
	if (!found)
	{
		i=0;
		while ( (!found) && (i<verticalToolbars->CountItems()) )
		{
			tmpBar =(ToolBar*) verticalToolbars->ItemAt(i);
			if (strcmp(signature,tmpBar->Name()) == B_OK)
				found=true;
			else
				i++;
		}
	}
	if (found)
		return tmpBar;
	else
		return NULL;
}

ToolMenu* PWindow::GetToolMenu(const char* toolbarSignature,const char *signature)
{
	ToolBar *toolbar	= GetToolBar(toolbarSignature);
	if ( toolbar != NULL 
		&& (verticalToolbars->HasItem(toolbar) == true || horizontalToolbars->HasItem(toolbar)== true) 
		&& signature!=NULL) {
			return toolbar->GetToolMenu(signature);
	}
	else
		return NULL;
}

ToolItem* PWindow::GetToolItem(const char* toolbarSignature,const char *signature)
{
	ToolBar *toolbar	= GetToolBar(toolbarSignature);
	if ( toolbar != NULL 
		&& (verticalToolbars->HasItem(toolbar)== true || horizontalToolbars->HasItem(toolbar)== true)
		&& signature!=NULL)
	{
		return toolbar->GetToolItem(signature);
	}
	else
		return NULL;
}


/**
 *Adds a BMenu to the ToolBar of the PWindow, if inxed<0 its insert "at the end"!
 *It shoud always be an translated BMenu so please use the LocaleKit to create
 *the BMenu
 *
 *@todo
 */
status_t PWindow::AddMenu(BMenu *menu,int32 index)
{
	//** need to check if menu is valid
	if (index <0)
		menuBar->AddItem(menu);
	else
		menuBar->AddItem(menu,index);
	BMenuItem *menuItem = menu->Superitem();
	localizeMenuItems->AddPointer("item",(void *) menuItem );
	char	*itemString= new char[strlen(menuItem->Label())];
	strcpy(itemString, menuItem->Label());
	localizeMenuItems->AddPointer("itemstring",itemString);
	menuItem->SetLabel(B_TRANSLATE(itemString));
	return B_OK;
}

status_t PWindow::AddMenuItem(const char *menuSignatur, BMenuItem *menuItem,int32 index)
{
	BMenu	*menu =	GetMenu(menuSignatur);
	if (menu != NULL)
	{
		if (index <0)
			menu->AddItem(menuItem);
		else
			menu->AddItem(menuItem,index);
		localizeMenuItems->AddPointer("item",(void *) menuItem );
		char	*itemString= new char[strlen(menuItem->Label())];
		strcpy(itemString, menuItem->Label());
		localizeMenuItems->AddPointer("itemstring",itemString);
		return B_OK;
	}
	else
		return B_BAD_VALUE;
}


status_t PWindow::AddToolBar(ToolBar *toolBar)
{
	TRACE();
//	ToolBar *tmpToolBar;
	if (toolBar->GetLayout()==B_ITEMS_IN_ROW)
	{

		//adjust the Toolbar
		toolBar->MoveTo(1,P_M_MAIN_VIEW_TOP+1);
		toolBar->ResizeTo(P_M_MAIN_VIEW_RIGHT-2,(toolBar->Bounds()).Height());
		AddChild(toolBar);
		P_M_MAIN_VIEW_TOP=toolBar->Frame().bottom;
		horizontalToolbars->AddItem(toolBar);
		ReCalcToolBars(B_ITEMS_IN_COLUMN);
	}
	else
	{
		toolBar->MoveTo(P_M_MAIN_VIEW_LEFT,P_M_MAIN_VIEW_TOP+1.0);
		AddChild(toolBar);
		toolBar->ResizeTo((toolBar->Bounds()).Width(),P_M_MAIN_VIEW_BOTTOM-P_M_MAIN_VIEW_TOP-1);
		P_M_MAIN_VIEW_LEFT=toolBar->Frame().right+1;
		verticalToolbars->AddItem(toolBar);
	//	ReCalcToolBars(B_ITEMS_IN_ROW);
		//recalc the istems in Row
	}
	if (mainView)
	{
		mainView->ResizeTo(P_M_MAIN_VIEW_RIGHT-P_M_MAIN_VIEW_LEFT-2,P_M_MAIN_VIEW_BOTTOM-P_M_MAIN_VIEW_TOP-2);
		mainView->MoveTo(P_M_MAIN_VIEW_LEFT+1,P_M_MAIN_VIEW_TOP+1);
	}

	toolBar->SetButtonBorder(false);
	return B_OK;
}

status_t PWindow::AddToolMenu(const char *toolbarSignatur,ToolMenu *toolMenu)
{
	ToolBar *toolbar= GetToolBar(toolbarSignatur);
	status_t err=B_OK;
	if (toolbar!=NULL)
	{
		if ((verticalToolbars->HasItem(toolbar) == true || horizontalToolbars->HasItem(toolbar) == true ) && (toolMenu!=NULL))
		{
				toolbar->AddItem(toolMenu);
				//** later implement Tooltip
		}
		else
			err = B_BAD_VALUE;
	}
	else
		err= B_BAD_VALUE;
	return err;

}

status_t PWindow::AddToolItem(const char *toolbarSignatur,const char *toolmenuSignatur,ToolItem *toolItem,int32 index)
{
	ToolMenu *toolMenu=GetToolMenu(toolbarSignatur,toolmenuSignatur);
	status_t err = B_OK;
	if (toolMenu!= NULL)
	{
		if (index<0)
		{
			toolMenu->AddItem(toolItem);
			err = B_OK;
		}
		else
			//at the Moment there is no inserting at a specific Position implemented
			err = B_BAD_INDEX;
	}
	else
	{
		ToolBar *toolBar = GetToolBar(toolbarSignatur);
		toolBar->AddItem(toolItem);
//		err = B_BAD_VALUE;
	}
	return err;
}


status_t	PWindow::RemoveMenu(const char* menuSignature)
{
	BMenu*	tmpItem		= GetMenu(menuSignature);
	BMenu*	super		= NULL;
	if (tmpItem)
		super			= tmpItem->Supermenu();
	if (super)
	{
		super->RemoveItem(tmpItem);
		return B_OK;
	}
	else
		return B_BAD_VALUE;

}

status_t	PWindow::RemoveMenuItem(const char* signature)
{
	BMenuItem*	tmpItem		= GetMenuItem(signature);
	BMenu* 		super		= NULL;
	if (tmpItem)
			super			= tmpItem->Menu();
	if (super)
	{
		super->RemoveItem(tmpItem);
		return B_OK;
	}
	else
		return B_BAD_VALUE;
}

status_t	PWindow::RemoveToolBar(const char* signature)
{
	TRACE();
	status_t	err		= B_OK;
	bool		removed	= true;
	ToolBar *tmpToolBar		= GetToolBar(signature);
	if (tmpToolBar!=NULL){
		removed = RemoveChild(tmpToolBar);
		if (removed != true)
			err = B_ERROR;
	}
	if (verticalToolbars->HasItem(tmpToolBar)) {
		if (!verticalToolbars->RemoveItem(tmpToolBar))
			err = B_ERROR;
		else
			P_M_MAIN_VIEW_LEFT	-= (tmpToolBar->Frame().Width()+1);
	}
	else if (horizontalToolbars->HasItem(tmpToolBar)) {
		if (!horizontalToolbars->RemoveItem(tmpToolBar))
			err=B_ERROR;
		else {
			P_M_MAIN_VIEW_TOP	-= tmpToolBar->Frame().Height();
			ReCalcToolBars(B_ITEMS_IN_COLUMN);
		}
	}
	mainView->ResizeTo(P_M_MAIN_VIEW_RIGHT-P_M_MAIN_VIEW_LEFT-2,P_M_MAIN_VIEW_BOTTOM-P_M_MAIN_VIEW_TOP-2);
	mainView->MoveTo(P_M_MAIN_VIEW_LEFT+1,P_M_MAIN_VIEW_TOP+1);
	return err;
}

status_t	PWindow::RemoveToolMenu(const char* toolbarSignature,const char* signature)
{

	status_t err	= B_OK;
	ToolBar		*toolBar		= GetToolBar(toolbarSignature);
	ToolMenu	*tmpToolMenu	= GetToolMenu(toolbarSignature,signature);
	if ((tmpToolMenu)&& (toolBar))
		toolBar->RemoveItem(tmpToolMenu);
	else
		err		= B_BAD_VALUE;
	return err;
}

status_t	PWindow::RemoveToolItem(const char* toolbarSignature,const char* signature)
{
	ToolBar		*toolBar		= GetToolBar(toolbarSignature);
	ToolItem	*tmpToolItem	= GetToolItem(toolbarSignature,signature);
	status_t err	= B_OK;
	if ((tmpToolItem) && (toolBar))
		toolBar->RemoveItem(tmpToolItem);
	else
		err		= B_BAD_VALUE;
	return err;
}


void PWindow::ReCalcToolBars(menu_layout layout)
{
	TRACE();
	int i;
	ToolBar *tmpToolBar=NULL;
	if (layout==B_ITEMS_IN_ROW)
	{
		for (i=0;i<horizontalToolbars->CountItems();i++)
		{
			tmpToolBar=(ToolBar *)(horizontalToolbars->ItemAt(i));
			tmpToolBar->MoveTo(P_M_MAIN_VIEW_LEFT,(tmpToolBar->Frame()).top);
			tmpToolBar->ResizeTo(P_M_MAIN_VIEW_RIGHT-P_M_MAIN_VIEW_LEFT,(tmpToolBar->Bounds()).Height());
		}
		/*if (tmpToolBar != NULL)
			P_M_MAIN_VIEW_TOP=tmpToolBar->Bounds().bottom+1.0;*/
	}
	else
	{
		for (i=0;i<verticalToolbars->CountItems();i++)
		{
			tmpToolBar=(ToolBar *)(verticalToolbars->ItemAt(i));
			tmpToolBar->MoveTo(tmpToolBar->Frame().left,P_M_MAIN_VIEW_TOP+1.0);
			tmpToolBar->ResizeTo((tmpToolBar->Bounds()).Width(),P_M_MAIN_VIEW_BOTTOM-P_M_MAIN_VIEW_TOP-1);
		}
		/*if (tmpToolBar != NULL)
			P_M_MAIN_VIEW_LEFT=tmpToolBar->Frame().right+1.0;*/
	}
}



void PWindow::FrameResized(float width, float height)
{
		//just to remove the warnings .. from the compiler
///		width	= width;
	//	height	= height;
		BWindow::FrameResized(width, height);
		P_M_MAIN_VIEW_LEFT		= mainView->Frame().left;
		P_M_MAIN_VIEW_TOP 		= mainView->Frame().top;
		P_M_MAIN_VIEW_BOTTOM	= mainView->Frame().bottom;
		P_M_MAIN_VIEW_RIGHT		= mainView->Frame().right;
}

void PWindow::Show(void)
{
	BWindow::Show(),
	CreatEditorList();
}
