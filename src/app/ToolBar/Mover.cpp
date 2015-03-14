#include "Mover.h"
#include "ToolBar.h"

Mover::Mover():BArchivable()
{
	Init(BRect(0,0,5,20),B_ITEMS_IN_ROW,P_M_VORWARD);
}
Mover::Mover(BRect rect,menu_layout ori,direction direct):BArchivable()
{
	Init(rect,ori,direct);

}
Mover::Mover(BMessage *archive):BArchivable(archive)
{
	status_t err;
	err = archive->FindBool("ToolItem::highlighted", &highlighted); 
	err = archive->FindBool("Mover::enabled",enabled);
	err = archive->FindRect("Mover::frame",&frame);
	err = archive->FindInt32("Mover::mover_menu_layout",(int32 *)&mover_menu_layout);
	err = archive->FindInt32("Mover::mover_direction",(int32 *)&mover_direction);
	err = archive->FindFloat("Mover::steps",&steps);
	err = archive->FindBool("Mover::scroll",&scroll);
}

status_t Mover::Archive(BMessage *archive,bool deep) const
{
	status_t err;
	err = BArchivable::Archive(archive,deep);
	err = archive->AddString("class", "Mover"); 
	err = archive->AddBool("Mover::highlighted",highlighted);
	err = archive->AddBool("Mover::enabled",enabled);
	err = archive->AddRect("Mover::frame",frame);
	err = archive->AddInt32("Mover::mover_menu_layout",(int32)mover_menu_layout);
	err = archive->AddInt32("Mover::mover_direction",(int32)mover_direction);
	err = archive->AddFloat("Mover::steps",steps);
	err = archive->AddBool("Mover::scroll",scroll);
	return err;
}
BArchivable* Mover::Instantiate(BMessage *archive)
{
	if ( !validate_instantiation(archive, "Mover") )
		return NULL;
	return new Mover(archive); 
}

void Mover::Init(BRect rect,menu_layout ori,direction direct)
{
	mover_menu_layout=ori;
	mover_direction=direct;
	steps=1.0;
	frame=rect;
	enabled=true;
	highlighted=true;
}


void Mover::SetSteps(float stp)
{
	if ((stp<-0.5)&&(stp>0.5))
	{
		steps=(float)mover_direction*stp;
	}
	
}
void Mover::SetDirection(direction direct)
{
	if (mover_direction!=direct)
	{
		mover_direction=direct;
		steps=-1.0*steps;
	}
}


void Mover::SetLayout(menu_layout justification)
{
if (mover_menu_layout != justification)
	{
/*		float height = frame.Height();
		float width  = frame.Width();
		frame.SetRightBottom(BPoint(frame.left +height,frame.top+width));*/
		mover_menu_layout = justification;
	}	
}


void Mover::Draw(ToolBar *tb,BRect /*updateRect*/)
{
	BRect tbRect=tb->Bounds();
	if (mover_direction == P_M_VORWARD)
	{
		frame.OffsetTo(0.0,0.0);
		if (tb->GetLayout() == B_ITEMS_IN_ROW)
		{
			frame.right  = MOVER_WIDTH;
			frame.bottom = tbRect.Height();
		}
		else
		{
			frame.right  = tbRect.Width();
			frame.bottom = MOVER_WIDTH;
		}
		
	}
	else
	{
		if (tb->GetLayout() == B_ITEMS_IN_ROW)
		{
			frame=BRect(tbRect.Width()-MOVER_WIDTH,0,tbRect.Width(),tbRect.Height());
		}
		else
		{
			frame=BRect(0,tbRect.Height()-MOVER_WIDTH,tbRect.Width(),tbRect.Height());
		}
	}
	tb->SetHighColor(moverGray);
	tb->FillRect(frame);
	tb->SetHighColor(0,0,0,255);
	tb->StrokeRect(frame);
	if (tb->GetLayout() == B_ITEMS_IN_ROW)
	{
		BPoint middle=BPoint((frame.left+frame.right)/2.0,(frame.top+frame.bottom)/2.0);
		BPoint p1=BPoint(middle.x-mover_direction,middle.y-1);
		BPoint p2=BPoint(middle.x-mover_direction,middle.y+1);
		BPoint p3=BPoint(middle.x+mover_direction,middle.y);
//		tb->StrokeTriangle(p1,p2,p3);
		tb->FillTriangle(p1,p2,p3);
	}
	else
	{
		BPoint middle=BPoint((frame.left+frame.right)/2.0,(frame.top+frame.bottom)/2.0);
		BPoint p1=BPoint(middle.x-1,middle.y-mover_direction);
		BPoint p2=BPoint(middle.x+1,middle.y-mover_direction);
		BPoint p3=BPoint(middle.x,middle.y+mover_direction);
		tb->StrokeTriangle(p1,p2,p3);
	}
//	tb->FillTriangle(p1,p2,p3)
}

void Mover::MouseMoved(BView *tb,BPoint point)
{
	if (frame.Contains(point))
	{
		if (mover_menu_layout == B_ITEMS_IN_ROW)
			tb->ScrollBy(mover_direction,0);
		else
			tb->ScrollBy(0,mover_direction);
		//*nen Timer setzen, der gestartet wird		
	}
	else 
	{
		//*timer beenden
	}
}

