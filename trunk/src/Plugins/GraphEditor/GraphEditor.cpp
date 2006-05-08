#include <support/List.h>
#include <interface/Font.h>
#include <interface/ScrollView.h>
#include <interface/GraphicsDefs.h>
#include <translation/TranslationUtils.h>
#include <translation/TranslatorFormats.h>

#include <string.h>
#include "GraphEditor.h"
#include "PCommandManager.h"
#include "Renderer.h"
#include "ClassRenderer.h"
#include "ConnectionRenderer.h"

#include "ToolBar.h"

GraphEditor::GraphEditor():PEditor(),BView(BRect(0,0,200,200),"GraphEditor",B_FOLLOW_ALL_SIDES,B_WILL_DRAW |B_NAVIGABLE|B_NAVIGABLE_JUMP|B_FRAME_EVENTS)
{
	TRACE();
	Init();
	BView::SetDoubleBuffering(1);
	SetDrawingMode(B_OP_ALPHA);
}

void GraphEditor::Init(void)
{
	TRACE();
/*	insertCommand	= NULL;
	selectCommand	= NULL;
	deleteCommand	= NULL;
	connectCommand	= NULL;*/
	printRect		= NULL;	
	selectRect		= NULL;
	startMouseDown	= NULL;
	activRenderer	= NULL;
	mouseReciver	= NULL;
	rendersensitv	= new BRegion();
	renderString	= new char[30];
	key_hold		= false;
	connecting		= false;
	gridEnabled		= false;
	fromPoint		= new BPoint(0,0);
	toPoint			= new BPoint(0,0);
	renderer		= new BList();
	scale			= 1.0;
	configMessage	= new BMessage();
	
	
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
	
	scaleMenu		= new BMenu(_T("Scale"));
	BMessage	*newScale	= new BMessage(B_E_NEW_SCALE);
	newScale->AddFloat("scale",0.1);
	scaleMenu->AddItem(new BMenuItem("10 %",newScale,0,0));
	newScale	= new BMessage(B_E_NEW_SCALE);
	newScale->AddFloat("scale",0.25);
	scaleMenu->AddItem(new BMenuItem("25 %",newScale,0,0));
	newScale	= new BMessage(B_E_NEW_SCALE);
	newScale->AddFloat("scale",0.33);
	scaleMenu->AddItem(new BMenuItem("33 %",newScale,0,0));
	newScale	= new BMessage(B_E_NEW_SCALE);
	newScale->AddFloat("scale",0.5);
	scaleMenu->AddItem(new BMenuItem("50 %",newScale,0,0));
	newScale	= new BMessage(B_E_NEW_SCALE);
	newScale->AddFloat("scale",0.75);
	scaleMenu->AddItem(new BMenuItem("75 %",newScale,0,0));
	newScale	= new BMessage(B_E_NEW_SCALE);
	newScale->AddFloat("scale",1.00);
	scaleMenu->AddItem(new BMenuItem("100 %",newScale,0,0));
	newScale	= new BMessage(B_E_NEW_SCALE);
	newScale->AddFloat("scale",1.5);
	scaleMenu->AddItem(new BMenuItem("150 %",newScale,0,0));
	newScale	= new BMessage(B_E_NEW_SCALE);
	newScale->AddFloat("scale",2);
	scaleMenu->AddItem(new BMenuItem("200 %",newScale,0,0));
	newScale	= new BMessage(B_E_NEW_SCALE);
	newScale->AddFloat("scale",5);
	scaleMenu->AddItem(new BMenuItem("500 %",newScale,0,0));
	newScale	= new BMessage(B_E_NEW_SCALE);
	newScale->AddFloat("scale",10);
	scaleMenu->AddItem(new BMenuItem("1000 %",newScale,0,0));
	
	grid		= new ToolItem("Grid",BTranslationUtils::GetBitmap(B_PNG_FORMAT,"grid"),new BMessage(B_E_GRID_CHANGED),P_M_TWO_STATE_ITEM);
	penSize		= new FloatToolItem(_T("Pen Size"),1.0,new BMessage(B_E_PEN_SIZE_CHANGED));
	colorItem	= new ColorToolItem(_T("Fill"),fillColor,new BMessage(B_E_COLOR_CHANGED));
	patternItem	= new PatternToolItem(_T("Pattern"),B_SOLID_HIGH, new BMessage(B_E_PATTERN_CHANGED));
}

