#include "TextEditorControl.h"

TextEditorControl::TextEditorControl(BRect rect,const  char *name, BMessage *message, uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP, uint32 flags = B_WILL_DRAW | B_PULSE_NEEDED| B_NAVIGABLE):  BTextView(rect,name,BRect(0,0,rect.Width(),rect.Height()),resizingMode,flags), BInvoker(message,NULL)
{
	TRACE();
	singleLine	= true;
	changed		= false;
	commit		= true;
}

void TextEditorControl::AttachedToWindow(void)
{
	TRACE();
	if (Target() == NULL)
	{
		SetTarget(Parent());
	}
	SetViewColor(B_TRANSPARENT_COLOR);
	SetEventMask(B_POINTER_EVENTS,B_LOCK_WINDOW_FOCUS | B_NO_POINTER_HISTORY);
	SelectAll();
}

void TextEditorControl::DetachedFromWindow()
{
	TRACE();
}

void TextEditorControl::MakeFocus(bool flag = true)
{
	TRACE();
	PRINT(("%s - MakeFocus(%d)\n",Text(),flag));
	BTextView::MakeFocus(flag);
	/*if (flag == false) 
	{
		Invoke();
	}*/
}
void TextEditorControl::MouseDown(BPoint point)
{
	TRACE();
	BTextView::MouseDown(point);
	if (!Bounds().Contains(point))
	{
		Invoke();
	}
}
void TextEditorControl::MouseUp(BPoint point)
{
	TRACE();
	BTextView::MouseUp(point);
	if (!Bounds().Contains(point))
	{
		Invoke();
	}
}

void TextEditorControl::KeyDown(const char *bytes, int32 numBytes)
{
	if ((singleLine)&&(bytes[0] == B_ENTER))
	{
		Invoke();
	}
	else if (bytes[0] == B_ESCAPE)
	{
		commit=false;
//		Parent()->MakeFocus(true);
//		Parent()->RemoveChild(this);
	}
	else
	{
			changed		= true;
			BTextView::KeyDown(bytes,numBytes);
	}
//		BTextView::KeyDown(bytes,numBytes);
}
status_t TextEditorControl::Invoke(BMessage *message = NULL)
{
	TRACE();
	BMessage copy(*Message()); 
	copy.AddPointer("newValue", Text()); 
	copy.AddInt32("size",TextLength()+1);
	if (Parent())
	{
		Parent()->MakeFocus(true);
		Parent()->RemoveChild(this);
	}
	if ((changed) && (commit))
		BInvoker::Invoke(&copy);
}