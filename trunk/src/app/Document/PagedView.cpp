#include <interface/GraphicsDefs.h>
#include <math.h>

#include "PagedView.h"
#include <Debug.h>


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
	colums			= 1;
	rows			= 1;
	paged			= false;
	childList		= vector<PagedRect>();
}

void  PagedView::AttachedToWindow(void){
	TRACE();
	SetViewColor(230,230,230,255);
}


void PagedView::AddChild(BView* _view){
	TRACE();
	renderBitmap->AddChild(_view);
}

bool PagedView::RemoveChild(BView *_view){
	TRACE();
	return renderBitmap->RemoveChild(_view);
}

BView* PagedView::ChildAt(int32 index) const{
	TRACE();
	return renderBitmap->ChildAt(index);

}

int32 PagedView::CountChildren(void) const{
	TRACE();
	return renderBitmap->CountChildren();
}

BView* PagedView::FindView(const char* name) const{
	TRACE();
	return renderBitmap->FindView(name);
}

BView*  PagedView::FindView(BPoint point) const{
	TRACE();
	return renderBitmap->FindView(point);
}


void PagedView::Draw(BRect updateRect)
{

	/**call all draw routines of the BViews
	wich are added to the BBitmap and sync
	them to the BBitmap
	Maby check if the View is Visible
	and therefore need to be drawn
	to save RenderTime
	maby its enought to interset the update Rect,
	move the offset in pagedView
	and then call the Childrens Draw with this rect
	*/
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
		found->MouseUp(where);
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
				paperRect.OffsetBy(10,10);
				FillRect(paperRect,B_SOLID_LOW);
				paperRect.OffsetBy(-10,-10);
				FillRect(paperRect);
				StrokeRect(paperRect);
			}
			drawRect	= printingRect & updateRect;
			if (drawRect.IsValid()){
				sourceRect	= drawRect;
				sourceRect.OffsetBy((*it).DiffOffset());
				DrawBitmapAsync(renderBitmap,sourceRect,printingRect);
				restoreHighColor=HighColor();
				SetHighColor(0,0,255,200);
				SetPenSize(2.0);
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
			count = q*colums+x;
			if (pageLayout == COL_AS_NEEDED){
				cy = q;
				cx = i;
			}
			else {
				cx = count % pageLayout;
				cy = (int32)floor(count / pageLayout);
			}

			nx = (cx*pageRect.Width())+cx*margin;
			ny = (cy*pageRect.Height())+cy*margin;
			printArea	= BRect (nx+leftDiff,ny+topDiff,nx+printRect.Width(),ny+printRect.Height());
			paperArea	= BRect(nx,ny,nx+pageRect.Width(),ny+pageRect.Height());
			childList.push_back(PagedRect(paperArea,printArea,sourceArea));
		}

}
