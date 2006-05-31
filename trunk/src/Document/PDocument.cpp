#include <interface/Alert.h>
#include <interface/PrintJob.h>

#include "PDocument.h"
#include "PDocLoader.h"
#include "PDocumentManager.h"
#include "PluginManager.h"
#include "BasePlugin.h"
#include "Tools/Indexer.h"

PDocument::PDocument(PDocumentManager *initManager):BLooper(),BReadWriteLocker() 
{
	TRACE();
	documentManager=initManager;
	Init();
	Run();
	windowManager->AddPWindow(new PWindow(BRect(100,100,500,400),this));
}

PDocument::PDocument(PDocumentManager *initManager,entry_ref *openEntry):BLooper(),BReadWriteLocker() 
{
	TRACE();
	documentManager=initManager;
	Init();
	Run();
	windowManager->AddPWindow(new PWindow(BRect(100,100,500,400),this));
	
}

PDocument::PDocument(BMessage *archive):BLooper(archive),BReadWriteLocker()
{
	TRACE();
	Init(archive);
	Run();
}

PDocument::~PDocument()
{
	TRACE();
	documentManager->RemoveDocument(this);
}

status_t PDocument::Archive(BMessage* archive, bool deep = true) const
{
	TRACE();
	return B_ERROR;
}

BArchivable* PDocument::Instantiate(BMessage* message)
{
	TRACE();
	return NULL;
}

void PDocument::MessageReceived(BMessage* message)
{
	TRACE();
	switch(message->what) 
	{
		case MENU_FILE_SAVE:
		{
			if (entryRef)
				SavePanel();
			else
				Save();
			break;
		}
		case MENU_FILE_SAVEAS:
		{
			SavePanel();
			break;
		}
		case MENU_FILE_PRINT:
		{
			Print();
			break;
		}
		case MENU_MACRO_START_RECORDING:
		{
			commandManager->StartMacro();
			break;
		}
		case MENU_MACRO_STOP_RECORDING:
		{
			commandManager->StopMacro();
			break;
		}
		case P_C_MACRO_TYPE:
		{
			message->PrintToStream();
			commandManager->PlayMacro(message);
			break;
		}
		case  B_SAVE_REQUESTED:
		{
				entry_ref *ref	= new entry_ref;
				const char* name;
				message->FindRef("directory",ref);
				message->FindString("name", &name);
				SetEntry(ref,name);
				Save();
			break;
		}
		case B_COPY:
		{
			BMessage	*copyMessage	= new BMessage(P_C_EXECUTE_COMMAND);
			copyMessage->AddString("Command::Name","Copy");
			copyMessage->AddBool("shadow",true);
			commandManager->Execute(copyMessage);
			break;
		}
		case B_PASTE:
		{
			BMessage	*pasteMessage	= new BMessage(P_C_EXECUTE_COMMAND);
			pasteMessage->AddString("Command::Name","Paste");
			pasteMessage->AddBool("shadow",true);
			commandManager->Execute(pasteMessage);
			break;
		}

		case B_UNDO:
		{
			commandManager->Undo(NULL);
			break;
		}
		case B_REDO:
		{
			commandManager->Redo(NULL);
			break;
		}
		case P_C_EXECUTE_COMMAND:
		{
			commandManager->Execute(message);
			break;
		}
		default:
			BLooper::MessageReceived(message);
			break;
	}
	
}

void PDocument::Init()
{
	TRACE();
	allNodes		= new BList();
	allConnections	= new BList();
	selected		= new BList();
	valueChanged	= new BList();
	trashed			= new BList();

	printerSetting	= NULL;
	documentSetting	= new BMessage();
	bounds			= BRect(0,0,100,100);
	printableRect	= NULL;
	paperRect		= NULL;
	savePanel		= NULL;
	width			= 100;
	height			= 100;
	dirty			= true;
	entryRef		= NULL;
	modified		= false;
	windowManager	= new WindowManager(this);
	editorManager	= new PEditorManager(this);
	commandManager	= new PCommandManager(this);
	//** to do.. helpManager		= new HelpManager();
}

void PDocument::Init(BMessage *archive)
{
	TRACE();
	
}

const char* PDocument::Title(void)
{
	TRACE();
	if (ReadLock())
	{
		if (entryRef)
			return entryRef->name;
		else
			return "Untitled";
		ReadUnlock();
	}
	else
	{
		PRINT(("\tDEBUG:\tTitle() - Cant ReadLock()\n"));
		return NULL;
	}
}

