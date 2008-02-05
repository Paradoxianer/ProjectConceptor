#include "ToolBar.h"
#include "ToolBarSeperator.h"
#include "Mover.h"
#include "ToolItem.h"
#include "ToolMenu.h"
#include <math.h>
#include <support/ClassInfo.h>

#if __GNUC__ >= 3
#define max(a,b) ((a)>(b))?a:b
#define min(a,b) ((a)<(b))?a:b
#endif

ToolBar::ToolBar(BRect rect,  const char *name,menu_layout ori): BControl(rect, name,"ToolBar",NULL, B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW|B_FULL_UPDATE_ON_RESIZE)
{
	TRACE();
	SetViewColor(ui_color(B_MENU_BACKGROUND_COLOR));
	SetDrawingMode(B_OP_ALPHA);
	right_margin			= top_margin=bottom_margin=left_margin=2;
	toolitems				= new BList();
	tool_bar_menu_layout	= ori;
	mouseTrace				= false;
	klickedItem				= NULL;
	eventReciver			= NULL;
	countx					= 1;
	county					= 1;
	rightIconBorder			= 1;
	bottomIconBorder		= 1;
	
	if (ori == B_ITEMS_IN_ROW)
	{
		BView::ResizeTo(rect.Width()-1, top_margin+ITEM_HEIGHT+bottom_margin);
		vorward_mover=new Mover(BRect(0,0,MOVER_WIDTH,rect.Height()),ori,P_M_VORWARD);
		backward_mover=new Mover(BRect(rect.Width()-MOVER_WIDTH,0,rect.Width(),rect.Height()),ori,P_M_BACKWARD);
	}
	else if (ori == B_ITEMS_IN_COLUMN)
	{
		SetResizingMode(B_FOLLOW_TOP_BOTTOM);
		BView::ResizeTo(left_margin+ITEM_WIDTH+right_margin,rect.Height());
		vorward_mover=new Mover(BRect(0,0,rect.Width(),MOVER_WIDTH),ori,P_M_VORWARD);
		backward_mover=new Mover(BRect(0,rect.Height()-MOVER_WIDTH,rect.Width(),rect.Height()),ori,P_M_BACKWARD);
	}
	else
	{
		SetResizingMode(B_FOLLOW_ALL_SIDES);
	}
}
ToolBar::ToolBar(BMessage *archive):BControl(archive)
{
	TRACE();
	status_t err=B_OK;
	int32 i=0;
//	Init();

	//**check if the tName ist good??
	BMessage tmpArchive;
	ToolItem *tmpItem;
	ToolBarSeperator *tmpSeperator;
	
	while (err==B_OK)
	{
		err = archive->FindMessage("ToolBar::toolitems",i,&tmpArchive);
		if (err==B_OK)
		{
			if ( !validate_instantiation(&tmpArchive, "ToolItem"))
				tmpItem=new ToolItem(&tmpArchive);
			else if ( !validate_instantiation(&tmpArchive, "ToolBarSeperator"))
				tmpSeperator= new ToolBarSeperator(&tmpArchive);	
		}
		i++;
	}
	err = archive->FindInt32("ToolBar::tool_bar_menu_layout",(int32 *)&tool_bar_menu_layout);
	err = archive->FindMessage("ToolBar::vorward_mover",&tmpArchive);
	if (err == B_OK)
		vorward_mover = new Mover(&tmpArchive);
	err = archive->FindMessage("ToolBar::backward_mover",&tmpArchive);
	if (err == B_OK)
		backward_mover = new Mover(&tmpArchive);
	err = archive->FindFloat("ToolBar::left_margin",&left_margin);
	err = archive->FindFloat("ToolBar::right_margin",&right_margin);
	err = archive->FindFloat("ToolBar::top_margin",&top_margin);
	err = archive->FindFloat("ToolBar::bottom_margin",&bottom_margin);
	err = archive->FindBool("ToolBar::mouseTrace",&mouseTrace);
	err = archive->FindInt32("ToolBar::border",(int32 *)&border);
	err = archive->FindInt32("ToolBar::countx",(int32 *)&countx);
	err = archive->FindInt32("ToolBar::county",(int32 *)&county);
}

ToolBar::~ToolBar()
{
}

