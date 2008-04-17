#include <interface/GraphicsDefs.h>
#include <math.h>

#include "PagedView.h"


PagedView::PagedView(BRect rect,char _name):BView(rect,"PagedView",B_FOLLOW_ALL_SIDES,B_WILL_DRAW)
{
	TRACE();
	Init();
}

void PagedView::Init(void)
{
	TRACE();
	printRect		= BRect();
	pageSize		= BRect();;
	renderBitmap	= new BBitmap(Bounds(),B_RGB32,true);
	colums			= 1;
	ows				= 1;
	aged			= false
	pageMode		= 0;
	childList		= vector<PagedRect>();
}


void PagedView::AddChild(BView* _view){
	renderBitmap->AddChild(_view);
}

bool PagedView::RemoveChild(BView *_view){
	return renderBitmap->RemoveChild(_view);
}

BView* PagedView::ChildAt(int32 index){
	return renderBitmap->ChildAt(index);

}

int32 PagedView::CountChildren(void){
	return renderBitmap->CountChildren();
}

void PagedView::Draw(BRect updateRect)
{
	if (paged)
		DrawPages(updateRect);
}

void PagedView::MouseDown(BPoint where)
{
	BView *found	= NULL;
	if (paged){
		//find the page the user clicked on :)
		vector<PagedRect>::iterator it;
		for ( it=myvector.begin() ; (it < myvector.end()) && (found == NULL); it++ ){
			if (*it.PageRect().Contains(where))
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
		for ( it=myvector.begin() ; (it < myvector.end()) && (found == NULL); it++ ){
			if (*it.PageRect().Contains(where))
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
		for ( it=myvector.begin() ; (it < myvector.end()) && (found == NULL); it++ ){
			if (*it.PageRect().Contains(where))
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
}


void PagedView::DrawPages(BRect updateRect)
{
	vector<PagedRect>::iterator it;
	BRect						sourceRect;
	BRect						printingRect;
	BRect						paperRect;
	rgb_color					restoreHighColor;
	PageRect
	for ( it=myvector.begin() ; it < myvector.end(); it++ ){
			paperRect		=(*it).PageRect();
			printingRect	=(*it).PrintRect();
			if (paperRect.Intersects(updateRect)){
				paperRect.OffsetBy(10,10);
				FillRect(paperRect,B_SOLID_LOW);
				paperRect.OffsetBy(-10,-10);
				FillRect(paperRect);
				StrokeRect(paperRect);
			}
			if (printingRect.Intersects(updateRect)){
				DrawBitmapAsync(renderBitmap,(*it).SourceRect(),printingRect);
				restoreHighColor=HighColor();
				SetHighColor(0,0,255,200);
				SetPenSize(2.0);
				StrokeRect(targetRect);
				SetHighColor(restoreHighColor);
			}
	}
	Sync();
}

void PagedView::CalculatePages(void){
	BRect		sourceArea;
	BRect		printArea;
	BRect		paperArea;
	PagedRect	*pageArea;
	int32		q, i, cx, cy,count;
	float		x, y, nx, ny;
	float		leftDiff, topDiff;
	leftDiff	= printRect.left - paperRect.left;
	topDiff		= printRect.top - paperRect.top;
	columns = ceil(Bounds->Width()/printRect.Width());
	row		= ceil(Bounds->Height()/printRect.Height());
	childList.clear();
	for (q=0; q< rows; q++)
		for (i=0;i<columns;i++)
		{
			x=i*printRect.Width();
			y=q*printRect.Height();
			sourceRect	= BRect(x,y,x+printRect.Width(),y+printRect.Heigth());
			count = q*columns+x;
			if (pageLayout == COL_AS_NEEDED){
				cy = q;
				cx = i;
			}
			else {
				cx = count % pageLayout;
				cy = floor(count / pageLayout);
			}

			nx = (cx*pageSize.Width())+cx*margin;
			ny = (cy*pageSize.Height())+cy*margin;
			printArea	= BRect (nx+leftDiff,ny+topDiff,nx+printRect.Width(),ny+printRect.Height());
			paperArea	= BRect(nx,ny,nx+pageSize.Width(),ny+pageSize.Height());
/*			if (paperRect.Intersects(updateRect))
			{
				paperRect.OffsetBy(10,10);
				FillRect(paperRect);
				paperRect.OffsetBy(-10,-10);
				FillRect(paperRect);
				DrawRect(paperRect);
				DrawRect(targetRect);
			}
			if (targetRect.Intersects(updateRect))
				DrawBitmapAsync(renderBitmap,drawRect,targetRect);
				DrawRect(targetRect);*/
			pagedArea	= new PagedRect(paperArea,printArea,sourceArea);
		}

}