void PDocument::Resize(float toX,float toY)
{
	TRACE();
	bounds.right	= toX;
	bounds.bottom	= toY;
	PRINT(("\tDEBUG:\tResize() - new Rect\n"));
	PRINT_OBJECT(bounds);
	editorManager->BroadCast(new BMessage(P_C_DOC_BOUNDS_CHANGED));
}


void PDocument::SetDocumentSettings(BMessage *settings)
{
	TRACE();
	if (settings!=NULL)
	{
		delete documentSetting;
		documentSetting	= settings;
	}
}
BMessage* PDocument::PrintSettings(void)
{
	TRACE();
	status_t result = B_OK;
	BPrintJob printJob("document's name");
	if (! printerSetting) 
	{
		result = printJob.ConfigPage();
		if ((result == B_OK) && (WriteLock())) 
		{
			printerSetting	= printJob.Settings();
			paperRect 		= new BRect(printJob.PaperRect());
			printableRect	= new BRect(printJob.PrintableRect());
			SetModified();
			WriteUnlock();
		}
	} 
	return printerSetting;
}
void PDocument::SetPrintSettings(BMessage *settings)
{
	TRACE();
	if (ReadLock())
	{
		if (settings!=NULL)
		{
			delete printerSetting;
			printerSetting	= settings;
		}
		ReadUnlock();
	}
}


void PDocument::Print(void)
{
	TRACE();
	status_t	result	= B_OK;
	BPrintJob	printJob("doc_name"); 
	if (ReadLock())
	{
		if (!printerSetting)
		{
			result = printJob.ConfigPage();
			if (result == B_OK)
			{
				// Get the user Settings
				printerSetting = printJob.Settings();
				// Use the new settings for your internal use
				paperRect = new BRect(printJob.PaperRect());
				printableRect = new BRect(printJob.PrintableRect());
			}
		}
		// Setup the driver with user settings
		printJob.SetSettings(printerSetting);
		
		result = printJob.ConfigJob();
		if (result == B_OK)
		{
			// WARNING: here, setup CANNOT be NULL.
	         if (printerSetting == NULL) 
	         {
	            // something's wrong, handle the error and bail out
	         }
//**dont know why this was here :-)      		 delete printerSetting;

			// Get the user Settings
			printerSetting	= printJob.Settings(); 
			paperRect 		= new BRect(printJob.PaperRect());
			printableRect	= new BRect(printJob.PrintableRect());
  
			// Now you have to calculate the number of pages
			// (note: page are zero-based)
			int32 firstPage = printJob.FirstPage();
			int32 lastPage = printJob.LastPage();
        
			// Verify the range is correct
			// 0 ... LONG_MAX -> Print all the document
			// n ... LONG_MAX -> Print from page n to the end
			// n ... m -> Print from page n to page m
			PEditor *activePEditor=editorManager->GetActivPEditor();
			if (activePEditor != NULL)
			{
				BView *editorView=activePEditor->GetView();
				if (editorView != NULL)
				{
					// calculate the Number of Pages
					bool locked	= editorView->LockLooper();
					int32 xPages,yPages;
					BRect editorRect = editorView->Bounds();
					xPages=editorRect.IntegerWidth()/printableRect->IntegerWidth();
					if (editorRect.IntegerWidth()%printableRect->IntegerWidth() > 0)
						xPages++;
					yPages=editorRect.IntegerHeight()/printableRect->IntegerHeight();
					if (editorRect.IntegerHeight()% printableRect->IntegerHeight() > 0)
						yPages++;
					int32 document_last_page= xPages*yPages;
					if (lastPage > document_last_page)
						lastPage = document_last_page;
					int32 nbPages = lastPage - firstPage + 1;
					if (nbPages <= 0)
						;
						//**return B_ERROR;
					// Now we can print the page
					printJob.BeginJob();
					// Print all pages
						bool can_continue = printJob.CanContinue(); 
					for (int i=firstPage-1; can_continue && i<lastPage ; i++) 
					{
						float xStart	= (i%xPages)*printableRect->Width();
						float yStart	= (i/xPages)*printableRect->Height();
						BRect viewRect(xStart,yStart,xStart+printableRect->Width(),yStart+printableRect->Height());
//						printJob.DrawView(editorView, viewRect, BPoint(0,0)); 
						printJob.DrawView(editorView, viewRect, BPoint(printableRect->left,printableRect->top)); 
						printJob.SpoolPage(); 
						/*if (user_has_canceled)
						{
		       	        // tell the print_server to cancel the printjob
   	 	   	        	printJob.CancelJob();
							can_continue = false;
							break;
   	         		}*/
						can_continue = printJob.CanContinue(); 
					}
					if (can_continue)
						printJob.CommitJob();
					else
						result = B_ERROR; 
					if (locked) editorView->UnlockLooper();
				}
			}
			else
			{
				//**error like no active Editor
			}
		}
		ReadUnlock();
	}
}