status_t ToolBar::Archive(BMessage *archive, bool deep) const
{
	TRACE();
	status_t err=B_OK;
	err = BView::Archive(archive,deep);
	err = archive->AddString("class", "ToolBar"); 
	BMessage tmpArchive;
	BaseItem *baseItem;
	for (int32 i=0;i<toolitems->CountItems();i++)
	{
			baseItem=(BaseItem *)(toolitems->ItemAt(i));
			if (baseItem->Archive(&tmpArchive, deep) == B_OK)
				err = archive->AddMessage("ToolBar::toolitems",&tmpArchive);
	}
	err = archive->AddInt32("ToolBar::tool_bar_menu_layout",(int32)tool_bar_menu_layout);
	tmpArchive.MakeEmpty();
	if (vorward_mover->Archive(&tmpArchive, deep) == B_OK)
		err = archive->AddMessage("ToolBar::vorward_mover",&tmpArchive);
	if (backward_mover->Archive(&tmpArchive, deep) == B_OK)
		err = archive->AddMessage("ToolBar::backward_mover",&tmpArchive);
	err = archive->AddFloat("ToolBar::left_margin",left_margin);
	err = archive->AddFloat("ToolBar::right_margin",right_margin);
	err = archive->AddFloat("ToolBar::top_margin",top_margin);
	err = archive->AddFloat("ToolBar::bottom_margin",bottom_margin);
	err = archive->AddBool("ToolBar::mouseTrace",mouseTrace);
	err = archive->AddInt32("ToolBar::border",(int32)border);
	err = archive->AddInt32("ToolBar::countx",(int32)countx);
	err = archive->AddInt32("ToolBar::county",(int32)county);		
	return err;	
}

BArchivable* ToolBar::Instantiate(BMessage *archive)
{
	TRACE();
	if ( !validate_instantiation(archive, "ToolBar") )
		return NULL;
	return new ToolBar(archive); 
}

void ToolBar::AddItem(BaseItem *item)
{
	TRACE();
	if (item != NULL)
	{
		item->AttachedToToolBar(this);
		BaseItem *baseItem=(BaseItem *)(toolitems->LastItem());
		if (baseItem)
		{
			if (tool_bar_menu_layout == B_ITEMS_IN_ROW)
				item->MoveTo((baseItem->Frame()).right+right_margin+left_margin,top_margin);
			else if (tool_bar_menu_layout == B_ITEMS_IN_COLUMN)
				item->MoveTo(left_margin,(baseItem->Frame()).bottom+bottom_margin+top_margin);
			rightIconBorder		= item->Frame().right+right_margin;
			bottomIconBorder	= item->Frame().bottom+bottom_margin;
		}
		else
		{
			item->MoveTo(left_margin,top_margin);
			rightIconBorder		= item->Frame().right+right_margin;
			bottomIconBorder	= item->Frame().bottom+bottom_margin;
		}
		toolitems->AddItem(item);
		if (tool_bar_menu_layout == B_ITEMS_IN_MATRIX)
		{
			countx=(uint32)ceil(sqrt(toolitems->CountItems())-0.5);
			county=(uint32)ceil(sqrt(toolitems->CountItems()));
			ReorderItems();
		}
	}
}

/**
 * @todo implemet a right version :-)
 */
void ToolBar::RemoveItem(BaseItem *item)
{
	TRACE();
	if (item != NULL)
	{
		item->DetachedFromToolBar(this);
		toolitems->RemoveItem(item);	
		if (tool_bar_menu_layout == B_ITEMS_IN_MATRIX)
		{
			countx=(uint32)ceil(sqrt(toolitems->CountItems())-0.5);
			county=(uint32)ceil(sqrt(toolitems->CountItems()));
			ReorderItems();
		}
		else
		{
			BaseItem *actItem	= NULL;
			BaseItem *oldItem	= NULL;
			for (int32 i = 0 ; i<toolitems->CountItems();i++)
			{
				actItem= (BaseItem *)toolitems->ItemAt(i);
				if (oldItem)
				{
					if (tool_bar_menu_layout == B_ITEMS_IN_ROW)
						item->MoveTo((oldItem->Frame()).right+right_margin+left_margin,top_margin);
					else if (tool_bar_menu_layout == B_ITEMS_IN_COLUMN)
						item->MoveTo(left_margin,(oldItem->Frame()).bottom+bottom_margin+top_margin);
					rightIconBorder		= item->Frame().right+right_margin;
					bottomIconBorder	= item->Frame().bottom+bottom_margin;
				}
				else
				{
					item->MoveTo(left_margin,top_margin);
					rightIconBorder		= item->Frame().right+right_margin;
					bottomIconBorder	= item->Frame().bottom+bottom_margin;
				}
				oldItem=actItem;	
			}
		}
		
	}
}

