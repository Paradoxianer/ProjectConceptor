#include <interface/GraphicsDefs.h>
#include <math.h>
#include <Debug.h>

#include "PagedView.h"



PagedView::PagedView(BRect _rect,char *_name,uint32 resizingMode,uint32 flags,page_layout _pageLayout):BView(_rect,_name,resizingMode,flags){
	pageLayout=_pageLayout;
	TRACE();
	Init();
}

void PagedView::Init(void){
	TRACE();
	printRect		= BRect();
	pageRect		= BRect();;
	renderBitmap	= new BBitmap(Bounds(),B_RGB32,true);
	drawView		= new BView(BRect(Bounds()),"drawView",0,0);
	colums			= 1;
	rows			= 1;
	paged			= true;
	margin			= 10.0;
	childList		= vector<PagedRect>();
}

void  PagedView::AttachedToWindow(void){
	TRACE();
	SetViewColor(230,230,230,255);
	SetHighColor(0,0,0,255);
	SetLowColor(255,255,255,255);
	SetDrawingMode(B_OP_ALPHA);
}


void PagedView::Draw(BRect updateRect)
{

	
	renderBitmap->Lock();
	for (int32 i = 0; i<renderBitmap->CountChildren(); i++) {
		renderBitmap->ChildAt(i)->Draw(Bounds());
		renderBitmap->ChildAt(i)->Sync();
	}
	renderBitmap->Unlock();
	if ((paged) && (!IsPrinting()))
		DrawPages(updateRect);
	else
		DrawBitmap(renderBitmap,updateRect,updateRect);

}

void PagedView::MouseDown(BPoint where)
{
	BView *found	= NULL;
	if (paged){
		//find the page the user clicked on :)
		vector<PagedRect>::iterator it;
		for ( it=childList.begin() ; (it < childList.end()) && (found == NULL); it++ ){
			if ((*it).PageRect().Contains(where))
				found = renderBitmap->FindView(where-(*it).DiffOffset());
		}
	}
	else
		found = renderBitmap->FindView(where);
	if (found!=NULL)
		found->MouseDown(where);
	Invalidate();
}


void PagedView::MouseMoved(	BPoint where, uint32 code, const BMessage *a_message)
{
	BView *found	= NULL;
	if (paged){
		//find the page the user clicked on :)
		vector<PagedRect>::iterator it;
		for ( it=childList.begin() ; (it < childList.end()) && (found == NULL); it++ ){
			if ((*it).PageRect().Contains(where))
				found = renderBitmap->FindView(where-(*it).DiffOffset());
		}
	}
	else
		found = renderBitmap->FindView(where);
	if (found!=NULL)
		found->MouseMoved( where, code, a_message);
}

void PagedView::MouseUp(BPoint where)
{
	BView *found	= NULL;
	if (paged){
		//find the page the user clicked on :)
		vector<PagedRect>::iterator it;
		for ( it=childList.begin() ; (it < childList.end()) && (found == NULL); it++ ){
			if ((*it).PageRect().Contains(where))
				found = renderBitmap->FindView(where-(*it).DiffOffset());
		}
	}
	else
		found = renderBitmap->FindView(where);
	if (found!=NULL)
		found->MouseUp(where);
	Invalidate();
}

void PagedView::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
		default:
			BView::MessageReceived(message);
			break;
	}
}

void PagedView::FrameResized(float width, float height)
{
	TRACE();
	//**pass all Resize stuff through;
	CalculatePages();
}


void PagedView::DrawPages(BRect updateRect)
{
	vector<PagedRect>::iterator it;
	BRect						sourceRect;
	BRect						printingRect;
	BRect						paperRect;
	BRect						drawRect;
	rgb_color					restoreHighColor;
	for ( it=childList.begin() ; it < childList.end(); it++ ){
			paperRect		=(*it).PageRect();
			printingRect	=(*it).PrintRect();
			if (paperRect.Intersects(updateRect)){
				paperRect.OffsetBy(3,3);
				restoreHighColor=HighColor();
				SetHighColor(100,100,100,100);
				FillRect(paperRect);
				paperRect.OffsetBy(-3,-3);
				FillRect(paperRect,B_SOLID_LOW);
				SetHighColor(restoreHighColor);

				StrokeRect(paperRect);
			}
			drawRect	= printingRect & updateRect;
			if (drawRect.IsValid()){
				sourceRect	= drawRect;
				sourceRect.OffsetBy((*it).DiffOffset());
				DrawBitmapAsync(renderBitmap,sourceRect,printingRect);
				restoreHighColor=HighColor();
				SetHighColor(55,55,200,100);
			//	SetPenSize(1.0);
				StrokeRect(printingRect);
				SetHighColor(restoreHighColor);
			}
	}
	Sync();
}

void PagedView::SetPageRect(BRect _pageRect){
	if (_pageRect.IsValid()){
		pageRect=_pageRect;
		CalculatePages();
	}
}

void PagedView::SetPrintRect(BRect _printRect){
	if (_printRect.IsValid()){
		printRect=_printRect;
		CalculatePages();
	}
}


void PagedView::CalculatePages(void){
	TRACE();
	BRect		sourceArea;
	BRect		printArea;
	BRect		paperArea;
	PagedRect	*pageArea;
	int32		q, i, cx, cy,count;
	float		x, y, nx, ny;
	float		leftDiff, topDiff;
	leftDiff	= printRect.left - pageRect.left;
	topDiff		= printRect.top - pageRect.top;
	colums		= ceil(Bounds().Width()/printRect.Width());
	rows		= ceil(Bounds().Height()/printRect.Height());
	childList.clear();
	for (q=0; q< rows; q++)
		for (i=0;i<colums;i++)
		{
			x=i*printRect.Width();
			y=q*printRect.Height();
			sourceArea	= BRect(x,y,x+printRect.Width(),y+printRect.Height());
			count = q*colums+i;
			if (pageLayout == COL_AS_NEEDED){
				cy = q;
				cx = i;
			}
			else {
				cx = count % pageLayout;
				cy = (int32)floor(count / pageLayout);
			}

			nx = (cx*pageRect.Width())+cx*margin+margin;
			ny = (cy*pageRect.Height())+cy*margin+margin;
			printArea	= BRect (nx+leftDiff,ny+topDiff,nx+leftDiff+printRect.Width(),ny+topDiff+printRect.Height());
			paperArea	= BRect(nx,ny,nx+pageRect.Width(),ny+pageRect.Height());
			childList.push_back(PagedRect(paperArea,printArea,sourceArea));
		}

}