void PDocument::SetEntry(entry_ref *saveEntry,const char *name)
{
	TRACE();
	if (ReadLock())
	{
		if (saveEntry!=NULL)
		{
			BDirectory dir(saveEntry);
			BEntry entry(&dir, name);
			if (entryRef == NULL)
				entryRef= new entry_ref;
			entry.GetRef(entryRef);
		}
		ReadUnlock();
	}
	else
	{
		new BAlert("",_T("Error setting File"),"Ohh");
	}
	
}

void PDocument::Save(void)
{
	TRACE();
	Indexer		*indexer				= new Indexer(this);
	status_t	err 					= B_OK;
	int32		i						= 0;
	BMessage	*tmpMessage				= new BMessage();
	BMessage	*commandManage			= new BMessage();
	BMessage	*tmpNode				= NULL;
	BMessage	*allNodesMessage		= new BMessage();
	BMessage	*allConnectionsMessage	= new BMessage();
	BMessage	*selectedMessage		= new BMessage();

	if (printerSetting)
		tmpMessage->AddMessage("PDocument::printerSetting",printerSetting);
	tmpMessage->AddMessage("PDocument::documentSetting",documentSetting);
	//save all Nodes
	for (i=0; i<allNodes->CountItems();i++)
	{
		tmpNode=(BMessage *)allNodes->ItemAt(i);
		allNodesMessage->AddMessage("node",indexer->IndexNode(tmpNode));
	}
	tmpMessage->AddMessage("PDocument::allNodes",allNodesMessage);
	//save all Connections
	for (i=0; i<allConnections->CountItems();i++)
	{
		tmpNode=(BMessage *)allConnections->ItemAt(i);
		allConnectionsMessage->AddMessage("node",indexer->IndexConnection(tmpNode));
	}
	tmpMessage->AddMessage("PDocument::allConnections",allConnectionsMessage);
	//save the selected List
	for (i=0; i<selected->CountItems();i++)
	{
		selectedMessage->AddPointer("node",selected->ItemAt(i));
	}
	tmpMessage->AddMessage("PDocument::selected",selectedMessage);
	//save all Command related Stuff like Undo/Makor
//	commandManager->Archive(commandManage);
	for (i=0;i<(commandManager->GetUndoList())->CountItems();i++)
	{
		commandManage->AddMessage("undo",indexer->IndexMacroCommand((BMessage *)(commandManager->GetUndoList())->ItemAt(i)));

	}
	for (i=0;i<(commandManager->GetMacroList())->CountItems();i++)
	{
		commandManage->AddMessage("macro",(BMessage *)(commandManager->GetMacroList())->ItemAt(i));

	}
	commandManage->AddInt32("undoStatus",commandManager->GetUndoIndex());
	tmpMessage->AddMessage("PDocument::commandManager", commandManage);	
	
	if (entryRef) 
	{
		BFile *file=	new BFile(entryRef,B_WRITE_ONLY | B_ERASE_FILE | B_CREATE_FILE);
		err=file->InitCheck();
		PRINT(("ERROR\tSave file error %s\n",strerror(err)));
		err = tmpMessage->Flatten(file);
	}
	if (err==B_OK)
	{
		modified=false;
	}
	else
		PRINT(("ERROR:\tPDocument","Save error %s\n",strerror(err)));
	delete	indexer;
	delete	tmpMessage;
	delete	commandManage;
	//delete	tmpNode;
	delete	allNodesMessage;
	delete	allConnectionsMessage;
	delete	selectedMessage;
}

void PDocument::Load(void)
{
	TRACE();
	PDocLoader	*docLoader	= new PDocLoader(this,new BEntry(entryRef));
	delete allNodes;
	delete allConnections;
	delete printerSetting;
	delete selected;
	BMessage	*node	= NULL;
	
	allNodes 		= docLoader->GetAllNodes();
	for (int32 i=0;i<allNodes->CountItems();i++)
	{
		node=((BMessage*)allNodes->ItemAt(i));
		node->AddPointer("doc",this);
		valueChanged->AddItem(node);
	}
	allConnections	= docLoader->GetAllConnections();
	for (int32 i=0;i<allConnections->CountItems();i++)
	{
		node= (BMessage *)allConnections->ItemAt(i);
		node->AddPointer("doc",this);
		valueChanged->AddItem(node);
	}
	selected = docLoader->GetSelectedNodes();
	delete commandManager;
	commandManager= new PCommandManager(this);
	commandManager->SetMacroList(docLoader->GetMacroList());
	commandManager->SetUndoList(docLoader->GetUndoList());
	commandManager->SetUndoIndex(docLoader->GetUndoIndex());
//	commandManager->LoadMacros(docLoader->GetCommandManagerMessage());
//	commandManager->LoadUndo(docLoader->GetCommandManagerMessage());
	printerSetting	= docLoader->GetPrinterSetting();
	editorManager->BroadCast(new BMessage(P_C_VALUE_CHANGED));
}

