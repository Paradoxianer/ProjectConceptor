#ifndef PROJEKT_CONCEPTOR_WINDOW_H
#define PROJEKT_CONCEPTOR_WINDOW_H
/*
 * @author Paradoxon
 */

#include "ToolBar.h"
#include "PluginManager.h"
#include "PMenuAccess.h"
#include "ShortCutFilter.h"


#include <app/AppDefs.h>
#include <interface/MenuBar.h>
#include <interface/Window.h>
#include <interface/TabView.h>

class PDocument;
class PEditor;

class MainView;

/**
 * @class PDocument
 *
 * @brief  PDocument is the heart of the Programm it contains all Nodes, Connections and necessary obtions
 * to manage these stuff
 *
 * @author Matthias Lindner
 * @version 0.01
 * @date 2005/10/04
 * @Contact: two4god@web.de
 *
 */
class PWindow: public BWindow, public PMenuAccess
{

public:
							PWindow(BRect rect,PDocument *doc);
							PWindow(BMessage *archive);
							~PWindow(void);
	virtual void			MessageReceived(BMessage *message);

	/**Set a new Shortcutlist. But Remember if you set a new shortcutlist you also "remove" all default shortcuts from the document.
	 *please use instead AddShortCutList and Removeshortcutlist
	 */
	virtual void			SetShortCutList(BMessage *shortcutList){shortcutFilter->SetShortCutList(shortcutList);};
	/**Add a new Shortcutlist. This means the old Shortcuts are retained. if you set an shortcut wich is already in use it´s been replaced
	 */
	virtual void			AddShortCutList(BMessage *shortcutList){shortcutFilter->AddShortCutList(shortcutList);};
	/**Remove all Shortcuts in the Shortcutlist.
	 * @warning Previous overitten Shortcust are NOT restored!
	 */
	virtual void			RemoveShortCutList(BMessage *shortcutList){shortcutFilter->RemoveShortCutList(shortcutList);};
	/**Calle To add a Editor to the MainView (with a new Tab)
	 */
	virtual	void			AddEditor(const char *name,PEditor *newEditor);
	/**Removes the current selected Edtior
	 */
	virtual	void			RemoveEditor(void);


	//++++++++++++++++++++++PMenuAcces
	virtual	BMenu*			GetMenu(const char* signatur);
	virtual	BMenuItem*		GetMenuItem(const char* signatur);

	virtual	ToolBar*		GetToolBar(const char *signatur);
	virtual	ToolMenu*		GetToolMenu(const char* toolbarSignature,const char *signature);
	virtual	ToolItem*		GetToolItem(const char* toolbarSignature,const char *signature);

	virtual	status_t		AddMenu(BMenu *menu,int32 index = -1);
	virtual	status_t		AddMenuItem(const char *menuSignatur, BMenuItem *menuItem,int32 index = -1);

	virtual	status_t		AddToolBar(ToolBar *toolbar);
	virtual	status_t		AddToolMenu(const char *toolbarSignatur,ToolMenu *toolMenu);
	virtual	status_t		AddToolItem(const char *toolbarSignatur,const char *toolmenuSignatur,ToolItem *toolItem,int32 index = -1);


	virtual status_t		RemoveMenu(const char* menuSignature) ;
	virtual status_t		RemoveMenuItem(const char* signature);

	virtual status_t		RemoveToolBar(const char* signature);
	virtual status_t		RemoveToolMenu(const char* toolbarSignature,const char* toolmenuSignature) ;
	virtual status_t		RemoveToolItem(const char* toolbarSignature,const char* toolitemSignature);
	//++++++++++++++++++++++PMenuAcces

	virtual	bool			QuitRequested();


	virtual void			FrameResized(float width, float height);
	virtual void			Show(void);
			void			Init(void);
			float			P_M_MAIN_VIEW_LEFT;
			float			P_M_MAIN_VIEW_TOP;
			float			P_M_MAIN_VIEW_BOTTOM;
			float			P_M_MAIN_VIEW_RIGHT;


protected:
			void			CreatEditorList(void);

			BMenuBar *		MakeMenu(void);
			BMenuBar *		MakeStatusBar(void);
			void			ChangeLanguage(void);
			void			MakeToolbars(void);
			void			ReCalcToolBars(menu_layout layout);


			ShortCutFilter	*shortcutFilter;

			BMessage		*savemessage;
			int32			window_id;

			BMenuBar		*menuBar;
//			BMenu			*menu;
			BMenu			*windows;
			BMessage		*localizeMenuItems;
			BMessage		*localizeToolTips;
			BMessage		*oldShortcutMessage;

	/*		ToolbarView		*verticalToolbars;
			ToolbarView		*horizontalToolbars;*/

			BMenuBar		*statusBar;
	//		ProjektTabView	*tabView;
			BList			*verticalToolbars;
			BList			*horizontalToolbars;


			MainView		*mainView;
			PDocument		*doc;
private:
	#ifdef B_ZETA_VERSION_1_0_0
    	    BLanguageNotifier languageChanger;
    #endif

};
#endif
