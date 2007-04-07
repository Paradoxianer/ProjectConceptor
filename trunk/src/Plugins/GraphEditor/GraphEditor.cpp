#include <support/List.h>
#include <interface/Font.h>
#include <interface/ScrollView.h>
#include <interface/GraphicsDefs.h>
#include <translation/TranslationUtils.h>
#include <translation/TranslatorFormats.h>
#include <storage/Resources.h>
#include <support/DataIO.h>

#include <string.h>
#include "GraphEditor.h"
#include "PCommandManager.h"
#include "Renderer.h"
#include "ClassRenderer.h"
#include "ConnectionRenderer.h"
#include "GroupRenderer.h"

#include "ToolBar.h"
#include "InputRequest.h"

const char		*G_E_TOOL_BAR			= "G_E_TOOL_BAR";

GraphEditor::GraphEditor(image_id newId):PEditor(),BView(BRect(0,0,200,200),"GraphEditor",B_FOLLOW_ALL_SIDES,B_WILL_DRAW |B_NAVIGABLE|B_NAVIGABLE_JUMP|B_FRAME_EVENTS)
{
	TRACE();
	pluginID	= newId;
	Init();
	BView::SetDoubleBuffering(1);
	SetDrawingMode(B_OP_ALPHA);
}

void GraphEditor::Init(void)
{
	TRACE();
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
	rgb_color	highColor			= {0, 0, 0, 255};
	patternMessage->AddRGBColor("HighColor",highColor);
	rgb_color 	lowColor			= {128, 128, 128, 255};
	patternMessage->AddRGBColor("LowColor",lowColor);
	patternMessage->AddData("Pattern",B_PATTERN_TYPE,(const void *)&B_SOLID_HIGH,sizeof(B_SOLID_HIGH));

	scaleMenu		= new BMenu(_T("Scale"));
	BMessage	*newScale	= new BMessage(G_E_NEW_SCALE);
	newScale->AddFloat("scale",0.1);
	scaleMenu->AddItem(new BMenuItem("10 %",newScale,0,0));
	newScale	= new BMessage(G_E_NEW_SCALE);
	newScale->AddFloat("scale",0.25);
	scaleMenu->AddItem(new BMenuItem("25 %",newScale,0,0));
	newScale	= new BMessage(G_E_NEW_SCALE);
	newScale->AddFloat("scale",0.33);
	scaleMenu->AddItem(new BMenuItem("33 %",newScale,0,0));
	newScale	= new BMessage(G_E_NEW_SCALE);
	newScale->AddFloat("scale",0.5);
	scaleMenu->AddItem(new BMenuItem("50 %",newScale,0,0));
	newScale	= new BMessage(G_E_NEW_SCALE);
	newScale->AddFloat("scale",0.75);
	scaleMenu->AddItem(new BMenuItem("75 %",newScale,0,0));
	newScale	= new BMessage(G_E_NEW_SCALE);
	newScale->AddFloat("scale",1.00);
	scaleMenu->AddItem(new BMenuItem("100 %",newScale,0,0));
	newScale	= new BMessage(G_E_NEW_SCALE);
	newScale->AddFloat("scale",1.5);
	scaleMenu->AddItem(new BMenuItem("150 %",newScale,0,0));
	newScale	= new BMessage(G_E_NEW_SCALE);
	newScale->AddFloat("scale",2);
	scaleMenu->AddItem(new BMenuItem("200 %",newScale,0,0));
	newScale	= new BMessage(G_E_NEW_SCALE);
	newScale->AddFloat("scale",5);
	scaleMenu->AddItem(new BMenuItem("500 %",newScale,0,0));
	newScale	= new BMessage(G_E_NEW_SCALE);
	newScale->AddFloat("scale",10);
	scaleMenu->AddItem(new BMenuItem("1000 %",newScale,0,0));

	grid		= new ToolItem("Grid",BTranslationUtils::GetBitmap(B_PNG_FORMAT,"grid"),new BMessage(G_E_GRID_CHANGED),P_M_TWO_STATE_ITEM);
	penSize		= new FloatToolItem(_T("Pen Size"),1.0,new BMessage(G_E_PEN_SIZE_CHANGED));
	colorItem	= new ColorToolItem(_T("Fill"),fillColor,new BMessage(G_E_COLOR_CHANGED));
	patternItem	= new PatternToolItem(_T("Pattern"),B_SOLID_HIGH, new BMessage(G_E_PATTERN_CHANGED));

	toolBar				= new ToolBar(BRect(1,1,50,2800),G_E_TOOL_BAR,B_ITEMS_IN_COLUMN);
	//loading ressource_images from the PluginRessource
	image_info	*info 	= new image_info;
	BBitmap		*bmp	= NULL;
	size_t		size;
	// look up the plugininfos
	get_image_info(pluginID,info);
	// init the ressource for the plugin files
	BResources *res=new BResources(new BFile((const char*)info->name,B_READ_ONLY));
	// load the addBool icon
	const void *data=res->LoadResource((type_code)'PNG ',"group",&size);
	if (data)
	{
		//translate the icon because it was png but we ne a bmp
		bmp = BTranslationUtils::GetBitmap(new BMemoryIO(data,size));
		if (bmp)
		{
			BMessage	*groupMessage = new BMessage(G_E_GROUP);
			addGroup	= new ToolItem("addGroup",bmp,groupMessage);
			toolBar->AddItem(addGroup);
		}
	}
	toolBar->AddSeperator();
	data=res->LoadResource((type_code)'PNG ',"addBool",&size);
	if (data)
	{
		//translate the icon because it was png but we ne a bmp
		bmp = BTranslationUtils::GetBitmap(new BMemoryIO(data,size));
		if (bmp)
		{
			BMessage	*addBoolMessage = new BMessage(G_E_ADD_ATTRIBUTE);
			addBoolMessage->AddInt32("type",B_BOOL_TYPE);
			addBool		= new ToolItem("addBool",bmp,addBoolMessage);
			toolBar->AddItem(addBool);
		}
	}

	data=res->LoadResource((type_code)'PNG ',"addText",&size);
	if (data)
	{
		bmp = BTranslationUtils::GetBitmap(new BMemoryIO(data,size));
		if (bmp)
		{
			BMessage	*addTextMessage = new BMessage(G_E_ADD_ATTRIBUTE);
			addTextMessage->AddInt32("type",B_STRING_TYPE);
			addText		= new ToolItem("addText",bmp,addTextMessage);
			toolBar->AddItem(addText);
		}
	}
}