void ToolBar::AddSeperator(void)
{
	TRACE();

	ToolBarSeperator *seperator = new ToolBarSeperator(tool_bar_menu_layout);
//	toolitems->AddItem(seperator);
	ToolBar::AddItem(seperator);
}

void ToolBar::RemoveSeperator(void)
{
	bool 				found		= false;
	ToolBarSeperator	*seperator	= NULL;
	for (int32 i = toolitems->CountItems()-1;((!found) && (i>0)); i--)
	{
		seperator = dynamic_cast <ToolBarSeperator *>((BaseItem *)toolitems->ItemAt(i));
		if (seperator != NULL)
		{
			RemoveItem(seperator);
			found=true;
		}
	}
}

void ToolBar::DrawAfterChildren(BRect updateRect)
{
	vorward_mover->Draw(this,updateRect);
	backward_mover->Draw(this,updateRect);
}
void ToolBar::SetLayout(menu_layout justification)
{
	TRACE();
	if (justification != tool_bar_menu_layout)
	{
		tool_bar_menu_layout=justification;
		vorward_mover->SetLayout(tool_bar_menu_layout);
		backward_mover->SetLayout(tool_bar_menu_layout);
		int i=0;
		ToolBarSeperator	*seperator = NULL;
		BaseItem			*baseItem  = NULL;
		for (i=0;i<toolitems->CountItems();i++)
		{
			baseItem=(BaseItem *)(toolitems->ItemAt(i));
			seperator = dynamic_cast <ToolBarSeperator *>(baseItem);
			if (seperator != NULL)
				//**toolitems umsortieren
				seperator->SetLayout(tool_bar_menu_layout);
		}
	}
}




void ToolBar::Draw(BRect /*updateRect*/)
{
//	BView::Draw(updateRect);
//	int i=0;
//	ToolItem *toolItem=NULL;
	/*SetDrawingMode(B_OP_OVER);
	SetHighColor(ui_color(B_UI_MENU_BACKGROUND_COLOR));
	FillRect(updateRect);*/
	rgb_color darker1=tint_color(ViewColor(),B_DARKEN_1_TINT);
	rgb_color lighter1=tint_color(ViewColor(),B_LIGHTEN_1_TINT);
	rgb_color darker2=tint_color(ViewColor(),B_DARKEN_2_TINT);
	rgb_color lighter2=tint_color(ViewColor(),B_LIGHTEN_2_TINT);
	BRect out_rect=Bounds();
	out_rect.left=1.0;
/*	BRect in_rect=out_rect;
	in_rect.InsetBy(1,1);*/
	BeginLineArray(8);
	AddLine(out_rect.LeftBottom(),out_rect.LeftTop(),lighter2);
	AddLine(out_rect.LeftTop(),out_rect.RightTop(),lighter2);
	AddLine(out_rect.RightTop(),out_rect.RightBottom(),darker2);
	AddLine(out_rect.RightBottom(),out_rect.LeftBottom(),darker2);
	/*AddLine(in_rect.LeftBottom(),in_rect.LeftTop(),lighter1);
	AddLine(in_rect.LeftTop(),in_rect.RightTop(),lighter1);
	AddLine(in_rect.RightTop(),in_rect.RightBottom(),darker1);
	AddLine(in_rect.RightBottom(),in_rect.LeftBottom(),darker1);*/
	EndLineArray();
/*//	SetDrawingMode(B_OP_ALPHA);
	for (i=0;i<toolitems->CountItems();i++)
	{
		toolItem = (ToolItem *)(toolitems->ItemAt(i));
		BRect toolRect = toolItem->Frame();
		if (toolRect.Intersects(updateRect))
			toolItem->Draw(toolRect);
	}*/
}
void ToolBar::MouseDown(BPoint point)
{
	int32	i			= 0;
	bool	contains	= false;
	ToolItem *toolItem=NULL;
	if (eventReciver!=NULL)
		eventReciver->MouseDown(point);
	while (!contains&&(i<toolitems->CountItems()))
	{
		toolItem = (ToolItem *)(toolitems->ItemAt(i));
		BRect toolRect = toolItem->Frame();
		contains=toolRect.Contains(point);
		if (contains)
		{
			toolItem->MouseDown(point);
			toolItem->Draw(toolRect);
			klickedItem=toolItem;
			SetMouseEventMask(B_POINTER_EVENTS,B_LOCK_WINDOW_FOCUS | B_NO_POINTER_HISTORY);
			mouseTrace=true;
		}	
		i++;
	}
}
void ToolBar::MouseUp(BPoint point)
{
	if (eventReciver!=NULL)
		eventReciver->MouseUp(point);
	if (mouseTrace)
	{
/*		uint32	i			= 0;
		bool	contains	= false;
		ToolItem *toolItem=NULL;
		while (!contains&&(i<toolitems->CountItems()))
		{
			toolItem = (ToolItem *)(toolitems->ItemAt(i));
			BRect toolRect = toolItem->Frame();
			contains=toolRect.Contains(point);
			if (contains)
			{
				toolItem->MouseUp(point);
				toolItem->Draw(this,toolRect);
			}
			i++;
		}
		if toolItem==klickedItem*/
		BRect toolRect =klickedItem->Frame();
		klickedItem->MouseUp(point);
		klickedItem->Draw(toolRect);
		mouseTrace=false;
	}
	
}
void ToolBar::ReorderItems(void)
{
	TRACE();

	int32 i=0;
	float xPos=left_margin;
	float yPos=top_margin;
	float maxY=0;
	float maxX=0;
	BaseItem *baseItem;

	for(i=0;i<toolitems->CountItems();i++)
	{
		baseItem=(BaseItem *)(toolitems->ItemAt(i));
		baseItem->MoveTo(xPos,yPos);
		xPos=baseItem->Frame().right+right_margin+left_margin;
		maxY=max(maxY,baseItem->Frame().bottom);
		if ((i!=0)&&(((i+1)%countx) == 0))
		{	
			maxX=max(maxX,baseItem->Frame().right+right_margin);
			xPos=left_margin;
			yPos=maxY+bottom_margin+top_margin;	
		}
	}
	rightIconBorder		= maxX;
	bottomIconBorder	= maxY+bottom_margin;
}