void GraphEditor::AttachedToManager(void)
{
	TRACE();
//	PCommandManager *commandManager = doc->GetCommandManager();
	//insertCommand contains off an object insert and an select
/*	insertCommand		= commandManager->GetPCommand("Insert");
	selectCommand		= commandManager->GetPCommand("Select");
	deleteCommand		= commandManager->GetPCommand("Delete");
	connectCommand		= commandManager->GetPCommand("Connect");
	moveCommand			= commandManager->GetPCommand("Move");
	resizeCommand		= commandManager->GetPCommand("Resize");
	changeValueCommand	= commandManager->GetPCommand("ChangeValue");*/
	sentTo				= new BMessenger(doc);
	id					= manager->IndexOf(this);
	sprintf(renderString,"GraphEditor%ld::Renderer",id);
	//put this in a seperate function??
	nodeMessage->AddPointer("doc",doc);
	
	BMessage	*shortcuts	= new BMessage();
	BMessage	*sendMessage;
	shortcuts->AddInt32("key",B_DELETE);
	sendMessage	= new BMessage(P_C_EXECUTE_COMMAND);
	sendMessage->AddString("Command::Name","Delete");
	shortcuts->AddInt32("modifiers",0);
	shortcuts->AddMessage("message",sendMessage);
	shortcuts->AddPointer("handler",doc);
	configMessage->AddMessage("Shortcuts",shortcuts);
	InitAll();
}

void GraphEditor::DetachedFromManager(void)
{
	TRACE();
}

BList* GraphEditor::GetPCommandList(void)
{
	TRACE();
	//at the Moment we dont support special Commands :-)
	return NULL;
}
void GraphEditor::InitAll()
{
	TRACE();
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
}
void GraphEditor::PreprocessBeforSave(BMessage *container)
{
	TRACE();
	PRINT(("GraphEditor::PreprocessBeforSave:\n"));
	char	*name; 
	uint32	type; 
	int32	count; 
	int32	i		= 0;
	//remove all the Pointer to the Renderer so that on the next load a new Renderer are added
	while (container->GetInfo(B_POINTER_TYPE,i ,(const char **)&name, &type, &count) == B_OK)
	{
		if ((strstr(name,"GraphEditor") != NULL) || 
			(strcasecmp(name,"Outgoing") == B_OK) ||
			(strcasecmp(name,"Incoming") == B_OK) ||
			(strcasecmp(name,"doc") == B_OK) )
		{
			container->RemoveName(name);
			i--;
		}
		i++;
	}
	PRINT_OBJECT(*container);
}


void GraphEditor::ValueChanged()
{
	TRACE();
	BList		*changedNodes	= doc->GetChangedNodes();
	BList		*allTrashed		= doc->GetTrash();

	BMessage	*node			= NULL;
	Renderer	*painter		= NULL;
	BRect		frame;
	BRect		invalid;
	for (int32 i=0;i<changedNodes->CountItems();i++)
	{
		node = (BMessage *)changedNodes->ItemAt(i);
		if (node->FindPointer(renderString,(void **)&painter) == B_OK)
			painter->ValueChanged();
		else
			InsertRenderObject(node);
	}
	for (int32 i=0;i<allTrashed->CountItems();i++)
	{
		node = (BMessage *)allTrashed->ItemAt(i);
//		DeleteRenderObject(node);
	}
	if (BView::LockLooper())
	{
//		invalid.InsetBy(-20,-20);
//		Invalidate(invalid);
		Invalidate();
		BView::UnlockLooper();
	}
/*	BMessage	*node		= NULL;
	int32		i			= 0;

	while (wich->FindPointer("node",i,(void **)&node) == B_OK)	
	{
		if ( (node->FindPointer("GraphEditor::Renderer",(void **)&renderer) == B_OK) &&  (renderer != NULL))
			renderer->ValueChanged();
		i++;
	}*/
}

void GraphEditor::SetDirty(BRegion *region)
{
	TRACE();
	BView::Invalidate(region);
}


