/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <interface/Window.h>

#include "BasicGroupView.h"



BasicGroupView::BasicGroupView(BMessage *message):BBox(BRect(0,0,100,100),"GroupView",B_FOLLOW_LEFT|B_FOLLOW_TOP,B_WILL_DRAW)
{
}

void BasicGroupView::Init()
{
}

void BasicGroupView::AttachedToWindow()
{
}

void BasicGroupView::Draw(BRect updateRect)
{
	BView::Draw(updateRect);
}

void BasicGroupView::DrawAfterChildren(BRect updateRect)
{
	if (selectRect)
	{
		SetHighColor(81,131,171,120);
		FillRect(*selectRect);
		SetHighColor(31,81,121,255);
		StrokeRect(*selectRect);
	}
	else if (connecting)
	{
		SetHighColor(50,50,50,255);
		StrokeLine(fromRect->RightBottom(),*toPoint);
	}
}


void BasicGroupView::MouseDown(BPoint where)
{
	BMessage *currentMsg = Window()->CurrentMessage();
	if (currentMsg->what == B_MOUSE_DOWN)
	{
		uint32 buttons = 0;
		currentMsg->FindInt32("buttons", (int32 *)&buttons);
		uint32 modifiers = 0;
		currentMsg->FindInt32("modifiers", (int32 *)&modifiers);
		if (buttons & B_PRIMARY_MOUSE_BUTTON)
	 	{
			startMouseDown=new BPoint(where);
			//EventMaske setzen so dass die Maus auch über den View verfolt wird
			SetMouseEventMask(B_POINTER_EVENTS, B_NO_POINTER_HISTORY | B_SUSPEND_VIEW_FOCUS | B_LOCK_WINDOW_FOCUS);
		}
	}
}

void BasicGroupView::MouseUp(BPoint where)
{
	if (startMouseDown != NULL)
	{
		if (selectRect != NULL)
		{
			BMessage *selectMessage=new BMessage();
			selectMessage->AddRect("selectRect",*selectRect);
//			sentTo->SendMessage(selectMessage);
		}
		else
		{
			BMessage *insertMessage=new BMessage();
			insertMessage->AddMessage("newObject",objectMessage);
			insertMessage->AddPoint("where",where);
//			sentTo->SendMessage(insertMessage);
			//InsertObject(insertMessage);
		}
		delete startMouseDown;
		startMouseDown=NULL;
		delete selectRect;
		selectRect=NULL;
		Invalidate();
	}
/*	else
	{
		BMessage *selectMessage=new BMessage(B_SELECT_ALL);
		selectMessage->AddBool("clearSelection",true);
		sentTo->SendMessage(selectMessage);
	}*/

}

void BasicGroupView::MouseMoved(	BPoint where, uint32 code, const BMessage *a_message)
{
	if (startMouseDown)
	{
		//only if the user hase moved the Mouse we start to select...
		float dx=where.x - startMouseDown->x;
		float dy=where.y - startMouseDown->y;
		float entfernung=sqrt(dx*dx+dy*dy);
		if (entfernung>max_entfernung)
		{
			if (selectRect!=NULL)
			{
				selectRect->SetLeftTop(*startMouseDown);
				selectRect->SetRightBottom(where);
			}
			else
			{
				selectRect=new BRect(*startMouseDown,where);
			}
			//make the Rect valid
			if (selectRect->left>selectRect->right)
			{
				float c=selectRect->left;
				selectRect->left=selectRect->right;
				selectRect->right=c;
			}
			if (selectRect->top>selectRect->bottom)
			{
				float c=selectRect->top;
				selectRect->top=selectRect->bottom;
				selectRect->bottom=c;
			}
			Invalidate();
		}
	}
}

void BasicGroupView::MessageReceived(BMessage *msg)
{
}

void BasicGroupView::FrameMoved(BPoint new_position)
{
}

void BasicGroupView::FrameResized(float new_width, float new_height)
{
}