ToolMenu* ToolBar::GetToolMenu(const char *signature)
{
	BList		*toolMenus	= new BList();
	ToolMenu	*tmpMenu	= NULL;
	int32 		i			= 0;
	int32		index		= 0;
	bool		found		= false;
	for (i=0;((i<toolitems->CountItems()) && (!found));i++)
	{
		tmpMenu = cast_as((ToolItem *)toolitems->ItemAt(i),ToolMenu);
		if (tmpMenu != NULL)
		{
			if (strcmp(tmpMenu->GetName(),signature) == B_OK)
			{
				found=true;
				index=i;
			}	
			toolMenus->AddItem(tmpMenu);
		}
	}
	for (i=0;((i<toolMenus->CountItems()) && (!found));i++)
	{
		tmpMenu=(ToolMenu *)toolMenus->ItemAt(i);
		tmpMenu=tmpMenu->GetToolMenu(signature);
		if (tmpMenu!=NULL)
			found=true;
	}
	if (found)
		return tmpMenu;
	else
		return NULL;
}

ToolItem* ToolBar::GetToolItem(const char *signature)
{
	BList		*toolMenus	= new BList();
	ToolMenu	*tmpMenu	= NULL;
	ToolItem	*tmpItem	= NULL;
	int32 		i			= 0;
	int32		index		= 0;
	bool		found		= false;
	for (i=0;((i<toolitems->CountItems()) && (!found));i++)
	{
		tmpItem	= (ToolItem *)toolitems->ItemAt(i);
		tmpMenu = cast_as(tmpItem,ToolMenu);
		if (tmpItem != NULL)
		{
			if (strcmp(tmpItem->GetName(),signature) == B_OK)
			{
				found=true;
				index=i;
			}	
			toolMenus->AddItem(tmpMenu);
		}
	}
	for (i=0;((i<toolMenus->CountItems()) && (!found));i++)
	{
		tmpMenu=(ToolMenu *)toolMenus->ItemAt(i);
		tmpItem=tmpMenu->GetToolItem(signature);
		if (tmpItem!=NULL)
			found=true;
	}
	if (found)
		return tmpMenu;
	else
		return NULL;
}
