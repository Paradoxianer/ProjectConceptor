#include <interface/CheckBox.h>
#include <interface/TextControl.h>
#include <support/String.h>

#include "MessageView.h"
#include "ColorItem.h"
#include "FontItem.h"
#include "PatternItem.h"
#include "RectItem.h"

#include "ProjectConceptorDefs.h"

MessageView::MessageView(BRect rect,BMessage *forMessage,uint32 resizingMode, uint32 flags):BBox(rect,NULL,resizingMode,flags)
{
	TRACE();
	configMessage=forMessage;
	Init();
}

void MessageView::Init()
{
	seperator = Bounds().Width()/3;
}


void MessageView::ChangeLanguage()
{
	TRACE();
}

void MessageView::SetConfigMessage(BMessage *configureMessage)
{
	TRACE();
	configMessage=configureMessage;
	ValueChanged();
}

void MessageView::ValueChanged(void)
{
	char		*name; 
	uint32		type; 
	int32		count;
	#ifdef B_ZETA_VERSION_1_0_0
	for (int32 i = 0; configMessage->GetInfo(B_ANY_TYPE, i,(const char **) &name, &type, &count) == B_OK; i++)
	#else
	for (int32 i = 0; configMessage->GetInfo(B_ANY_TYPE, i,(char **) &name, &type, &count) == B_OK; i++)
	#endif
	{
		//calculate the Position where to add the next View
		float top = ItemTop();
		BRect rect = BRect(MARGIN_SPACE,top,Bounds().right-MARGIN_SPACE,top);
		switch(type)
		{
			case B_STRING_TYPE:
			{
				char		*string;
				configMessage->FindString(name,count-1,(const char **)&string);
				BTextControl	*stringItem	= new BTextControl(rect,name,name,string,NULL);
				AddChild(stringItem);
				BMessage	*tmpMessage = new BMessage(B_CONTROL_INVOKED);
				tmpMessage->AddString("name",name);
				tmpMessage->AddInt32("count",count-1);
				tmpMessage->AddInt32("type",type);
				stringItem->SetMessage(tmpMessage);
				break;
			}
			case B_RECT_TYPE:
			{
				BRect	valueRect;
				configMessage->FindRect(name,count-1,&valueRect);
				RectItem	*rectItem	= new RectItem(rect,name,valueRect);
				AddChild(rectItem);
				BMessage	*tmpMessage = new BMessage(B_CONTROL_INVOKED);
				tmpMessage->AddString("name",name);
				tmpMessage->AddInt32("count",count-1);
				tmpMessage->AddInt32("type",type);
				rectItem->SetMessage(tmpMessage);
				break;
			}
			case B_FLOAT_TYPE:
			{
				float	value;
				configMessage->FindFloat(name,count-1,&value);
				BString	floatString;
				floatString<<value;
				BTextControl	*stringItem	= new BTextControl(rect,name,name,floatString.String(),NULL);
				AddChild(stringItem);
				BMessage	*tmpMessage = new BMessage(B_CONTROL_INVOKED);
				tmpMessage->AddString("name",name);
				tmpMessage->AddInt32("count",count-1);
				tmpMessage->AddInt32("type",type);
				stringItem->SetMessage(tmpMessage);
				break;
			}
			case B_INT8_TYPE:
			case B_INT16_TYPE:
			case B_INT32_TYPE:
			{
				int32	value;
				configMessage->FindInt32(name,count-1,&value);
				BString	intString;
				intString<<value;
				BTextControl	*stringItem	= new BTextControl(rect,name,name,intString.String(),NULL);
				AddChild(stringItem);
				BMessage	*tmpMessage = new BMessage(B_CONTROL_INVOKED);
				tmpMessage->AddString("name",name);
				tmpMessage->AddInt32("count",count-1);
				tmpMessage->AddInt32("type",type);
				stringItem->SetMessage(tmpMessage);
				break;
			}
			case B_BOOL_TYPE:
			{
				bool	value;
				configMessage->FindBool(name,count-1,&value);
				BCheckBox	*boolItem	= new BCheckBox(rect,name,name,NULL);
				AddChild(boolItem);
				boolItem->SetValue(value);
				BMessage	*tmpMessage = new BMessage(B_CONTROL_INVOKED);
				tmpMessage->AddString("name",name);
				tmpMessage->AddInt32("count",count-1);
				tmpMessage->AddInt32("type",type);
				boolItem->SetMessage(tmpMessage);
				break;
			}
		}

	} 
}

float MessageView::ItemTop()
{
	int32 count	= CountChildren();
	if (count > 0)
	{
		BView *tmpChild = ChildAt(count-1);
		return tmpChild->Frame().bottom+MARGIN_SPACE;
		
	}
	else
		return MARGIN_SPACE;	
}

