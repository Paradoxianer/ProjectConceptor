#include <interface/Rect.h>
#include <interface/Screen.h>

#include "ToolMenu.h"
#include "BitmapMenuItem.h"


ToolMenu::ToolMenu(const char *name, BBitmap *icon,menu_layout layout = B_ITEMS_IN_MATRIX):ToolItem(name, icon, NULL, P_M_ONE_STATE_ITEM)
{
	popUpLayout		= layout;
	menuWindow		= NULL;
	menuPainter		= new ToolBar(BRect(0,0,(ITEM_WIDTH*3)+4,(ITEM_HEIGHT*3)+4),"menuDrawer",popUpLayout);
	menuPainter->SetModificationMessage(new BMessage(P_C_TOOLBAR_CHANGED));
	width			= 100;
	height			= 100;
	selectedItem	= NULL;
	labelFromMarked	= false;
	mousepressed	= false;
}

ToolMenu::ToolMenu(BMessage *archive):ToolItem(archive)
{
	status_t err = B_OK;
	err = archive->FindInt32("ToolMenu::popUpLayout",(int32 *)&popUpLayout);
	err = archive->FindFloat("ToolMenu::max_width",(float *)&max_width);
	err = archive->FindFloat("ToolMenu::max_height",(float *)&max_height);
	err = archive->FindFloat("ToolMenu::width",(float *)&width);
	err = archive->FindFloat("ToolMenu::height",(float *)&height);

	BMessage tmpArchive;
	err = archive->FindMessage("ToolMenu::menuPainter",&tmpArchive);
	if (err == B_OK)
		menuPainter=new ToolBar(&tmpArchive);
	tmpArchive.MakeEmpty();
	err = archive->FindMessage("ToolMenu::menuPainter",&tmpArchive);
	if (err == B_OK)
		menuWindow=new BWindow(&tmpArchive);
}

ToolMenu::~ToolMenu(void)
{
}

status_t ToolMenu::Archive(BMessage *archive,bool deep = true) const
{
	status_t err = B_OK;
	err = ToolItem::Archive(archive,deep);
	err = archive->AddString("class", "ToolMenu"); 
	err = archive->AddInt32("ToolMenu::popUpLayout",popUpLayout);
	err = archive->AddFloat("ToolMenu::max_width",max_width);
	err = archive->AddFloat("ToolMenu::max_height",max_height);
	err = archive->AddFloat("ToolMenu::width",width);
	err = archive->AddFloat("ToolMenu::height",height);

	BMessage tmpArchive;
	if (menuPainter)
	{
		err = menuPainter->Archive(&tmpArchive,deep);
		err = archive->AddMessage("ToolMenu::menuPainter",&tmpArchive);
	}
	tmpArchive.MakeEmpty();
	if (menuWindow)
	{
		err = menuPainter->Archive(&tmpArchive,deep);
		err = archive->AddMessage("ToolMenu::menuPainter",&tmpArchive);
	}
	return err;
}

BArchivable* ToolMenu::Instantiate(BMessage *archive)
{
	if ( !validate_instantiation(archive, "ToolMenu") )
		return NULL;
	return new ToolMenu(archive); 

}

bool ToolMenu::AddItem(BaseItem *item)
{
	menuPainter->AddItem(item);
	return true;
}

/*ToolItem* ToolMenu::RemoveItem(int32 *item)
{
	
}*/

void ToolMenu::Draw(BRect updateRect)
{
	ToolItem::Draw(updateRect);
}

