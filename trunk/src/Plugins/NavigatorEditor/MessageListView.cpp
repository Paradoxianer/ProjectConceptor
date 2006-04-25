#include <support/ClassInfo.h>
#include <string.h>

#include "MessageListView.h"
#include "StringItem.h"
#include "RectItem.h"
#include "FloatItem.h"
#include "BoolItem.h"
#include "NodeItem.h"

MessageListView::MessageListView(BRect rect, BMessage * forContainer):BOutlineListView(rect,"MessageListView")
{
	container=forContainer;
	ValueChanged();
}


void MessageListView::MouseDown(BPoint point)
{
	BOutlineListView::MouseDown(point);
}

void MessageListView::ValueChanged()
{
	MakeEmpty();
	AddMessage(container,NULL);
}

void MessageListView::AddMessage(BMessage *message,BListItem* superItem)
{
	char		*name; 
	uint32		type; 
	int32		count; 
	for (int32 i = 0; message->GetInfo(B_ANY_TYPE, i,(const char **) &name, &type, &count) == B_OK; i++)
	{
		switch(type)
		{
			case B_MESSAGE_TYPE:
			{
				BMessage	*tmpMessage		= new BMessage();
				if (message->FindMessage(name,count-1,tmpMessage)==B_OK)
				{
					BListItem	*newSuperItem=new BStringItem(name);
					if (superItem)
						AddUnder(newSuperItem,superItem);
					else
						AddItem(newSuperItem);
					AddMessage(tmpMessage,newSuperItem);
				}
				break;
			}
			case B_STRING_TYPE:
			{
				char		*string;
				message->FindString(name,count-1,(const char **)&string);
				if (superItem)
					AddUnder(new StringItem(name,string),superItem);
				else
					AddItem(new StringItem(name,string));
				break;
			}
			case B_RECT_TYPE:
			{
				BRect	rect;
				message->FindRect(name,count-1,&rect);
				if (superItem)
					AddUnder(new RectItem(name,rect),superItem);
				else
					AddItem(new RectItem(name,rect));
				break;
			}
			case B_FLOAT_TYPE:
			{
				float	value;
				message->FindFloat(name,count-1,&value);
				if (superItem)
					AddUnder(new FloatItem(name,value),superItem);
				else
					AddItem(new FloatItem(name,value));
				break;
			}
			case B_BOOL_TYPE:
			{
				bool	value;
				message->FindBool(name,count-1,&value);
				if (superItem)
					AddUnder(new BoolItem(name,value),superItem);
				else
					AddItem(new BoolItem(name,value));
				break;
			}
			case B_POINTER_TYPE:
			{
				if (strcmp(name,"Outgoing") == B_OK)
				{
					BList		*list		= NULL;
					BStringItem	*masterItem	= new BStringItem(name);
					BMessage	*connection	= NULL;
					BMessage	*toNode		= NULL;
					if (superItem)
						AddUnder(masterItem,superItem);
					else
						AddItem(masterItem);
					message->FindPointer(name,count-1,(void **)&list);
					
					for (int32 i=0;i<list->CountItems();i++)
					{	
						connection	= (BMessage*)list->ItemAt(i);
						connection->FindPointer("To",(void **)&toNode);
						AddUnder(new NodeItem(toNode),masterItem);
					}
				}
				

/*				message->FindPointer(name,count,(void **)&pointer);
				char	*className = (char *)class_name(pointer);
				if (superItem)
					AddUnder(new StringItem(name,className),superItem);
				else
					AddItem(new StringItem(name,className));

				break;*/
			}
		}

	} 

}