void GraphEditor::Draw(BRect updateRect)
{
	SetHighColor(230,230,230,255);
	BView::Draw(BRect(updateRect.left/scale,updateRect.top/scale,updateRect.right/scale,updateRect.bottom/scale));
	if (gridEnabled)
	{
		int32		xcount		= (Frame().Width()/gridWidth)+1;
		int32		ycount		= (Frame().Height()/gridWidth)+1;
		float		x			= 0;
		float		y			= 0;
		rgb_color	gridColor	= tint_color(ViewColor(),1.1);
		BeginLineArray(xcount+ycount);
		for (int32 i=1;i<xcount;i++)
		{
			AddLine(BPoint(x,Bounds().top),BPoint(x,Frame().Height()),gridColor);
			x += gridWidth;
		}
		for (int32 i=1;i<ycount;i++)
		{
			AddLine(BPoint(Bounds().left,y),BPoint(Frame().Width(),y),gridColor);
			y += gridWidth;
		}
		EndLineArray();
	}
	renderer->DoForEach(DrawRenderer,this);
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

void GraphEditor::MouseDown(BPoint where)
{
	BView::MouseDown(where);
	BView::MakeFocus(true);
	BPoint		scaledWhere;
	scaledWhere.x	= where.x / scale;
	scaledWhere.y	= where.y / scale;	

//	BMessage *currentMsg = Window()->CurrentMessage();
	uint32 modifiers = 0;
	uint32 buttons = 0;
	GetMouse(&where,&buttons);
	bool found	=	false;
	for (int32 i=(renderer->CountItems()-1);((!found) && (i>=0) );i--)
	{
		if (((Renderer*)renderer->ItemAt(i))->Caught(scaledWhere))
		{
			mouseReciver = (Renderer*)renderer->ItemAt(i);
			mouseReciver->MouseDown(scaledWhere);
			found			= true;
		}
	}
	if (!found)
	{
//		currentMsg->FindInt32("buttons", (int32 *)&buttons);

//		currentMsg->FindInt32("modifiers", (int32 *)&modifiers);
		if (buttons & B_PRIMARY_MOUSE_BUTTON)
	 	{
			startMouseDown=new BPoint(scaledWhere);
			//EventMaske setzen so dass die Maus auch über den View verfolgt wird
			SetMouseEventMask(B_POINTER_EVENTS, B_NO_POINTER_HISTORY | B_SUSPEND_VIEW_FOCUS | B_LOCK_WINDOW_FOCUS);
		}
	}
}


void GraphEditor::MouseMoved(	BPoint where, uint32 code, const BMessage *a_message)
{
	BView::MouseMoved(where,code,a_message);
	BPoint		scaledWhere;
	scaledWhere.x	= where.x / scale;
	scaledWhere.y	= where.y / scale;	
	if (startMouseDown)
	{
		//only if the user hase moved the Mouse we start to select...
		float dx=scaledWhere.x - startMouseDown->x;
		float dy=scaledWhere.y - startMouseDown->y;
		float entfernung=sqrt(dx*dx+dy*dy);
		if (entfernung>max_entfernung)
		{
			if (selectRect!=NULL)
			{
				selectRect->SetLeftTop(*startMouseDown);
				selectRect->SetRightBottom(scaledWhere);
			}
			else
			{
				selectRect=new BRect(*startMouseDown,scaledWhere);
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
	else if (mouseReciver != NULL)
	{
		mouseReciver->MouseMoved(scaledWhere,code,a_message);
	}
}

void GraphEditor::MouseUp(BPoint where)
{
	BView::MouseUp(where);
	BPoint		scaledWhere;
	scaledWhere.x	= where.x / scale;
	scaledWhere.y	= where.y / scale;	
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
				InsertObject(scaledWhere,false);
			else
				InsertObject(scaledWhere,true);
			//InsertObject(insertMessage);
		}
		delete startMouseDown;
		startMouseDown=NULL;
		delete selectRect;
		selectRect=NULL;
		Invalidate();
	}
	else if (mouseReciver != NULL)
	{
		mouseReciver->MouseUp(scaledWhere);
	}
/*	else
	{
		BMessage *selectMessage=new BMessage(B_SELECT_ALL);
		selectMessage->AddBool("clearSelection",true);
		sentTo->SendMessage(selectMessage);
	}*/

}

void GraphEditor::KeyDown(const char *bytes, int32 numBytes)
{
	TRACE();
}

void GraphEditor::KeyUp(const char *bytes, int32 numBytes)
{
	TRACE();
}
void GraphEditor::AttachedToWindow(void)
{
	TRACE();
	SetViewColor(230,230,230,255);
	PWindow 	*pWindow	= (PWindow *)Window();
	BMenuBar	*menuBar	= (BMenuBar *)pWindow->FindView(P_M_STATUS_BAR);
	menuBar->AddItem(scaleMenu);
	scaleMenu->SetTargetForItems(this);
	if (doc)
		InitAll();
	ToolBar		*toolBar	= (ToolBar *)pWindow->FindView(P_M_STANDART_TOOL_BAR);
	toolBar->AddSeperator();
	toolBar->AddSeperator();
	toolBar->AddSeperator();	
	toolBar->AddSeperator();
	toolBar->AddSeperator();		
	toolBar->AddItem(grid);
	toolBar->AddSeperator();	
	toolBar->AddItem(penSize);
	toolBar->AddItem(colorItem);
	toolBar->AddItem(patternItem);
	grid->SetTarget(this);
	penSize->SetTarget(this);
	colorItem->SetTarget(this);
	patternItem->SetTarget(this);
}



void GraphEditor::DetachedFromWindow(void)
{
	TRACE();
	if (Window())
	{
		PWindow 	*pWindow		= (PWindow *)Window();
		BMenuBar	*menuBar		= (BMenuBar *)pWindow->FindView(P_M_STATUS_BAR);
		if (menuBar)
			menuBar->RemoveItem(scaleMenu);
		ToolBar		*toolBar	= (ToolBar *)pWindow->FindView(P_M_STANDART_TOOL_BAR);
		if (toolBar)
		{
			toolBar->RemoveItem(penSize);
			toolBar->RemoveItem(colorItem);
			toolBar->RemoveItem(patternItem);
			toolBar->RemoveSeperator();
			toolBar->RemoveItem(grid);
			toolBar->RemoveSeperator();
			toolBar->RemoveSeperator();
			toolBar->RemoveSeperator();	
			toolBar->RemoveSeperator();
			toolBar->RemoveSeperator();	
		}
		Renderer	*nodeRenderer	= NULL;
		for (int32 i=0;i<renderer->CountItems();i++)
		{
			nodeRenderer = (Renderer *)renderer->ItemAt(i);
			(nodeRenderer->GetMessage())->PrintToStream();
		}
		while(renderer->CountItems()>0)
		{
			nodeRenderer = (Renderer *)renderer->ItemAt(0);
//			DeleteRenderObject(nodeRenderer->GetMessage());
			RemoveRenderer(nodeRenderer);
		}
	}
}	
void GraphEditor::MessageReceived(BMessage *message)
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
			BRect		docRect		= doc->Bounds();
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
			BMessage	*connection		= new BMessage(P_C_CONNECTION_TYPE);
			BMessage	*commandMessage	= new BMessage(P_C_EXECUTE_COMMAND);
			BMessage	*subCommandMessage	= new BMessage(P_C_EXECUTE_COMMAND);

			BMessage	*from			= NULL;
			BMessage	*to				= NULL;
			BMessage	*data			= new BMessage();
			BPoint		*toPointer		= new BPoint(-10,-10);
			BPoint		*fromPointer	= new BPoint(-10,-10);
			Renderer	*foundRenderer	= NULL;
			if (message->FindPointer("From",(void **)&from) == B_OK)
			{
				message->FindPoint("To",toPointer);
				foundRenderer = FindNodeRenderer(*toPointer);
				if (foundRenderer)
					to = foundRenderer->GetMessage();
				//to = doc->FindObject(toPointer);
			}
			else
			{
				message->FindPointer("To",(void **)&to);
				message->FindPoint("From",fromPointer);
				foundRenderer = FindNodeRenderer(*fromPointer);
				if (foundRenderer)
					from  = foundRenderer->GetMessage();
			//	from	= doc->FindObject(fromPointer);
			}
			data->AddString("Name","Unbenannt");
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
			break;
		}
		case B_E_NEW_SCALE:
		{
			message->FindFloat("scale",&scale);
			SetScale(scale);
			FrameResized(0,0);
			Invalidate();
			break;
		}
		case B_E_GRID_CHANGED:
		{
			gridEnabled =! gridEnabled;
			Invalidate();
			break;
		}
		case B_E_COLOR_CHANGED:
		{
			rgb_color	*newColor=	new rgb_color(colorItem->GetColor());
			rgb_color	tmpNewColor =	{255, 0, 0, 255};
			BMessage	*changeColorMessage	= new BMessage(P_C_EXECUTE_COMMAND);
			changeColorMessage->AddString("Command::Name","ChangeValue");
			changeColorMessage->AddBool("selected",true);
			changeColorMessage->AddString("name","FillColor");
			changeColorMessage->AddString("subgroup","Pattern");
			changeColorMessage->AddInt32("type",B_RGB_COLOR_TYPE);
			changeColorMessage->AddPointer("newValue",newColor); 
			changeColorMessage->AddInt32("size",sizeof(*newColor));
			sentTo->SendMessage(changeColorMessage);
			break;
		}
		case B_E_PEN_SIZE_CHANGED:
		{
			float		*newPenSize				= new float(penSize->GetValue());
			BMessage	*changePenSizeMessage	= new BMessage(P_C_EXECUTE_COMMAND);
			changePenSizeMessage->AddString("Command::Name","ChangeValue");
			changePenSizeMessage->AddBool("selected",true);
			changePenSizeMessage->AddString("name","PenSize");
			changePenSizeMessage->AddString("subgroup","Pattern");
			changePenSizeMessage->AddInt32("type",B_FLOAT_TYPE);
			changePenSizeMessage->AddPointer("newValue",newPenSize); 
			changePenSizeMessage->AddInt32("size",sizeof(*newPenSize));
			sentTo->SendMessage(changePenSizeMessage);
			break;
		}
		default:
			BView::MessageReceived(message);
			break;
	}
}

