#include <support/ClassInfo.h>
#include <string.h>

#include "ProjectConceptorDefs.h"
#include "PDocument.h"

#include "MessageListView.h"
#include "StringItem.h"
#include "RectItem.h"
#include "FloatItem.h"
#include "BoolItem.h"
#include "NodeItem.h"

MessageListView::MessageListView(PDocument *document,BRect rect, BMessage * forContainer):BOutlineListView(rect,"MessageListView")
{
	doc				= document;
	container		= forContainer;
	baseEditMessage	=  new BMessage(P_C_EXECUTE_COMMAND);
	baseEditMessage->AddPointer("node",container);
	baseEditMessage->AddString("Command::Name","ChangeValue");
	baseEditMessage->AddMessage("valueContainer",new BMessage());
	editMessage		= new BMessage(*baseEditMessage);
}


void MessageListView::MouseDown(BPoint point)
{
	BOutlineListView::MouseDown(point);
}

void MessageListView::AttachedToWindow(void)
{
	ValueChanged();
}

void MessageListView::ValueChanged()
{
	MakeEmpty();
	if (container != NULL)
		AddMessage(container,NULL);
}

void MessageListView::SetContainer(BMessage *message)
{
	container = message;
	baseEditMessage->ReplacePointer("node",container);
	delete editMessage;
	editMessage		= new BMessage(*baseEditMessage);
	ValueChanged();
}

void MessageListView::AddMessage(BMessage *message,BListItem* superItem)
{
	char		*name; 
	uint32		type; 
	int32		count;
	#ifdef B_ZETA_VERSION_1_0_0
	for (int32 i = 0; message->GetInfo(B_ANY_TYPE, i,(const char **) &name, &type, &count) == B_OK; i++)
	#else
	for (int32 i = 0; message->GetInfo(B_ANY_TYPE, i,(char **) &name, &type, &count) == B_OK; i++)
	#endif
	{
		switch(type)
		{
			case B_MESSAGE_TYPE:
			{
				BMessage	*tmpMessage		= new BMessage();
				BMessage	*valueContainer	= new BMessage();
				if (message->FindMessage(name,count-1,tmpMessage)==B_OK)
				{
					BListItem	*newSuperItem=new BStringItem(name);
					if (superItem)
						AddUnder(newSuperItem,superItem);
					else
					{
						AddItem(newSuperItem);
						delete editMessage;
						editMessage		= new BMessage(*baseEditMessage);
					}
					editMessage->FindMessage("valueContainer",valueContainer);
					valueContainer->AddString("subgroup",name);
					editMessage->ReplaceMessage("valueContainer",valueContainer);
					AddMessage(tmpMessage,newSuperItem);
				}
				break;
			}
			case B_STRING_TYPE:
			{
				char		*string;
				message->FindString(name,count-1,(const char **)&string);
				StringItem *stringItem = new StringItem(name,string);
				if (superItem)
				{
					AddUnder(stringItem,superItem);
				}
				else
				{
					AddItem(stringItem);
					delete editMessage;
					editMessage		= new BMessage(*baseEditMessage);
				}
				BMessage *tmpMessage = new BMessage(*editMessage);
				stringItem->SetMessage(tmpMessage);
				stringItem->SetTarget(doc);
				break;
			}
			case B_RECT_TYPE:
			{
				BRect	rect;
				message->FindRect(name,count-1,&rect);
				RectItem	*rectItem	= new RectItem(name,rect);
				if (superItem)
					AddUnder(rectItem,superItem);
				else
				{
					AddItem(rectItem);
					delete editMessage;
					editMessage		= new BMessage(*baseEditMessage);
				}
				BMessage *tmpMessage = new BMessage(*editMessage);
				rectItem->SetMessage(tmpMessage);
				rectItem->SetTarget(doc);
				break;
			}
			case B_FLOAT_TYPE:
			{
				float	value;
				message->FindFloat(name,count-1,&value);
				FloatItem *floatItem	= new FloatItem(name,value);
				if (superItem)
					AddUnder(floatItem,superItem);
				else
				{
					AddItem(floatItem);
					delete editMessage;
					editMessage		= new BMessage(*baseEditMessage);
				}
				BMessage *tmpMessage = new BMessage(*editMessage);
				floatItem->SetMessage(tmpMessage);
				floatItem->SetTarget(doc);
				break;
			}
			case B_BOOL_TYPE:
			{
				bool	value;
				message->FindBool(name,count-1,&value);
				BoolItem	*boolItem	= new BoolItem(name,value);
				if (superItem)
					AddUnder(boolItem,superItem);
				else
				{
					AddItem(boolItem);
					delete editMessage;
					editMessage		= new BMessage(*baseEditMessage);
				}
				BMessage *tmpMessage = new BMessage(*editMessage);
				boolItem->SetMessage(tmpMessage);
				boolItem->SetTarget(doc);
				break;
			}
			case B_POINTER_TYPE:
			{
				if (strcmp(name,"Node::outgoing") == B_OK)
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
						connection->FindPointer("Node::to",(void **)&toNode);
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

void MessageListView::MessageReceived(BMessage *message)
{
	TRACE();
	BaseListItem	*item;
	switch(message->what) 
	{
		case ITEM_CHANDED:
		{
			if ( (message->FindPointer("item",(void **)&item)==B_OK) && (item != NULL) )
				item->Invoke();
			break;
		}
		default:
			BOutlineListView::MessageReceived(message);
			break;
	}
}