void GraphEditor::AttachedToManager(void)
{
	TRACE();

	sentTo				= new BMessenger(doc);
	id					= manager->IndexOf(this);
	status_t	err		= B_OK;
	sprintf(renderString,"GraphEditor%ld::Renderer",id);
	//put this in a seperate function??
	nodeMessage->AddPointer("doc",doc);

	BMessage	*shortcuts	= new BMessage();
	BMessage	*tmpMessage	= new BMessage();
	BMessage	*sendMessage;
	shortcuts->AddInt32("key",B_DELETE);
	sendMessage	= new BMessage(P_C_EXECUTE_COMMAND);
	sendMessage->AddString("Command::Name","Delete");
	shortcuts->AddInt32("modifiers",0);
	shortcuts->AddMessage("message",sendMessage);
	shortcuts->AddPointer("handler",new BMessenger(doc,NULL,&err));


	BMessage	*addBoolMessage = new BMessage(G_E_ADD_ATTRIBUTE);
	addBoolMessage->AddInt32("type",B_BOOL_TYPE);
	shortcuts->AddInt32("key",'b');
	shortcuts->AddInt32("modifiers",B_COMMAND_KEY);
	shortcuts->AddMessage("message",addBoolMessage);
	shortcuts->AddPointer("handler",new BMessenger(this,NULL,&err));

	BMessage	*insertNode = new BMessage(G_E_INSERT_NODE);
	shortcuts->AddInt32("key",B_INSERT);
	shortcuts->AddInt32("modifiers",0);
	shortcuts->AddMessage("message",insertNode);
	shortcuts->AddPointer("handler",new BMessenger(this,NULL,&err));

	if (configMessage->FindMessage("Shortcuts",tmpMessage)==B_OK)
		configMessage->ReplaceMessage("Shortcuts",shortcuts);
	else
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
	void		*renderer		= NULL;
	for (int32 i=0;i<allNodes->CountItems();i++)
	{
		node = (BMessage *)allNodes->ItemAt(i);
		if (node->FindPointer(renderString,&renderer) != B_OK)
		{
			InsertRenderObject(node);
		}
	}
	for (int32 i=0;i<allConnections->CountItems();i++)
	{
		node = (BMessage *)allConnections->ItemAt(i);
		if (node->FindPointer(renderString,&renderer) != B_OK)
		{
			InsertRenderObject(node);
		}
	}
}

