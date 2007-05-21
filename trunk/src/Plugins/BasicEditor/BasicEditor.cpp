#include <support/List.h>
#include <interface/Font.h>
#include <interface/GraphicsDefs.h>
#include <string.h>
#include "BasicEditor.h"
#include "PCommandManager.h"
#include "Renderer.h"


BasicEditor::BasicEditor():PEditor(),BView(BRect(0,0,200,200),"BasicEditor",B_FOLLOW_ALL_SIDES,B_WILL_DRAW |B_NAVIGABLE|B_NAVIGABLE_JUMP| B_DRAW_ON_CHILDREN)
{
	Init();
	BView::SetDoubleBuffering(1);
	SetDrawingMode(B_OP_ALPHA);
}

void BasicEditor::Init(void)
{
	printRect		= NULL;	
	selectRect		= NULL;
	startMouseDown	= NULL;
	rendersensitv	= new BRegion;
	renderString	= new char[30];
	key_hold		= false;
	connecting		= false;
	fromPoint		= new BPoint(0,0);
	toPoint			= new BPoint(0,0);
	configuration	= new BMessage();

	font_family		family;
	font_style		style;
	


	BMessage		*dataMessage	= new BMessage();
	dataMessage->AddString("Name","Untitled");
	//preparing the standart ObjectMessage
	nodeMessage	= new BMessage(P_C_CLASS_TYPE);
	nodeMessage->AddMessage("Data",dataMessage);
	//Preparing the standart FontMessage
	fontMessage		= new BMessage(B_FONT_TYPE);
	fontMessage->AddInt8("Encoding",be_plain_font->Encoding());
	fontMessage->AddInt16("Face",be_plain_font->Face());
	be_plain_font->GetFamilyAndStyle(&family,&style);
	fontMessage->AddString("Family",(const char*)&family);
	fontMessage->AddInt32("Flags", be_plain_font->Flags());
	fontMessage->AddFloat("Rotation",be_plain_font->Rotation());
	fontMessage->AddFloat("Shear",be_plain_font->Shear());
	fontMessage->AddFloat("Size",be_plain_font->Size());
	fontMessage->AddInt8("Spacing",be_plain_font->Spacing());
	fontMessage->AddString("Style",(const char*)&style);
	rgb_color	fontColor			= {111, 151, 181, 255};
	fontMessage->AddRGBColor("Color",fontColor);
	
	//perparing Pattern Message
	patternMessage	=new BMessage();
	//standart Color 
	rgb_color	fillColor			= {152, 180, 190, 255};
	patternMessage->AddRGBColor("FillColor",fillColor);
	rgb_color	borderColor			= {0, 0, 0, 255};
	patternMessage->AddRGBColor("BorderColor",borderColor);
	patternMessage->AddFloat("PenSize",1.0);
	patternMessage->AddInt8("DrawingMode",B_OP_ALPHA);
	patternMessage->AddFloat("Scale",1.0);
	rgb_color	highColor			= {0, 0, 0, 255};
	patternMessage->AddRGBColor("HighColor",highColor);
	rgb_color 	lowColor			= {128, 128, 128, 255};
	patternMessage->AddRGBColor("LowColor",lowColor);
	patternMessage->AddData("Pattern",B_PATTERN_TYPE,(const void *)&B_SOLID_HIGH,sizeof(B_SOLID_HIGH));
}

void BasicEditor::AttachedToManager(void)
{
	sentTo			= new BMessenger(doc);
	id				=	manager->IndexOf(this);
	sprintf(renderString,"BasicEditor%ld::Renderer",id);

	renderPlugins	= pluginManager->GetPluginsByType(B_E_RENDERER);
	//put this in a seperate function??
	BList		*allNodes		= doc->GetAllNodes();
	BList		*allConnections	= doc->GetAllConnections();

	BMessage	*node			= NULL;
	BView		*renderer		= NULL;
	for (int32 i=0;i<allNodes->CountItems();i++)
	{
		node = (BMessage *)allNodes->ItemAt(i);
		if (node->FindPointer(renderString,(void **)&renderer) != B_OK)
		{
			InsertRenderObject(node);
		}
	}	
	for (int32 i=0;i<allConnections->CountItems();i++)
	{
		node = (BMessage *)allConnections->ItemAt(i);
		if (node->FindPointer(renderString,(void **)&renderer) != B_OK)
		{
			InsertRenderObject(node);
		}
	}	
	nodeMessage->AddPointer("doc",doc);
}