void PDocument::SavePanel()
{
	TRACE();
	if (!savePanel)
	{
		savePanel = new BFilePanel(B_SAVE_PANEL, new BMessenger(this),NULL,0,false);
	}
	if (entryRef) savePanel->SetPanelDirectory(entryRef);
	savePanel->Window()->SetWorkspaces(B_CURRENT_WORKSPACE);
	savePanel->Show();
}


bool PDocument::QuitRequested(void)
{
	TRACE();
	//check modified if there are changes wich we need to save
	bool	returnValue = true;
	bool	readLock	= false;
	if (modified)
	{
		readLock = ReadLock();
		BAlert *myAlert = new BAlert("title", "Save changes to ...", _T("Cancel"), _T("Don't save"), _T("Save"), B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT); 
		myAlert->SetShortcut(0, B_ESCAPE);
		int32 button_index = myAlert->Go();
		if (button_index == 0)
			returnValue = false;
		else if (button_index == 1)
			returnValue =  true;
		else
		{
			Save();
			returnValue	= true;
		}
		if (readLock)
		ReadUnlock();
	}
	return returnValue;
}

BMessage* PDocument::FindObject(BPoint *where)
{
	TRACE();
	BRect		*frame	= new BRect(-10,-10,-10,-10);
	BMessage	*node	= NULL;
	bool		found	= false;
	for (int32 i=0;( (i<allNodes->CountItems())&&(!found) );i++)
	{
		node=(BMessage *)allNodes->ItemAt(i);
		node->FindRect("Frame",frame);
		if (frame->Contains(*where))
			found=true;
	}
	if (found)
		return node;
	else
		return NULL;
}	

BMenu *PDocument::GetMenu(const char* signatur)
{
	BMenu **alleMenus	= new BMenu*[windowManager->CountPWindows()+1];
	int32 	i			= 0;
	for (i=0; i<windowManager->CountPWindows(); i++)
	{
		alleMenus[i]=(windowManager->PWindowAt(i))->GetMenu(signatur);
	}
	alleMenus[i+1] = NULL;
	return alleMenus[0];
}

BMenuItem*	PDocument::GetMenuItem(const char* signatur)
{
	BMenuItem **alleMenuItems	= new BMenuItem*[windowManager->CountPWindows()+1];
	int32 	i					= 0;
	for (i=0; i<windowManager->CountPWindows(); i++)
	{
		alleMenuItems[i]=(windowManager->PWindowAt(i))->GetMenuItem(signatur);
	}
	alleMenuItems[i+1] = NULL;
	return alleMenuItems[0];
}

ToolBar* PDocument::GetToolBar(const char *signatur)
{
	ToolBar **alleToolBars		= new ToolBar*[windowManager->CountPWindows()+1];
	int32 	i					= 0;
	for (i=0; i<windowManager->CountPWindows(); i++)
	{
		alleToolBars[i]=(windowManager->PWindowAt(i))->GetToolBar(signatur);
	}
	alleToolBars[i+1] = NULL;
	return alleToolBars[0];
}

ToolMenu* PDocument::GetToolMenu(const char* toolbarSignature,const char *signature)
{
	ToolMenu **alleToolMenus	= new ToolMenu*[windowManager->CountPWindows()+1];
	int32 	i					= 0;
	for (i=0; i<windowManager->CountPWindows(); i++)
	{
		alleToolMenus[i]=(windowManager->PWindowAt(i))->GetToolMenu(toolbarSignature,signature);
	}
	alleToolMenus[i+1] = NULL;
	return alleToolMenus[0];
}

ToolItem* PDocument::GetToolItem(const char* toolbarSignature,const char *signature)
{
	ToolItem **alleToolItems	= new ToolItem*[windowManager->CountPWindows()+1];
	int32 	i					= 0;
	for (i=0; i<windowManager->CountPWindows(); i++)
	{
		alleToolItems[i]=(windowManager->PWindowAt(i))->GetToolItem(toolbarSignature,signature);
	}
	alleToolItems[i+1] = NULL;
	return alleToolItems[0];
}