void ToolMenu::MouseDown(BPoint point)
{
	if ((menuWindow == NULL) || (menuWindow->IsHidden()))
	{
		//** Unterschied zwischen Vertikal und Horizontal beachten
		BScreen *screenchecker=new BScreen(B_MAIN_SCREEN_ID);
		BPoint startpoint=Frame().RightTop();
		startpoint=parentToolBar->ConvertToScreen(startpoint);
		startpoint.x++;
		if (menuWindow == NULL)
		{
			menuWindow=new BWindow(menuPainter->Bounds(),tName,B_BORDERED_WINDOW,B_NOT_CLOSABLE);
			menuWindow->SetFeel(B_FLOATING_APP_WINDOW_FEEL);
			parentToolBar->Window()->AddToSubset(menuWindow);
			menuWindow->AddChild(menuPainter);
		}
		menuWindow->Lock();
//** need remove		float widht, height;
		menuPainter->GetPreferredSize(&width,&height);

		menuWindow->ResizeTo(width,height);
		menuWindow->Unlock();	
		menuWindow->MoveTo(startpoint);
		BRect menuWindowFrame=menuWindow->Frame();
		menuWindow->Lock();	
		BRect screenFrame=screenchecker->Frame();
		if (screenFrame.right<menuWindowFrame.right)
		{
			//move Menu Window to the left
			startpoint=Frame().LeftTop();
			startpoint=parentToolBar->ConvertToScreen(startpoint);
			startpoint.x-=menuWindowFrame.Width();
			startpoint.y=menuWindowFrame.top;
			startpoint.x--;
			menuWindow->MoveTo(startpoint);
		}
		if (screenFrame.bottom<menuWindowFrame.bottom)
		{
			//reload menuWindowFrame because it coud have changed during the if bevore
			menuWindowFrame=menuWindow->Frame();
			startpoint=Frame().RightBottom();
			startpoint=parentToolBar->ConvertToScreen(startpoint);
			startpoint.y-=menuWindowFrame.Height();
			startpoint.x=menuWindowFrame.left;
			//move Menu Window to top
			menuWindow->MoveTo(startpoint);
		}
		menuWindow->Show();
		menuWindow->Unlock();
	}
	if (Bounds().Contains(point))
			mousepressed	= false;
//	ToolItem::MouseDown(BPoint(5,5));
	SetEventMask(B_POINTER_EVENTS,B_LOCK_WINDOW_FOCUS | B_NO_POINTER_HISTORY);
}
void ToolMenu::MouseUp(BPoint point)
{
	if (!Bounds().Contains(point)) 
	{
		menuWindow->Hide();
		SetEventMask(B_NO_EVENTS,B_LOCK_WINDOW_FOCUS | B_NO_POINTER_HISTORY);
		BPoint screenklick		= ConvertToScreen(point);
		BPoint	menuWindowPoint;
		int32 i=0;
		bool contains=false;
		ToolItem *toolItem=NULL;
		while (!contains&&(i<menuPainter->toolitems->CountItems()))
		{
			toolItem = (ToolItem *)(menuPainter->toolitems->ItemAt(i));
			BRect toolRect;

			if (menuPainter->LockLooper())
			{
				toolRect 		= toolItem->Frame();
				menuWindowPoint = menuWindow->ConvertFromScreen(screenklick);
				menuPainter->UnlockLooper();
			}
			contains=toolRect.Contains(menuWindowPoint);
			if (contains)
			{
			
				if ((!mousepressed) && (menuPainter->LockLooper())) 
				{
					toolItem->MouseDown(menuWindowPoint);
					toolItem->MouseUp(menuWindowPoint);
					menuPainter->UnlockLooper();
				}
				if ((labelFromMarked)&&(toolItem!=selectedItem))
				{
					if (selectedItem)
						selectedItem->SetState(P_M_ITEM_UP);
					selectedItem=toolItem;
					toolItemBitmap=selectedItem->GetBitmap();
				}
			}	
			i++;
		}
	}
	else
		mousepressed	= true;
	ToolItem::MouseUp(point);
}

void ToolMenu::SetLabelFromMarked(bool flag)
{
	labelFromMarked=flag;
	//** alle Einträge in Toolbar durchgehen und dann den ersten finden der Markiert ist..
}
void ToolMenu::MessageReceived(BMessage *message)
{
	switch(message->what) 
	{
		case P_C_TOOLBAR_CHANGED:
			{
			}
	}
}