void BasicEditor::DetachedFromManager(void)
{

}

void BasicEditor::PreprocessBeforSave(BMessage *container)
{
	TRACE();
	char	*name;
	uint32	type;
	int32	count;
	int32	i		= 0;
	//remove all the Pointer to the Renderer so that on the next load a new Renderer are added
	while (container->GetInfo(B_POINTER_TYPE,i ,(const char **)&name, &type, &count) == B_OK)
	{
		if ((strstr(name,"BasicEditor") != NULL) ||
			(strcasecmp(name,"Outgoing") == B_OK) ||
			(strcasecmp(name,"Incoming") == B_OK) ||
			(strcasecmp(name,"doc") == B_OK) )
		{
			container->RemoveName(name);
			i--;
		}
		i++;
	}
}


void BasicEditor::PreprocessAfterLoad(BMessage *container)
{
	//**nothing to do jet as i know
	container=container;
}

BList* BasicEditor::GetPCommandList(void)
{
	//at the Moment we dont support special Commands :-)
	return NULL;
}


void BasicEditor::ValueChanged()
{
	BList		*changedNodes	= doc->GetChangedNodes();
//	BList		*allTrashed		= doc->GetTrash();

	BMessage	*node			= NULL;
	BView		*renderer		= NULL;
	for (int32 i=0;i<changedNodes->CountItems();i++)
	{
		node = (BMessage *)changedNodes->ItemAt(i);
		if (node->FindPointer(renderString,(void **)&renderer) == B_OK)
		{
			if (renderer) 
				renderer->MessageReceived(new BMessage(P_C_VALUE_CHANGED));	
		}
		else
		{
			InsertRenderObject(node);
		}
	}
/*	for (int32 i=0;i<allTrashed->CountItems();i++)
	{
		node = (BMessage *)allTrashed->ItemAt(i);
		DeleteRenderObject(node);
	}*/
/*	BMessage	*node		= NULL;
	int32		i			= 0;

	while (wich->FindPointer("node",i,(void **)&node) == B_OK)	
	{
		if ( (node->FindPointer("BasicEditor::Renderer",(void **)&renderer) == B_OK) &&  (renderer != NULL))
			renderer->ValueChanged();
		i++;
	}*/
}

void BasicEditor::SetDirty(BRegion *region)
{
	BView::Invalidate(region);
}

bool BasicEditor::IsFocus(void) const
{
	return	 BView::IsFocus();

}

void BasicEditor::MakeFocus(bool focus = true)
{
	BView::MakeFocus(focus);
}


void BasicEditor::Draw(BRect updateRect)
{
	BView::Draw(updateRect);
}

void BasicEditor::DrawAfterChildren(BRect updateRect)
{
	BView::DrawAfterChildren(updateRect);
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
		StrokeLine(*fromPoint,*toPoint);
	}
}


void BasicEditor::MouseDown(BPoint where)
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
			//EventMaske setzen so dass die Maus auch über den View verfolgt wird
			SetMouseEventMask(B_POINTER_EVENTS, B_NO_POINTER_HISTORY | B_SUSPEND_VIEW_FOCUS | B_LOCK_WINDOW_FOCUS);
		}
	}
}


void BasicEditor::MouseMoved(	BPoint where, uint32 code, const BMessage *a_message)
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