void GraphEditor::FrameResized(float width, float height)
{
	TRACE();
	BRect		docRect		= doc->Bounds();
	BView::FrameResized(width,height);
	BScrollView	*scrollView = dynamic_cast<BScrollView *> (BView::Parent());
	if (scrollView)
	{
		scrollView->ScrollBar(B_HORIZONTAL)->SetRange(0,docRect.Width()*scale);
		scrollView->ScrollBar(B_HORIZONTAL)->SetProportion(Bounds().Width()/(docRect.Width()*scale));
		scrollView->ScrollBar(B_VERTICAL)->SetRange(0,docRect.Height()*scale);
		scrollView->ScrollBar(B_VERTICAL)->SetProportion(Bounds().Height()/(docRect.Height()*scale));
	}
}

void GraphEditor::InsertObject(BPoint where,bool deselect)
{
	TRACE();
	BMessage	*commandMessage	= new BMessage(P_C_EXECUTE_COMMAND);
	BMessage	*newObject		= new BMessage(*nodeMessage);
	BMessage	*newFont		= new BMessage(*fontMessage);
	BMessage	*newPattern		= new BMessage(*patternMessage);
	newPattern->ReplaceRGBColor("FillColor",colorItem->GetColor());
	newPattern->ReplaceFloat("PenSize",penSize->GetValue());

	BMessage	*selectMessage	= new BMessage();
	if (GridEnabled())
	{
		where.x=where.x-fmod(where.x,GridWidth());
		where.y=where.y-fmod(where.y,GridWidth());
	}

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

void GraphEditor::InsertRenderObject(BMessage *node)
{
	TRACE();
	Renderer *newRenderer = NULL;
	switch(node->what) 
	{
		case P_C_CLASS_TYPE:
			newRenderer	= new  ClassRenderer(this,node);
		break;
		case P_C_GROUP_TYPE:
//			newRenderer = new BasicGroupView(theValue);
		break;
		case P_C_CONNECTION_TYPE:
			newRenderer	= new ConnectionRenderer(this,node);
		break;
	}		
	node->AddPointer(renderString,newRenderer);
	AddRenderer(newRenderer);
/*	BasePlugin	*theRenderer	= (BasePlugin*)renderPlugins->ItemAt(0);

	BView		*addView		= (BView *)theRenderer->GetNewObject(node);
	if (addView!=NULL)
	{
		AddChild(addView);

	}*/
}

void GraphEditor::DeleteRenderObject(BMessage *node)
{
	TRACE();
			TRACE();

}

void GraphEditor::AddRenderer(Renderer* newRenderer)
{
	TRACE();
	renderer->AddItem(newRenderer);
	activRenderer = newRenderer;
//	delete rendersensitv;
//	rendersensitv = new BRegion();
//	renderer->DoForEach(ProceedRegion,rendersensitv);
}

void GraphEditor::RemoveRenderer(Renderer *wichRenderer)
{
	TRACE();
	if (activRenderer == wichRenderer)
		activRenderer = NULL;
	if (mouseReciver == wichRenderer)
		mouseReciver = NULL;
	renderer->RemoveItem(wichRenderer);
	(wichRenderer->GetMessage())->RemoveName(renderString);

	delete wichRenderer;
/*	delete rendersensitv;
	rendersensitv = new BRegion();
	renderer->DoForEach(ProceedRegion,rendersensitv);*/
	//**recalc Region
}
Renderer* GraphEditor::FindRenderer(BPoint where)
{
	TRACE();
	Renderer *currentRenderer = NULL;
/*	if (rendersensitv->Contains(where))
	{*/
		bool found	=	false;
		for (int32 i=(renderer->CountItems()-1);((!found) && (i>=0));i--)
		{
			if (((Renderer*)renderer->ItemAt(i))->Caught(where))
			{
				found			= true;
				currentRenderer	= (Renderer*)renderer->ItemAt(i);
			}
		}
//	}
	return currentRenderer;
}

Renderer* GraphEditor::FindNodeRenderer(BPoint where)
{
	TRACE();
	Renderer *currentRenderer = NULL;
/*	if (rendersensitv->Contains(where))
	{*/
		bool found	=	false;
		for (int32 i=(renderer->CountItems()-1);((!found) && (i>=0));i--)
		{
			if (((Renderer*)renderer->ItemAt(i))->Caught(where))
			{
				currentRenderer	= (Renderer*)renderer->ItemAt(i);
				if (currentRenderer->GetMessage()->what == P_C_CLASS_TYPE)
					found			= true;
			}
		}
//	}
	return currentRenderer;
}

Renderer* GraphEditor::FindConnectionRenderer(BPoint where)
{
	TRACE();
	Renderer *currentRenderer = NULL;
/*	if (rendersensitv->Contains(where))
	{*/
		bool found	=	false;
		for (int32 i=(renderer->CountItems()-1);((!found) && (i>=0));i--)
		{
			if (((Renderer*)renderer->ItemAt(i))->Caught(where))
			{
				currentRenderer	= (Renderer*)renderer->ItemAt(i);
				if (currentRenderer->GetMessage()->what == P_C_CONNECTION_TYPE)
					found			= true;
			}
		}
//	}
	return currentRenderer;
}


void GraphEditor::BringToFront(Renderer *wichRenderer)
{
	TRACE();
	renderer->RemoveItem(wichRenderer);
	renderer->AddItem(wichRenderer);
	//**draw the new "onTop" renderer
	Invalidate();
	
}

void GraphEditor::SendToBack(Renderer *wichRenderer)
{
	TRACE();
	renderer->RemoveItem(wichRenderer);
	renderer->AddItem(wichRenderer,0);
	//**draw all wich are under the Thing redraw
	Invalidate();
}

bool GraphEditor::DrawRenderer(void *arg,void *editor)
{
	Renderer *painter=(Renderer *)arg;
	painter->Draw((GraphEditor*)editor,BRect(0,0,0,0));
	return false;
}

bool GraphEditor::ProceedRegion(void *arg,void *region)
{
	TRACE();
	Renderer *painter = (Renderer *)arg;
	((BRegion *)region)->Include(painter->Frame());
	return false;
}