void GraphEditor::PreprocessBeforSave(BMessage *container)
{
	TRACE();
	PRINT(("GraphEditor::PreprocessAfterLoad:\n"));
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
			(strcasecmp(name,"Parent") == B_OK)  ||
			(strcasecmp(name,"doc") == B_OK) )
		{
			container->RemoveName(name);
			i--;
		}
		i++;
	}
}

void GraphEditor::ValueChanged()
{
	TRACE();
	BList		*changedNodes	= doc->GetChangedNodes();
	BList		*allTrashed		= doc->GetTrash();
	BList		*allNodes		= doc->GetAllNodes();
	BList		*allConnections	= doc->GetAllConnections();

	BMessage	*node			= NULL;
	Renderer	*painter		= NULL;
	void		*pointer		= NULL;
	BRect		frame;
	BRect		invalid;
	for (int32 i=0;i<changedNodes->CountItems();i++)
	{
		node = (BMessage *)changedNodes->ItemAt(i);
		if (node->FindPointer(renderString,(void **)&painter) == B_OK)
		{
			if ((allConnections->HasItem(node))||(allNodes->HasItem(node)))
			{
				painter->ValueChanged();
				//if we procedded this node than it´s not changed anymore
//				changedNodes->RemoveItem(i);
//				i--;
			}
			else
			{
				RemoveRenderer(FindRenderer(node));
				//if we procedded this node than it´s not changed anymore
//				changedNodes->RemoveItem(i);
//				i--;
			}
		}
		else
		{
			//**check if this node is in the node or connection list because it it is not it´s a nodd frome a subgroup or it was deleted
			if (((allConnections->HasItem(node))||(allNodes->HasItem(node))) && (node->FindPointer("Parent",&pointer) !=B_OK))
			{
				InsertRenderObject(node);
 				//if we procedded this node than it´s not changed anymore
//				changedNodes->RemoveItem(i);
//				i--;

			}
			else
			{
				RemoveRenderer(FindRenderer(node));
				//if we procedded this node than it´s not changed anymore
//				changedNodes->RemoveItem(i);
//				i--;
			}
		}
	}

	if (BView::LockLooper())
	{
		Invalidate();
		BView::UnlockLooper();
	}
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

	BMessage *currentMsg = Window()->CurrentMessage();
	uint32 modifiers = 0;
	uint32 buttons	= 0;
//	GetMouse(&where,&buttons);
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
		currentMsg->FindInt32("buttons", (int32 *)&buttons);
		currentMsg->FindInt32("modifiers", (int32 *)&modifiers);
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

	toolBar->ResizeTo(30,pWindow->P_M_MAIN_VIEW_BOTTOM-pWindow->P_M_MAIN_VIEW_TOP);
	pWindow->AddToolBar(toolBar);
	addGroup->SetTarget(this);
	addBool->SetTarget(this);
	addText->SetTarget(this);
	ToolBar		*configBar	= (ToolBar *)pWindow->FindView(P_M_STANDART_TOOL_BAR);
	configBar->AddSeperator();
	configBar->AddSeperator();
	configBar->AddSeperator();
	configBar->AddSeperator();
	configBar->AddSeperator();
	configBar->AddItem(grid);
	configBar->AddSeperator();
	configBar->AddItem(penSize);
	configBar->AddItem(colorItem);

	grid->SetTarget(this);
	penSize->SetTarget(this);
	colorItem->SetTarget(this);	sentToMe	= new BMessenger((BView *)this);
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
//		pWindow->AddToolBar(toolBar);
		pWindow->RemoveToolBar(G_E_TOOL_BAR	);
		ToolBar		*configBar	= (ToolBar *)pWindow->FindView(P_M_STANDART_TOOL_BAR);
		if (configBar)
		{
			configBar->RemoveItem(penSize);
			configBar->RemoveItem(colorItem);
			configBar->RemoveItem(patternItem);
			configBar->RemoveSeperator();
			configBar->RemoveItem(grid);
			configBar->RemoveSeperator();
			configBar->RemoveSeperator();
			configBar->RemoveSeperator();
			configBar->RemoveSeperator();
			configBar->RemoveSeperator();
		}
		Renderer	*nodeRenderer	= NULL;
		for (int32 i=0;i<renderer->CountItems();i++)
		{
			nodeRenderer = (Renderer *)renderer->ItemAt(i);
		}
		while(renderer->CountItems()>0)
		{
			nodeRenderer = (Renderer *)renderer->ItemAt(0);
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
			ResizeTo(docRect.right,docRect.bottom);
			break;
		}
		case G_E_CONNECTING:
		{
				connecting = true;
				message->FindPoint("To",toPoint);
				message->FindPoint("From",fromPoint);
				Invalidate();
			break;
		}
		case G_E_CONNECTED:
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
		case G_E_NEW_SCALE:
		{
			message->FindFloat("scale",&scale);
			SetScale(scale);
			FrameResized(0,0);
			Invalidate();
			break;
		}
		case G_E_GRID_CHANGED:
		{
			gridEnabled =! gridEnabled;
			Invalidate();
			break;
		}
		case G_E_COLOR_CHANGED:
		{
			rgb_color	tmpNewColor =	{255, 0, 0, 255};
			BMessage	*changeColorMessage	= new BMessage(P_C_EXECUTE_COMMAND);
			changeColorMessage->AddString("Command::Name","ChangeValue");
			changeColorMessage->AddBool("selected",true);
			BMessage	*valueContainer	= new BMessage();
			valueContainer->AddString("name","FillColor");
			valueContainer->AddString("subgroup","Pattern");
			valueContainer->AddInt32("type",B_RGB_COLOR_TYPE);
			valueContainer->AddRGBColor("newValue",colorItem->GetColor());
			changeColorMessage->AddMessage("valueContainer",valueContainer);
			sentTo->SendMessage(changeColorMessage);
			break;
		}
		case G_E_PEN_SIZE_CHANGED:
		{
			BMessage	*changePenSizeMessage	= new BMessage(P_C_EXECUTE_COMMAND);
			changePenSizeMessage->AddString("Command::Name","ChangeValue");
			changePenSizeMessage->AddBool("selected",true);
			BMessage	*valueContainer	= new BMessage();
			valueContainer->AddString("name","PenSize");
			valueContainer->AddString("subgroup","Pattern");
			valueContainer->AddInt32("type",B_FLOAT_TYPE);
			valueContainer->AddFloat("newValue",penSize->GetValue());
			changePenSizeMessage->AddMessage("valueContainer",valueContainer);
			sentTo->SendMessage(changePenSizeMessage);
			break;
		}
		case G_E_ADD_ATTRIBUTE:
		{
			int32	type;
			char	*datadummy	= new char[4];
			strcpy(datadummy,"    ");
			message->FindInt32("type",&type);
			InputRequest	*inputAlert = new InputRequest(_T("Input AttributName"),_T("Name"), _T("Attribut"), _T("OK"),_T("Cancel"));
			char			*input		= NULL;
			char			*inputstr	= NULL;
			if (inputAlert->Go(&input)<1)
			{
				inputstr	= new char[strlen(input)+1];
				strcpy(inputstr,input);
				BMessage	*addMessage		= new BMessage(P_C_EXECUTE_COMMAND);
				addMessage->AddString("Command::Name","AddAttribute");
				addMessage->AddBool("selected",true);
				BMessage	*valueContainer	= new BMessage();

				valueContainer->AddInt32("type",B_MESSAGE_TYPE);
				valueContainer->AddString("name",inputstr);
				valueContainer->AddString("subgroup","Data");
				BMessage	*newAttribute	= new BMessage(type);
				newAttribute->AddString("Name",inputstr);
				newAttribute->AddData("Value",type,datadummy,sizeof(datadummy));
				valueContainer->AddMessage("newAttribute",newAttribute);
				addMessage->AddMessage("valueContainer",valueContainer);
				sentTo->SendMessage(addMessage);
			}
			break;
		}
		case G_E_INSERT_NODE:
		{
            sentTo->SendMessage(GenerateInsertCommand(P_C_CLASS_TYPE));
			break;
		}
		case G_E_INSERT_SIBLING:
		{
			 sentTo->SendMessage(GenerateInsertCommand(P_C_CLASS_TYPE));
			break;
		}
		case G_E_INVALIDATE:
		{
			Invalidate();
			break;
		}
		case G_E_GROUP:
		{
			BMessage	*commandMessage	= GenerateInsertCommand(P_C_GROUP_TYPE);
			commandMessage->ReplaceString("Command::Name","Group");
			sentTo->SendMessage(commandMessage);
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
	void	*tmpDoc	= NULL;
	if (node->FindPointer("doc",&tmpDoc)==B_OK)
		node->ReplacePointer("doc",doc);
	else
		node->AddPointer("doc",doc);
	switch(node->what)
	{
		case P_C_CLASS_TYPE:
			newRenderer	= new  ClassRenderer(this,NULL,node);
		break;
		case P_C_GROUP_TYPE:
			newRenderer = new GroupRenderer(this,NULL,node);
		break;
		case P_C_CONNECTION_TYPE:
			newRenderer	= new ConnectionRenderer(this,NULL,node);
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
	if (wichRenderer != NULL)
	{
		if (activRenderer == wichRenderer)
			activRenderer = NULL;
		if (mouseReciver == wichRenderer)
			mouseReciver = NULL;
		renderer->RemoveItem(wichRenderer);
		(wichRenderer->GetMessage())->RemoveName(renderString);

		delete wichRenderer;
	}
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
				if ((currentRenderer->GetMessage()->what == P_C_CLASS_TYPE) || (currentRenderer->GetMessage()->what == P_C_GROUP_TYPE) )
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

Renderer* GraphEditor::FindRenderer(BMessage *container)
{
	int32		i					= 0;
	Renderer	*currentRenderer	= NULL;
	bool		found				= false;

	while ((i<renderer->CountItems()) && (!found))
	{
		currentRenderer= (Renderer*)renderer->ItemAt(i);
		if (currentRenderer->GetMessage() == container)
			found=true;
		i++;
	}
	if (found)
		return currentRenderer;
	else
		return NULL;
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
BMessage *GraphEditor::GenerateInsertCommand(uint32 newWhat)
{
	BList		*selected			= doc->GetSelected();
	BMessage	*newNode	    	= new BMessage(*nodeMessage);
	BMessage	*newFont	    	= new BMessage(*fontMessage);
	BMessage	*newPattern		    = new BMessage(*patternMessage);
	BMessage	*connection		    = NULL;
	BMessage	*commandMessage     = new BMessage(P_C_EXECUTE_COMMAND);
	BMessage	*subCommandMessage	= new BMessage(P_C_EXECUTE_COMMAND);
	BRect       *fromRect           = new BRect();
	BRect       *selectRect         = NULL;
	BMessage	*from			    = NULL;
	BMessage	*to				    = newNode;
	BMessage	*data		    	= new BMessage();
	int32		i                   = 0;
	status_t    err                 = B_OK;
	BPoint      where;

    data->AddString("Name","Unbenannt");
    //insert new Node here*/
	newPattern->ReplaceRGBColor("FillColor",colorItem->GetColor());
	newPattern->ReplaceFloat("PenSize",penSize->GetValue());
    //** we need a good algorithm to find the best rect for this new node we just put it at 100,100**/
	where = BPoint(100,100);
	if (GridEnabled())
	{
		where.x = where.x-fmod(where.x,GridWidth());
		where.y = where.y-fmod(where.y,GridWidth());
	}
	newNode->what = newWhat;
	newNode->AddMessage("Font",newFont);
	newNode->AddMessage("Pattern",newPattern);
	commandMessage->AddString("Command::Name","Insert");
    subCommandMessage->AddString("Command::Name","Select");
	commandMessage->AddPointer("node",newNode);
    subCommandMessage->AddPointer("node",newNode);

    while (i<selected->CountItems())
	{
		from	= (BMessage *)selected->ItemAt(i);
        if (to != NULL && from!=NULL)
        {
            connection		    = new BMessage(P_C_CONNECTION_TYPE);
            err = from->FindRect("Frame",fromRect);
            if (!selectRect)
                selectRect = new BRect(*fromRect);
            else
                *selectRect = *selectRect | *fromRect;
            err = B_OK;
            connection->AddPointer("From",from);
            connection->AddPointer("To",to);
            connection->AddMessage("Data",data);
            connection->AddPointer("doc",doc);
            //** add the connections to the Nodes :-)
            commandMessage->AddPointer("node",connection);
        }
		i++;
	}
	where.x         = selectRect->right+100;
	int32 middle    = selectRect->top+(selectRect->Height()/2);
	where.y         = middle;
	int32 step      = -1;
	while (FindRenderer(where)!=NULL)
	{
		    where.y = middle + (step*85);
		if (step>0)
   			step++;
		step=-step;
	}
	newNode->AddRect("Frame",BRect(where,where+BPoint(100,80)));
	commandMessage->AddMessage("PCommand::subPCommand",subCommandMessage);
	return commandMessage;
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

