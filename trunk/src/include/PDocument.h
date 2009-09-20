#ifndef P_DOCUMENT_H
#define P_DOCUMENT_H

#include "PMenuAccess.h"

#include <app/Message.h>
#include <MessageRunner.h>
#include <app/Handler.h>
#include <app/Looper.h>
#include <interface/Region.h>
#include <interface/View.h>
#include <interface/Menu.h>
#include <storage/File.h>
#include <storage/FilePanel.h>
#include <support/Archivable.h>
#include <support/List.h>
#ifdef B_ZETA_VERSION_1_0_0
	#include <locale/Locale.h>
	#include <locale/LanguageNotifier.h>
#else
	#define _T(a) a
#endif


class HelpManager;
class PCommandManager;
class PCSavePanel;
class PDocumentManager;
class PEditorManager;
class PWindow;


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
 *@todo Implement Init(BMessage *archive)
 *
 * @see BMessage
 */
class PDocument :	public BLooper ,public PMenuAccess
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
			BRect				Bounds(void){return bounds;};
	/**
	 * Resizes the Document (mostly called from Commands)
	 */
			void				Resize(float toX,float toY);

	/**
	 * Returns if the Document is dirty .. not implemented at the moment
	 */
			bool				IsDirty(void){return dirty;};
	/**
	 * IsModified returns if there was any change in the document
	 */
			bool				IsModified(void){return modified;}
	/**
	 * SetModified set the Document to a modified state..
	 * if the user now try to quit he is asked
	 */
			void				SetModified(void);
	/**
	 * ResetModified set the Document to a unmodified state..
	 */
			void				ResetModified(void);

			BList				*GetChangedNodes(void){return valueChanged;};

			BList				*GetSelected(void){return selected;};
			BList				*GetAllNodes(void){return allNodes;};
			BList				*GetAllConnections(void){return allConnections;};
//			BList				*GetTrash(void){return trashed;};

	//* Returns the DocumentSettings in a BMessage
			BMessage*			DocumentSettings(void){return documentSetting;};
			void				SetDocumentSettings(BMessage *message);
			void				ShowSettings(void);

	//* Returns the PrintSettings as a BMessage if there is no PrintSettings a dialog shows up where the printsettings shoud be configured :-)
			BMessage*			PrintSettings(void);
	/**Set the PrintSettings
	 * @warning There is no check if the printersettings are valid!!!!!
	 */
			void				SetPrintSettings(BMessage *settings);
	/**Print´s the lastly activated Editor
	 *@todo some error handling and printerSettings.. autocall
	 */
			void				Print(void);

	/**Set the save and load entry
	 */
			void				SetEntry(entry_ref *saveEntry,const char *name);
	/**Set the save and load entry*/
			void				SetEntry(entry_ref *newEntry){entryRef = new entry_ref(*newEntry);};
	/**return the save and load entry*/
			entry_ref*			Entry(void){return entryRef;};

	/**save the file to the entry
	 *@see SetEntry
	 */
			void				Save(void);
	/**save the file from the entry
	 *
	 *@todo mime check
	 *
	 *@see SetEntry
	 */
			void				Load(void);

	/**shows a savePanel.. wich pass back an entry and autmatically call SetEntry
	 *@see SetEntry
	 */
			void				SavePanel();

			bool				QuitRequested(void);

			PWindow*			GetWindow(void){return window;};
			PCommandManager*	GetCommandManager(void){return commandManager;};
			PEditorManager*		GetEditorManager(void){return editorManager;};


			PDocumentManager	*BelongTo(void){return documentManager;};


	//++++++++++++++++++++++PMenuAccess

			BMenu*				GetMenu(const char* signatur);
			BMenuItem*			GetMenuItem(const char* signatur);

			ToolBar*			GetToolBar(const char *signatur);
			ToolMenu*			GetToolMenu(const char* toolbarSignature,const char *signature);
			ToolItem*			GetToolItem(const char* toolbarSignature,const char *signature);

			status_t			AddMenu(BMenu *menu,int32 index = -1);
			status_t			AddMenuItem(const char *menuSignatur, BMenuItem *menuItem,int32 index = -1);
			status_t			AddToolBar(ToolBar *toolbar);
			status_t			AddToolMenu(const char *toolbarSignatur,ToolMenu *toolMenu);
			status_t			AddToolItem(const char *toolbarSignatur,const char *toolmenuSignatur,ToolItem *toolItem,int32 index = -1);


			status_t			RemoveMenu(const char* menuSignature) ;
			status_t			RemoveMenuItem(const char* menuItemSignature);

			status_t			RemoveToolBar(const char* signature);
			status_t			RemoveToolMenu(const char* toolbarSignature,const char* toolmenuSignature) ;
			status_t			RemoveToolItem(const char* toolbarSignature,const char* toolitemSignature);

			BRect*				GetPrintRect(void){return printableRect;};

			BMessage*			FindObject(BPoint *where);

protected:
			void				Init(void);
			void				Init(BMessage *archive);
			/**save the file temporary for restoring
			 */
	 		void				AutoSave(void);
			/* puts all PDocument data to the given Stream;
			 */
			void				PushToStream(BPositionIO *pushTo);

			PDocumentManager	*documentManager;
			
			BList				*allNodes;
			BList				*allConnections;
			BList				*selected;
			//** change this to a set<> makes it more perfomrant and dont use duplicated entrys
			BList				*valueChanged;

			BRect				bounds;
			BRect				*paperRect;
			BRect				*printableRect;
			BMessage			*printerSetting;
			BMessage	 	 	*documentSetting;
			PCSavePanel			*savePanel;

		 	float				width,height;
			bool				dirty;
			entry_ref			*entryRef;
			BEntry				*autoSaveRef;
			bool				modified;

			PEditorManager		*editorManager;
			PCommandManager		*commandManager;
			PWindow				*window;
			BMessage			*testMacro;

			HelpManager			*helpManager;
			BMessageRunner		*autoSaver;
			bigtime_t			autoSaveIntervall;


private:

};
#endif
