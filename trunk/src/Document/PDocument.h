#ifndef P_DOCUMENT_H
#define P_DOCUMENT_H

#include <app/Message.h>
#include <app/Handler.h>
#include <app/Looper.h>
#include <interface/Region.h>
#include <interface/View.h>
#include <interface/Menu.h>

#include <storage/File.h>
#include <storage/FilePanel.h>
#include <support/Archivable.h>
#include <support/List.h>
#ifdef B_ZETA_VERSION_BETA
	#include <locale/Locale.h>
	#include <locale/LanguageNotifier.h>
#else
	#define _T(a) 
#endif 

#include <support/ReadWriteLocker.h>


#include "WindowManager.h"
#include "PEditorManager.h"
#include "PCommandManager.h"
#include "HelpManager.h"



class PDocumentManager;

/**
 * @class PDocument
 *
 * @brief  PDocument is the heart of the Programm it contains all Nodes, Connections and necessary obtions
 * to manage these stuff
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2005/10/04
 *
 *
 *@todo Helpmanager complete impelementation and integration in PDocument, ToolBar, ToolItem, ToolMenu PWindow and PEditor
 *
 * @see BMessage
 */
class PDocument :	public BLooper, public BReadWriteLocker,public PMenuAccess
{

friend class PCommand;

public:

								PDocument(PDocumentManager *initManager);
								PDocument(PDocumentManager *initManager,entry_ref *openEntry);

								PDocument(BMessage *archive);
							
								~PDocument();

	//+++BHandler+++
	virtual	status_t			Archive(BMessage* archive, bool deep = true) const;
	static	BArchivable			*Instantiate(BMessage *from);
	virtual	void				MessageReceived(BMessage* message);
	
	/**
	 * returns the title of the Document.. this is generated from the FileName
	 */
	const	char*				Title(void);

	/**
	 * Bounds returns the Bounds of the Document wicht enclose everey Node in the Document
	 */	
	virtual	BRect				Bounds(void){return bounds;};
	/**
	 * Resizes the Document (mostly called from Commands)
	 */	
	virtual void				Resize(float toX,float toY);

	/**
	 * Returns if the Document is dirty .. not implemented at the moment
	 */	
	virtual	bool				IsDirty(void){return dirty;};
	/**
	 * IsModified returns if there was any change in the document
	 */	
	virtual	bool				IsModified(void){return modified;}
	/**
	 * SetModified set the Document to a modified state.. 
	 * if the user now try to quit he is asked
	 */	
	virtual void				SetModified(void){modified=true;};
	/**
	 * ResetModified set the Document to a unmodified state.. 
	 */	
	virtual	void				ResetModified(void){modified=false;};

	virtual BList				*GetChangedNodes(void){return valueChanged;};

	virtual BList				*GetSelected(void){return selected;};
	virtual	BList				*GetAllNodes(void){return allNodes;};
	virtual BList				*GetAllConnections(void){return allConnections;};
	virtual BList				*GetTrash(void){return trashed;};

	//* Returns the DocumentSettings in a BMessage
	virtual	BMessage*			DocumentSettings(void){return documentSetting;};
	virtual	void				SetDocumentSettings(BMessage *message);


	//* Returns the PrintSettings as a BMessage if there is no PrintSettings a dialog shows up where the printsettings shoud be configured :-)
	virtual	BMessage*			PrintSettings(void);
	/**Set the PrintSettings 
	 * @warning There is no check if the printersettings are valid!!!!!
	 */
	virtual	void				SetPrintSettings(BMessage *settings);
	/**Print´s the lastly activated Editor
	 *@todo some error handling and printerSettings.. autocall
	 */
	virtual void				Print(void);

	/**Set the save and load entry
	 */
	virtual	void				SetEntry(entry_ref *saveEntry,const char *name);
	/**Set the save and load entry*/
	virtual	void				SetEntry(entry_ref *newEntry){entryRef = new entry_ref(*newEntry);};
	/**return the save and load entry*/
	virtual entry_ref*			Entry(void){return entryRef;};

	/**save the file to the entry
	 *@see SetEntry
	 */
	virtual	void				Save(void);
	/**save the file from the entry
	 *
	 *@todo mime check
	 *
	 *@see SetEntry
	 */
	virtual	void				Load(void);

	/**shows a savePanel.. wich pass back an entry and autmatically call SetEntry
	 *@see SetEntry
	 */
	virtual	void				SavePanel();

	virtual	bool				QuitRequested(void);

	virtual	WindowManager*		GetWindowManager(void){return windowManager;};
	virtual	PCommandManager*	GetCommandManager(void){return commandManager;};
	virtual	PEditorManager*		GetEditorManager(void){return editorManager;};

	
	virtual	PDocumentManager	*BelongTo(void){return documentManager;};


	//++++++++++++++++++++++PMenuAccess
	
	virtual	BMenu*				GetMenu(const char* signatur);
	virtual	BMenuItem*			GetMenuItem(const char* signatur);

	virtual	ToolBar*			GetToolBar(const char *signatur);
	virtual	ToolMenu*			GetToolMenu(const char* toolbarSignature,const char *signature);
	virtual	ToolItem*			GetToolItem(const char* toolbarSignature,const char *signature);

	virtual	status_t			AddMenu(BMenu *menu,int32 index = -1);
	virtual	status_t			AddMenuItem(const char *menuSignatur, BMenuItem *menuItem,int32 index = -1);
	virtual	status_t			AddToolBar(ToolBar *toolbar);
	virtual	status_t			AddToolMenu(const char *toolbarSignatur,ToolMenu *toolMenu);
	virtual	status_t			AddToolItem(const char *toolbarSignatur,const char *toolmenuSignatur,ToolItem *toolItem,int32 index = -1);


	virtual status_t			RemoveMenu(const char* menuSignature) ;
	virtual status_t			RemoveMenuItem(const char* menuItemSignature);

	virtual status_t			RemoveToolBar(const char* signature);
	virtual status_t			RemoveToolMenu(const char* toolbarSignature,const char* toolmenuSignature) ;
	virtual status_t			RemoveToolItem(const char* toolbarSignature,const char* toolitemSignature);
	
	virtual BRect*				GetPrintRect(void){return printableRect;};

	virtual	BMessage*			FindObject(BPoint *where);

protected:
	virtual	void				Init(void);
	virtual	void				Init(BMessage *archive);
	virtual	BMessage			*ReIndex(BList *nodeList);

			
			
			PDocumentManager	*documentManager;

			BList				*allNodes;
			BList				*allConnections;
			BList				*selected;
			BList				*trashed;
			
			BList				*valueChanged;
	
			BRect				bounds;
			BRect				*paperRect;
			BRect				*printableRect;
			BMessage			*printerSetting;
			BMessage	 	 	*documentSetting;
			BFilePanel			*savePanel;
		
		 	float				width,height;
			bool				dirty;
			entry_ref			*entryRef;
			bool				modified;
			BMenu				*fileMenu;
			BMenu				*editMenu;
			BMenu				*searchMenu;
			
			
			PEditorManager		*editorManager;
			PCommandManager		*commandManager;
			WindowManager		*windowManager;
			
			BMessage			*testMacro;
			
			HelpManager			*helpManager;
			
			
private:

};
#endif