status_t PDocument::AddMenu(BMenu *menu,int32 index = -1)
{
	status_t 	err	= B_OK;
	int32 		i	= 0;
	for (i=0; i<windowManager->CountPWindows(); i++)
	{
//		err = err || (windowManager->PWindowAt(i))->AddMenu(new BMenu(*menu),index);
		err = err || (windowManager->PWindowAt(i))->AddMenu(menu,index);
	}
	return err;
}

status_t PDocument::AddMenuItem(const char *menuSignatur, BMenuItem *menuItem,int32 index = -1)
{
	status_t 	err	= B_OK;
	int32 		i	= 0;
	for (i=0; i<windowManager->CountPWindows(); i++)
	{
//		err = err || (windowManager->PWindowAt(i))->AddMenuItem(menuSignatur,new BMenuItem(*menuItem),index);
		err = err || (windowManager->PWindowAt(i))->AddMenuItem(menuSignatur,menuItem,index);
	}
	return err;
}

status_t PDocument::AddToolBar(ToolBar *toolbar)
{
	status_t 	err	= B_OK;
	int32 		i	= 0;
	for (i=0; i<windowManager->CountPWindows(); i++)
	{
		err = err || (windowManager->PWindowAt(i))->AddToolBar(toolbar);
	}
	return err;
}

status_t PDocument::AddToolMenu(const char *toolbarSignatur,ToolMenu *toolMenu)
{
	status_t 	err	= B_OK;
	int32 		i	= 0;
	for (i=0; i<windowManager->CountPWindows(); i++)
	{
		err = err || (windowManager->PWindowAt(i))->AddToolMenu(toolbarSignatur,toolMenu);
	}
	return err;
}

status_t PDocument::AddToolItem(const char *toolbarSignatur,const char *toolmenuSignatur,ToolItem *toolItem,int32 index = -1)
{
	status_t 	err	= B_OK;
	int32 		i	= 0;
	for (i=0; i<windowManager->CountPWindows(); i++)
	{
		err = err || (windowManager->PWindowAt(i))->AddToolItem(toolbarSignatur,toolmenuSignatur,toolItem,index);
	}
	return err;
}

status_t PDocument::RemoveMenu(const char* menuSignature) 
{
	status_t 	err	= B_OK;
	int32 		i	= 0;
	for (i=0; i<windowManager->CountPWindows(); i++)
	{
		err = err || (windowManager->PWindowAt(i))->RemoveMenu(menuSignature);
	}
	return err;
}

status_t PDocument::RemoveMenuItem(const char* menuItemSignature)
{
	status_t 	err	= B_OK;
	int32 		i	= 0;
	for (i=0; i<windowManager->CountPWindows(); i++)
	{
		err = err || (windowManager->PWindowAt(i))->RemoveMenuItem(menuItemSignature);
	}
	return err;
}

status_t PDocument::RemoveToolBar(const char* signature)
{
	status_t 	err	= B_OK;
	int32 		i	= 0;
	for (i=0 ;i<windowManager->CountPWindows(); i++)
	{
		err = err || (windowManager->PWindowAt(i))->RemoveToolBar(signature);
	}
	return err;
}

status_t PDocument::RemoveToolMenu(const char* toolbarSignature,const char* toolmenuSignature) 
{
	status_t 	err	= B_OK;
	int32 		i	= 0;
	for (i=0; i<windowManager->CountPWindows(); i++)
	{
		err = err || (windowManager->PWindowAt(i))->RemoveToolMenu(toolbarSignature,toolmenuSignature);
	}
	return err;
}

status_t PDocument::RemoveToolItem(const char* toolbarSignature,const char* toolitemSignature)
{
	status_t 	err	= B_OK;
	int32 		i	= 0;
	for (i=0; i<windowManager->CountPWindows(); i++)
	{
		err = err || (windowManager->PWindowAt(i))->RemoveToolItem(toolbarSignature,toolitemSignature);
	}
	return err;
}


BMessage* PDocument::ReIndex(BList *nodeList)
{
	BMessage	*returnMessage	= new BMessage();
	BMessage	*node			= NULL;
	BList		*subList		= NULL;
	for (int32 i=0;i<nodeList->CountItems();i++)	
	{
		node =(BMessage *) nodeList->ItemAt(i);
		node->AddPointer("this",node);
		if ((node->FindPointer("SubContainer",(void **)&subList) == B_OK) && (subList != NULL) )
			node->AddMessage("SubContainerList",ReIndex(subList));
		returnMessage->AddMessage("node",node);
	}
	return returnMessage;
}