void BasicEditor::MouseUp(BPoint where)
{
	if (startMouseDown != NULL)
	{
		if (selectRect != NULL)
		{
			BMessage *selectMessage=new BMessage(P_C_EXECUTE_COMMAND);
			selectMessage->AddRect("frame",*selectRect);
			selectMessage->AddString("Command::Name","Select");
			sentTo->SendMessage(selectMessage);
		}
		else
		{
		/*	BMessage *insertMessage=new BMessage();
			insertMessage->AddMessage("newObject",nodeMessage);
			insertMessage->AddPoint("where",where);
			sentTo->SendMessage(insertMessage);*/
			BMessage	*currentMsg	= Window()->CurrentMessage();
			uint32		buttons		= 0;
			uint32		modifiers	= 0;
			currentMsg->FindInt32("buttons", (int32 *)&buttons);
			currentMsg->FindInt32("modifiers", (int32 *)&modifiers);
			if ((modifiers & B_CONTROL_KEY) != 0)
				InsertObject(where,false);
			else
				InsertObject(where,true);
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

void BasicEditor::KeyDown(const char *bytes, int32 numBytes)
{

}

void BasicEditor::KeyUp(const char *bytes, int32 numBytes)
{
}


void BasicEditor::MessageReceived(BMessage *message)
{
	switch(message->what) 
	{
		case P_C_VALUE_CHANGED:
		{
			ValueChanged();
			break;
		}
		case P_C_DOC_BOUNDS_CHANGED:
		{
			BRect	docRect = doc->Bounds();
			if (BView::LockLooper())
			{
				ResizeTo(docRect.right,docRect.bottom);
				BView::UnlockLooper();
			}
			break;
		}
		case B_E_CONNECTING:
		{
				connecting = true;
				message->FindPoint("To",toPoint);
				message->FindPoint("From",fromPoint);
				Invalidate();
			break;
		}
		case B_E_CONNECTED:
		{
			connecting = false;
			Invalidate();
			BMessage	*connection			= new BMessage(P_C_CONNECTION_TYPE);
			BMessage	*commandMessage		= new BMessage(P_C_EXECUTE_COMMAND);
			BMessage	*subCommandMessage	= new BMessage(P_C_EXECUTE_COMMAND);

			BMessage	*from		= NULL;
			BMessage	*to			= NULL;
			BMessage	*data		= new BMessage();
			BPoint		*toPointer	= new BPoint(-10,-10);
			message->FindPointer("From",(void **)&from);
			message->FindPoint("To",toPointer);
			data->AddString("Name","Unbenannt");
			to = doc->FindObject(toPointer);
			if (to != NULL && from!=NULL)
			{
				connection->AddPointer("From",from);
				connection->AddPointer("To",to);
				connection->AddMessage("Data",data);
				connection->AddPointer("doc",doc);
				//** add the connections to the Nodes :-)
				commandMessage->AddPointer("node",connection);
				commandMessage->AddString("Command::Name","Insert");
				subCommandMessage->AddString("Command::Name","Select");
				subCommandMessage->AddPointer("node",connection);
				commandMessage->AddMessage("PCommand::subPCommand",subCommandMessage);
				sentTo->SendMessage(commandMessage);
			}
		}
		default:
			BView::MessageReceived(message);
			break;
	}
}

void BasicEditor::InsertObject(BPoint where,bool deselect)
{
	BMessage	*commandMessage	= new BMessage(P_C_EXECUTE_COMMAND);
	BMessage	*newObject		= new BMessage(*nodeMessage);
	BMessage	*newFont		= new BMessage(*fontMessage);
	BMessage	*newPattern		= new BMessage(*patternMessage);
	BMessage	*selectMessage	= new BMessage();
	newObject->AddRect("Frame",BRect(where,where+BPoint(100,80)));
	newObject->AddMessage("Font",newFont);
	newObject->AddMessage("Pattern",newPattern);
	//preparing CommandMessage
	commandMessage->AddPointer("node",(void *)newObject);
	commandMessage->AddString("Command::Name","Insert");
	selectMessage->AddBool("deselect",deselect);
	selectMessage->AddPointer("node",(void *)newObject);
	selectMessage->AddString("Command::Name","Select");
	commandMessage->AddMessage("PCommand::subPCommand",selectMessage);
	sentTo->SendMessage(commandMessage);
}

void BasicEditor::InsertRenderObject(BMessage *node)
{
	BasePlugin	*theRenderer	= (BasePlugin*)renderPlugins->ItemAt(0);
	BView		*addView		= (BView *)theRenderer->GetNewObject(node);
	if (addView!=NULL)
	{
		AddChild(addView);
		node->AddPointer(renderString,addView);
	}
}
void BasicEditor::DeleteRenderObject(BMessage *node)
{
	BView	*newRenderer	= NULL;
	bool	 success		= true;
	if ((node->FindPointer(renderString,(void **)&newRenderer) == B_OK) && (newRenderer != NULL) )
	{
		node->RemoveName(renderString);
		if (LockLooper())
		{
			success=RemoveChild(newRenderer);
			UnlockLooper();
		}
	}
}

