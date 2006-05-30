#include "WindowManager.h"
#include <interface/Screen.h>



WindowManager::WindowManager(PDocument *initDoc)
{
	TRACE();
	document = initDoc;
	Init();
}

WindowManager::WindowManager(BMessage *archive)
{
	TRACE();
	Init();
	BMessage*	tmpMessage	= new BMessage();
	while (archive->FindMessage("WindowManager::windowList",tmpMessage))
	{
		AddPWindow(new PWindow(tmpMessage));	
	}
}

WindowManager::~WindowManager(void)
{
	//close all opened Windows
	PWindow	*tmpWindow = NULL;
	while (windowList->CountItems()>0)
	{
		tmpWindow=(PWindow *)windowList->RemoveItem(windowList->CountItems()-1);
		if (tmpWindow)
		{
			tmpWindow->Lock();
			tmpWindow->Quit();
			delete tmpWindow;
		}
	}
	delete windowList;
}

void WindowManager::Init()
{
	windowList=new BList();
	BScreen *tmpScreen	= new BScreen();
	windowRect	= tmpScreen->Frame();
	windowRect.InsetBy(50,50);
	windowRect.OffsetBy(-25,-25);
}



status_t WindowManager::Archive(BMessage *archive, bool deep = true) const
{
	TRACE();
	PWindow*	tmpWindow	= NULL;
	BMessage*	tmpMessage	= new BMessage();
	status_t	err			= B_OK;
	archive->AddString("class", "WindowManager"); 
	for (int32 i=0;i<windowList->CountItems();i++)
	{
		err 		= B_OK;
		tmpWindow	= (PWindow*)windowList->ItemAt(i);
		err			= tmpWindow->Archive(tmpMessage,deep);
		if (err == B_OK)
			err = archive->AddMessage("WindowManager::windowList",tmpMessage);
	}
	return B_OK;
}

BArchivable* WindowManager::Instantiate(BMessage *archive)
{
	TRACE();
 	if ( !validate_instantiation(archive, "WindowManager") )
      return NULL;
   return new WindowManager(archive); 
}


void WindowManager::AddPWindow(PWindow* pWindow)
{
	TRACE();
	//Check if pWindow is Valid or if it´s already in the list
	if (pWindow!=NULL)
	{
		if (!windowList->HasItem(pWindow))
		{
			if (windowList->AddItem(pWindow))
			{
				pWindow->SetManager(this);
				windowRect.OffsetBy(20,20);
				pWindow->MoveTo(windowRect.LeftTop());
				pWindow->ResizeTo(windowRect.Width(), windowRect.Height());
				BMessage *docConfig			= document->DocumentSettings();
			}
		}
	}
}

void WindowManager::RemovePWindow(PWindow* pWindow)
{
	TRACE();
	windowList->RemoveItem(pWindow);
	if (windowList->CountItems()==0)
	{
		BMessenger	*docMessenger=new BMessenger(NULL,document);
		docMessenger->SendMessage(B_QUIT_REQUESTED);
	}
}
void WindowManager::TitleWindows(int32 flag)
{
	
}

void WindowManager::CascadeWindows(void)
{
}


PWindow* WindowManager::GetPWindow(PEditor *editor)
{
	TRACE();
	bool		found		= false;
	int32		i			= windowList->CountItems();
	PWindow*	tmpWindow	= NULL;
	/*search through all registered Windows 
	 *if one of the Windows is the Parent
	 *of the editor.
	 */
	while ((!found)&&(i > 0))
	{
		i--;
		tmpWindow = (PWindow*)windowList->ItemAt(i);
		if (editor->GetView()->Window() == tmpWindow)
			found=true;
	}
	if (found)
		return tmpWindow;
	else
		return NULL;
	
}

PWindow* WindowManager::GetPWindow(uint32 flag)
{
	TRACE();
	return NULL;
}


PWindow* WindowManager::GetActivePWindow(void)
{
	TRACE();
	bool		found		= false;
	int32		i			= windowList->CountItems();
	PWindow*	tmpWindow	= NULL;
	/*search through all registered Windows 
	 *if one of the Windows is the Parent
	 *of the editor.
	 */
	while ((!found)&&(i > 0))
	{
		i--;
		tmpWindow = (PWindow*)windowList->ItemAt(i);
		if (tmpWindow->IsActive())
			found=true;
	}
	if (found)
		return tmpWindow;
	else
		return NULL;
}


