#include <support/List.h>
#include <interface/Font.h>
#include <interface/ScrollView.h>
#include <interface/GraphicsDefs.h>
#include <translation/TranslationUtils.h>
#include <translation/TranslatorFormats.h>
#include <storage/Resources.h>
#include <support/DataIO.h>
#include <string.h>
#include <Catalog.h>

#include "GraphEditor.h"
#include "PCommandManager.h"
#include "Renderer.h"
#include "ClassRenderer.h"
#include "ConnectionRenderer.h"
#include "GroupRenderer.h"
#include "PWindow.h"
#include "PEditorManager.h"


#include "ToolBar.h"
#include "ToolItem.h"
#include "InputRequest.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "GraphEditor"


const char		*G_E_TOOL_BAR			= "G_E_TOOL_BAR";

GraphEditor::GraphEditor(image_id newId):PEditor(),BView(BRect(0,0,400,400),"GraphEditor",B_FOLLOW_ALL_SIDES,B_WILL_DRAW | B_NAVIGABLE) {
	TRACE();
	pluginID	= newId;
	Init();
#ifndef __HAIKU__
	BView::SetDoubleBuffering(1);
#endif
	SetDrawingMode(B_OP_ALPHA);
}

void GraphEditor::Init(void) {
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
	myScrollParent	= NULL;


	font_family		family;
	font_style		style;

	BMessage		*dataMessage	= new BMessage();
	dataMessage->AddString("Name","Untitled");
	//preparing the standart ObjectMessage
	nodeMessage	= new BMessage(P_C_CLASS_TYPE);
	nodeMessage->AddMessage("Node::Data",dataMessage);
	//Preparing the standart FontMessage
	fontMessage		= new BMessage(B_FONT_TYPE);
	fontMessage->AddInt8("Font::Encoding",be_plain_font->Encoding());
	fontMessage->AddInt16("Font::Face",be_plain_font->Face());
	be_plain_font->GetFamilyAndStyle(&family,&style);
	fontMessage->AddString("Font::Family",(const char*)&family);
	fontMessage->AddInt32("Font::Flags", be_plain_font->Flags());
	fontMessage->AddFloat("Font::Rotation",be_plain_font->Rotation());
	fontMessage->AddFloat("Font::Shear",be_plain_font->Shear());
	fontMessage->AddFloat("Font::Size",be_plain_font->Size());
	fontMessage->AddInt8("Font::Spacing",be_plain_font->Spacing());
	fontMessage->AddString("Font::Style",(const char*)&style);
	rgb_color	fontColor			= {111, 151, 181, 255};
	fontMessage->AddInt32("Font::Color",*((int32 *)&fontColor));

	//perparing Pattern Message
	patternMessage	=new BMessage();
	//standart Color
	rgb_color	fillColor			= {152, 180, 190, 255};
	patternMessage->AddInt32("FillColor",*(int32 *)&fillColor);
	rgb_color	borderColor			= {0, 0, 0, 255};
	patternMessage->AddInt32("BorderColor",*(int32 *)&borderColor);
	patternMessage->AddFloat("PenSize",1.0);
	patternMessage->AddInt8("DrawingMode",B_OP_ALPHA);
	rgb_color	highColor			= {0, 0, 0, 255};
	patternMessage->AddInt32("HighColor",*(int32 *)&highColor);
	rgb_color 	lowColor			= {128, 128, 128, 255};
	patternMessage->AddInt32("LowColor",*(int32 *)&lowColor);
	patternMessage->AddData("Pattern",B_PATTERN_TYPE,(const void *)&B_SOLID_HIGH,sizeof(B_SOLID_HIGH),false);

	scaleMenu		= new BMenu(B_TRANSLATE("Scale"));
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

	grid		= new ToolItem(B_TRANSLATE("Grid"),BTranslationUtils::GetBitmap(B_PNG_FORMAT,"grid"),new BMessage(G_E_GRID_CHANGED),P_M_TWO_STATE_ITEM);
	penSize		= new FloatToolItem(B_TRANSLATE("Pen size"),1.0,new BMessage(G_E_PEN_SIZE_CHANGED));
	colorItem	= new ColorToolItem(B_TRANSLATE("Fill"),fillColor,new BMessage(G_E_COLOR_CHANGED));
	patternItem	= new PatternToolItem(B_TRANSLATE("Node::Pattern"),B_SOLID_HIGH, new BMessage(G_E_PATTERN_CHANGED));

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
	if (data) {
		//translate the icon because it was png but we ne a bmp
		bmp = BTranslationUtils::GetBitmap(new BMemoryIO(data,size));
		if (bmp) {
			BMessage	*groupMessage = new BMessage(G_E_GROUP);
			addGroup	= new ToolItem("addGroup",bmp,groupMessage);
			toolBar->AddItem(addGroup);
		}
	}
	toolBar->AddSeperator();
	data=res->LoadResource((type_code)'PNG ',"addBool",&size);
	if (data) {
		//translate the icon because it was png but we ne a bmp
		bmp = BTranslationUtils::GetBitmap(new BMemoryIO(data,size));
		if (bmp) {
			BMessage	*addBoolMessage = new BMessage(G_E_ADD_ATTRIBUTE);
			addBoolMessage->AddInt32("type",B_BOOL_TYPE);
			addBool		= new ToolItem("addBool",bmp,addBoolMessage);
			toolBar->AddItem(addBool);
		}
	}

	data=res->LoadResource((type_code)'PNG ',"addText",&size);
	if (data) {
		bmp = BTranslationUtils::GetBitmap(new BMemoryIO(data,size));
		if (bmp) {
			BMessage	*addTextMessage = new BMessage(G_E_ADD_ATTRIBUTE);
			addTextMessage->AddInt32("type",B_STRING_TYPE);
			addText		= new ToolItem("addText",bmp,addTextMessage);
			toolBar->AddItem(addText);
		}
	}
}

void GraphEditor::AttachedToManager(void) {
	TRACE();

	sentTo				= new BMessenger(doc);
	id					= manager->IndexOf(this);
	status_t	err		= B_OK;
	sprintf(renderString,"GraphEditor%ld::Renderer",id);
	//put this in a seperate function??
	nodeMessage->AddPointer("ProjectConceptor::doc",doc);

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
/*	if (doc!=NULL)
		this->ResizeTo(doc->Bounds().Width(),doc->Bounds().Height());*/
	UpdateScrollBars();
}

void GraphEditor::DetachedFromManager(void) {
	TRACE();
}

BView* GraphEditor::GetView(void) {
	if (myScrollParent)
		return myScrollParent;
	else {
		
		myScrollParent = new BScrollView("GEScrolly",this,B_FOLLOW_ALL_SIDES,0,true,true);
		return myScrollParent;
	}
	return this;
}


BList* GraphEditor::GetPCommandList(void) {
	TRACE();
	//at the Moment we dont support special Commands :-)
	return NULL;
}
void GraphEditor::InitAll() {
	TRACE();
	BList		*allNodes		= doc->GetAllNodes();
	BList		*allConnections	= doc->GetAllConnections();

	BMessage	*node			= NULL;
	void		*renderer		= NULL;
	for (int32 i=0;i<allNodes->CountItems();i++) {
		node = (BMessage *)allNodes->ItemAt(i);
		if (node->FindPointer(renderString,&renderer) != B_OK) {
			InsertRenderObject(node);
		}
	}
	for (int32 i=0;i<allConnections->CountItems();i++) {
		node = (BMessage *)allConnections->ItemAt(i);
		if (node->FindPointer(renderString,&renderer) != B_OK)
			InsertRenderObject(node);
	}
}

void GraphEditor::PreprocessBeforSave(BMessage *container) {
	TRACE();
	PRINT(("GraphEditor::PreprocessAfterLoad:\n"));
	char	*name;
	uint32	type;
	int32	count;
	int32	i		= 0;
	//remove all the Pointer to the Renderer so that on the next load a new Renderer are added
	#ifdef B_ZETA_VERSION_1_0_0
	while (container->GetInfo(B_POINTER_TYPE,i ,(const char **)&name, &type, &count) == B_OK)
	#else
	while (container->GetInfo(B_POINTER_TYPE,i ,(char **)&name, &type, &count) == B_OK)
	#endif
	{
		if ((strstr(name,"GraphEditor") != NULL) ||
			(strcasecmp(name,"Node::outgoing") == B_OK) ||
			(strcasecmp(name,"Node::incoming") == B_OK) ||
			(strcasecmp(name,"Parent") == B_OK)  ||
			(strcasecmp(name,"ProjectConceptor::doc") == B_OK) ){
			container->RemoveName(name);
			i--;
		}
		i++;
	}
}

void GraphEditor::PreprocessAfterLoad(BMessage *container) {
	//**nothing to do jet as i know
	container=container;
}

void GraphEditor::ValueChanged() {
	TRACE();
	//try to lock the document during we are painting
	printf("ValueChanged - trying to Lock the Document now\n");

	status_t err = doc->LockWithTimeout(TIMEOUT_LOCK);
	printf("DocLocError - %s\n",strerror(err));
	
	set<BMessage*>	*changedNodes	= doc->GetChangedNodes();
	set<BMessage*>::iterator it;

	BList		*allNodes	= doc->GetAllNodes();
	BList		*allConnections	= doc->GetAllConnections();

	BMessage	*node			= NULL;
	Renderer	*painter		= NULL;
	void		*pointer		= NULL;
	BRect		frame;
	BRect		invalid;
	int i=0;
	for ( it=changedNodes->begin();it!=changedNodes->end();it++) {
		node = *it;
		if (node->FindPointer(renderString,(void **)&painter) == B_OK) {
			if ((allConnections->HasItem(node))||(allNodes->HasItem(node)))
				painter->ValueChanged();
			else
				RemoveRenderer(FindRenderer(node));
		}
		else {
			//**check if this node is in the node or connection list because if it is not it´s a node frome a subgroup or it was deleted
			if (((allConnections->HasItem(node))||
			     (allNodes->HasItem(node))) &&
			    (node->FindPointer("Parent",&pointer) !=B_OK))
				InsertRenderObject(node);
			else
				RemoveRenderer(FindRenderer(node));
		}
	}
	if (err == B_OK)
	    doc->Unlock();
	Invalidate();
}

void GraphEditor::SetDirty(BRegion *region) {
	TRACE();
	BView::Invalidate(region);
}


void GraphEditor::Draw(BRect updateRect) {
	SetHighColor(230,230,230,255);
	SetScale((1.0/scale));
	PushState();
	BView::Draw(updateRect);
	SetScale(scale);
	PushState();
	if (gridEnabled) {
		int32		xcount		= (Frame().Width()/gridWidth)+1;
		int32		ycount		= (Frame().Height()/gridWidth)+1;
		float		x			= 0;
		float		y			= 0;
		rgb_color	gridColor	= tint_color(ViewColor(),1.1);
		BeginLineArray(xcount+ycount);
		for (int32 i=1;i<xcount;i++) {
			AddLine(BPoint(x,Bounds().top),BPoint(x,Frame().Height()),gridColor);
			x += gridWidth;
		}
		for (int32 i=1;i<ycount;i++) {
			AddLine(BPoint(Bounds().left,y),BPoint(Frame().Width(),y),gridColor);
			y += gridWidth;
		}
		EndLineArray();
	}
	renderer->DoForEach(DrawRenderer,this);
	if (selectRect) {
		SetHighColor(81,131,171,120);
		FillRect(*selectRect);
		SetHighColor(31,81,121,255);
		StrokeRect(*selectRect);
	}
	else if (connecting) {
		SetHighColor(50,50,50,255);
		StrokeLine(*fromPoint,*toPoint);
	}

}

void GraphEditor::MouseDown(BPoint where) {
	BView::MouseDown(where);
	BView::MakeFocus(true);
	
	BPoint		scaledWhere;
	scaledWhere.x	= where.x / scale;
	scaledWhere.y	= where.y / scale;

	BMessage *currentMsg = Window()->CurrentMessage();
	int32 modifiers		= 0;
	int32 buttons		= 0;
	int32 clicks		= 0;
	
	currentMsg->FindInt32("buttons", &buttons);
	currentMsg->FindInt32("clicks", &clicks);
	currentMsg->FindInt32("modifiers", &modifiers);
	bool found	=	false;
	for (int32 i=(renderer->CountItems()-1);((!found) && (i>=0) );i--) {
		if (((Renderer*)renderer->ItemAt(i))->Caught(scaledWhere)) {
			mouseReciver = (Renderer*)renderer->ItemAt(i);
			mouseReciver->MouseDown(scaledWhere,buttons, clicks, modifiers);
			found			= true;
		}
	}
	if (!found) {
		currentMsg->FindInt32("buttons", (int32 *)&buttons);
		currentMsg->FindInt32("modifiers", (int32 *)&modifiers);
		if (buttons & B_PRIMARY_MOUSE_BUTTON) {
			startMouseDown=new BPoint(scaledWhere);
			oldEventMask = EventMask();
			//EventMaske setzen so dass die Maus auch über den View verfolgt wird
			SetMouseEventMask(B_POINTER_EVENTS, B_NO_POINTER_HISTORY  | B_LOCK_WINDOW_FOCUS);
		}
		//if any other mousebutton was clicked we just send a deselect command
		else {
			BMessage *selectMessage=new BMessage(P_C_EXECUTE_COMMAND);
			selectMessage->AddRect("frame", BRect(-10000,-10000,-9999,-9999));
			selectMessage->AddString("Command::Name","Select");
			sentTo->SendMessage(selectMessage);
		}
	}
}


void GraphEditor::MouseMoved(	BPoint where, uint32 code, const BMessage *a_message) {
	BView::MouseMoved(where,code,a_message);
	BPoint		scaledWhere;
	scaledWhere.x	= where.x / scale;
	scaledWhere.y	= where.y / scale;
	if (startMouseDown) {
		//only if the user hase moved the Mouse we start to select...
		float dx=scaledWhere.x - startMouseDown->x;
		float dy=scaledWhere.y - startMouseDown->y;
		float entfernung=sqrt(dx*dx+dy*dy);
		if (entfernung>max_entfernung) {
			if (selectRect!=NULL) {
				selectRect->SetLeftTop(*startMouseDown);
				selectRect->SetRightBottom(scaledWhere);
			}
			else {
				selectRect=new BRect(*startMouseDown,scaledWhere);
			}

			//make the Rect valid
			if (selectRect->left>selectRect->right) {
				float c=selectRect->left;
				selectRect->left=selectRect->right;
				selectRect->right=c;
			}
			if (selectRect->top>selectRect->bottom) {
				float c=selectRect->top;
				selectRect->top=selectRect->bottom;
				selectRect->bottom=c;
			}
			Invalidate();
		}
	}
	else if (mouseReciver != NULL) {
		mouseReciver->MouseMoved(scaledWhere,code,a_message);
	}
}

void GraphEditor::MouseUp(BPoint where) {
	BView::MouseUp(where);
	BPoint		scaledWhere;
	scaledWhere.x	= where.x / scale;
	scaledWhere.y	= where.y / scale;
	if (startMouseDown != NULL) {
		if (selectRect != NULL) {
			BMessage *selectMessage=new BMessage(P_C_EXECUTE_COMMAND);
			selectMessage->AddRect("frame",*selectRect);
			selectMessage->AddString("Command::Name","Select");
			sentTo->SendMessage(selectMessage);
			SetMouseEventMask(oldEventMask);
		}
		else {

			BMessage	*currentMsg	= Window()->CurrentMessage();
			uint32		buttons		= 0;
			uint32		modifiers	= 0;
			currentMsg->FindInt32("buttons", (int32 *)&buttons);
			currentMsg->FindInt32("modifiers", (int32 *)&modifiers);
			if ((modifiers & B_CONTROL_KEY) != 0)
				InsertObject(scaledWhere,false);
			else
				InsertObject(scaledWhere,true);
		}
		delete startMouseDown;
		startMouseDown=NULL;
		delete selectRect;
		selectRect=NULL;
		Invalidate();
	}
	else if (mouseReciver != NULL) {
		mouseReciver->MouseUp(scaledWhere);
		mouseReciver = NULL;
	}
}



void GraphEditor::AttachedToWindow(void) {
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
	colorItem->SetTarget(this);
	sentToMe	= new BMessenger((BView *)this);
	BView *parent = myScrollParent->Parent();
	if (parent) {
		BRect rect = parent->Bounds();
		rect.InsetBy(2.5,2.5);
		myScrollParent->ResizeTo(rect.Width(),rect.Height());
	}
/*	if (doc!=NULL)
	{
		BRect rect = doc->Bounds();
		this->ResizeTo(rect.Width(),rect.Height());
		BRect		scrollRect	= myScrollParent->Bounds();
		scrollRect.right	-= (B_V_SCROLL_BAR_WIDTH +2);
		scrollRect.bottom	-= (B_H_SCROLL_BAR_HEIGHT+2);
		scrollRect.InsetBy(2,2);
		this->ConstrainClippingRegion(new BRegion(scrollRect));
	}*/
}



void GraphEditor::DetachedFromWindow(void) {
	TRACE();
	if (Window()) {
		PWindow 	*pWindow		= (PWindow *)Window();
		BMenuBar	*menuBar		= (BMenuBar *)pWindow->FindView(P_M_STATUS_BAR);
		if (menuBar)
			menuBar->RemoveItem(scaleMenu);
//		pWindow->AddToolBar(toolBar);
		pWindow->RemoveToolBar(G_E_TOOL_BAR	);
		ToolBar		*configBar	= (ToolBar *)pWindow->FindView(P_M_STANDART_TOOL_BAR);
		if (configBar) {
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
		for (int32 i=0;i<renderer->CountItems();i++) {
			nodeRenderer = (Renderer *)renderer->ItemAt(i);
		}
		while(renderer->CountItems()>0) {
			nodeRenderer = (Renderer *)renderer->ItemAt(0);
			RemoveRenderer(nodeRenderer);
		}
	}
}
void GraphEditor::MessageReceived(BMessage *message) {
	//TRACE();
	//message->PrintToStream();
	switch(message->what) {
		case P_C_VALUE_CHANGED: {
			ValueChanged();
			break;
		}
		case P_C_DOC_BOUNDS_CHANGED: {
			UpdateScrollBars();
			break;
		}
		case G_E_CONNECTING: {
				connecting = true;
				message->FindPoint("Node::to",toPoint);
				message->FindPoint("Node::from",fromPoint);
				Invalidate();
			break;
		}
		case G_E_CONNECTED: {
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
			if (message->FindPointer("Node::from",(void **)&from) == B_OK) {
				message->FindPoint("Node::to",toPointer);
				foundRenderer = FindNodeRenderer(*toPointer);
				if (foundRenderer)
					to = foundRenderer->GetMessage();
			}
			else {
				message->FindPointer("Node::to",(void **)&to);
				message->FindPoint("Node::from",fromPointer);
				foundRenderer = FindNodeRenderer(*fromPointer);
				if (foundRenderer)
					from  = foundRenderer->GetMessage();
			}
			data->AddString("Name","Unbenannt");
			if (to != NULL && from!=NULL) {
				connection->AddPointer("Node::from",from);
				connection->AddPointer("Node::to",to);
				connection->AddMessage("Node::Data",data);
				connection->AddInt8("Node::type",1);
				
				connection->AddPointer("ProjectConceptor::doc",doc);
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
		case G_E_NEW_SCALE: {
			//reset to our 100%
			SetScale((1.0/scale));
			message->FindFloat("scale",&scale);
			//now we can set the new scale
			SetScale(scale);
			PushState();
			//FrameResized(0,0);
			UpdateScrollBars();
			Invalidate();
			break;
		}
		case G_E_GRID_CHANGED: {
			gridEnabled =! gridEnabled;
			Invalidate();
			break;
		}
		case G_E_COLOR_CHANGED: {
			rgb_color	tmpNewColor =	colorItem->GetColor();;
			BMessage	*changeColorMessage	= new BMessage(P_C_EXECUTE_COMMAND);
			changeColorMessage->AddString("Command::Name","ChangeValue");
			changeColorMessage->AddBool("Node::selected",true);
			BMessage	*valueContainer	= new BMessage();
			valueContainer->AddString("name","FillColor");
			valueContainer->AddString("subgroup","Node::Pattern");
			valueContainer->AddInt32("type",B_INT32_TYPE);
			valueContainer->AddInt32("newValue",*(int32 *)&tmpNewColor);
			changeColorMessage->AddMessage("valueContainer",valueContainer);
			sentTo->SendMessage(changeColorMessage);
			break;
		}
		case G_E_PEN_SIZE_CHANGED: {
			BMessage	*changePenSizeMessage	= new BMessage(P_C_EXECUTE_COMMAND);
			changePenSizeMessage->AddString("Command::Name","ChangeValue");
			changePenSizeMessage->AddBool("Node::selected",true);
			BMessage	*valueContainer	= new BMessage();
			valueContainer->AddString("name","PenSize");
			valueContainer->AddString("subgroup","Node::Pattern");
			valueContainer->AddInt32("type",B_FLOAT_TYPE);
			valueContainer->AddFloat("newValue",penSize->GetValue());
			changePenSizeMessage->AddMessage("valueContainer",valueContainer);
			sentTo->SendMessage(changePenSizeMessage);
			break;
		}
		case G_E_ADD_ATTRIBUTE: {
			int32	type;
			BString datadummy	= BString("   ");
			message->FindInt32("type",&type);
			InputRequest	*inputAlert = new InputRequest(B_TRANSLATE("Input attributname"),B_TRANSLATE("Name"), B_TRANSLATE("Attribut"), B_TRANSLATE("OK"),B_TRANSLATE("Cancel"));
			char			*input		= NULL;
			char			*inputstr	= NULL;
			if (inputAlert->Go(&input)<1) {
				inputstr	= new char[strlen(input)+1];
				strcpy(inputstr,input);
				BMessage	*addMessage		= new BMessage(P_C_EXECUTE_COMMAND);
				addMessage->AddString("Command::Name","AddAttribute");
				addMessage->AddBool("Node::selected",true);
				BMessage	*valueContainer	= new BMessage();

				valueContainer->AddInt32("type",B_MESSAGE_TYPE);
				valueContainer->AddString("name",inputstr);
				valueContainer->AddString("subgroup","Node::Data");
				BMessage	*newAttribute	= new BMessage(type);
				newAttribute->AddString("Name",inputstr);
				if (type == B_STRING_TYPE)
					newAttribute->AddData("Value",type,datadummy.String(), datadummy.Length() + 1,false);
				else if (type == B_BOOL_TYPE)
					newAttribute->AddBool("Value",true);
				valueContainer->AddMessage("newAttribute",newAttribute);
				addMessage->AddMessage("valueContainer",valueContainer);
				sentTo->SendMessage(addMessage);
			}
			break;
		}
		case G_E_INSERT_NODE: {
			BMessage *generatedInsertCommand = GenerateInsertCommand(P_C_CLASS_TYPE);
			if (generatedInsertCommand)
	            sentTo->SendMessage(generatedInsertCommand);
			break;
		}
		case G_E_INSERT_SIBLING: {
			BMessage *generatedInsertCommand = GenerateInsertCommand(P_C_CLASS_TYPE);
			if (generatedInsertCommand)
				sentTo->SendMessage(generatedInsertCommand);
			break;
		}
		case G_E_INVALIDATE: {
			Invalidate();
			break;
		}
		case G_E_GROUP: {
			BMessage	*commandMessage	= GenerateInsertCommand(P_C_GROUP_TYPE);
			if (commandMessage) {
				commandMessage->ReplaceString("Command::Name","Group");
				sentTo->SendMessage(commandMessage);
			}
			break;
		}
		default:
			BView::MessageReceived(message);
			break;
	}
}

void GraphEditor::FrameResized(float width, float height) {
	TRACE();
	UpdateScrollBars();
}


void GraphEditor::InsertObject(BPoint where,bool deselect) {
	TRACE();
	BMessage	*commandMessage	= new BMessage(P_C_EXECUTE_COMMAND);
	BMessage	*newObject		= new BMessage(*nodeMessage);
	BMessage	*newFont		= new BMessage(*fontMessage);
	BMessage	*newPattern		= new BMessage(*patternMessage);
	rgb_color	tmpNewColor =	colorItem->GetColor();;

	newPattern->ReplaceInt32("FillColor",*(int32 *)&tmpNewColor);
	newPattern->ReplaceFloat("PenSize",penSize->GetValue());

	BMessage	*selectMessage	= new BMessage();
	if (GridEnabled()) {
		where.x=where.x-fmod(where.x,GridWidth());
		where.y=where.y-fmod(where.y,GridWidth());
	}

	newObject->AddRect("Node::frame",BRect(where,where+BPoint(100,40)));
	newObject->AddMessage("Node::Font",newFont);
	newObject->AddMessage("Node::Pattern",newPattern);
	//preparing CommandMessage
	commandMessage->AddPointer("node",(void *)newObject);
	commandMessage->AddString("Command::Name","Insert");
	selectMessage->AddBool("deselect",deselect);
	selectMessage->AddPointer("node",(void *)newObject);
	selectMessage->AddString("Command::Name","Select");
	commandMessage->AddMessage("PCommand::subPCommand",selectMessage);
	sentTo->SendMessage(commandMessage);
}

void GraphEditor::InsertRenderObject(BMessage *node) {
	TRACE();
	AddRenderer(CreateRendererFor(node));
}

Renderer* GraphEditor::CreateRendererFor(BMessage *node)
{
	void	*tmpDoc	= NULL;
	Renderer *newRenderer = NULL;
	if (node->FindPointer("ProjectConceptor::doc",&tmpDoc)==B_OK)
		node->ReplacePointer("ProjectConceptor::doc",doc);
	else
		node->AddPointer("ProjectConceptor::doc",doc);
	switch(node->what) {
		case P_C_CLASS_TYPE:
			newRenderer	= new  ClassRenderer(this,node);
		break;
		case P_C_GROUP_TYPE:
			newRenderer = new GroupRenderer(this,node);
		break;
		case P_C_CONNECTION_TYPE:
			newRenderer	= new ConnectionRenderer(this,node);
		break;
	}
	node->AddPointer(renderString,newRenderer);
	return newRenderer;
}

void GraphEditor::AddRenderer(Renderer* newRenderer) {
	TRACE();
	//if (BView::LockLooper()==B_OK){
	    renderer->AddItem(newRenderer);
	    BringToFront(newRenderer);
	    activRenderer = newRenderer;
	  //  BView::UnlockLooper();
	//}
}

void GraphEditor::RemoveRenderer(Renderer *wichRenderer) {
	TRACE();
	//** Find the Node for the Renderer check if it has a parent... then remove the renderer also from the nodeList of the Groupparent
	if (wichRenderer != NULL) {
		if (activRenderer == wichRenderer)
			activRenderer = NULL;
		if (mouseReciver == wichRenderer)
			mouseReciver = NULL;
		if (wichRenderer->GetMessage())
			(wichRenderer->GetMessage())->RemoveName(renderString);
		//check if this rendere belongs to the grapheditor or if not it belongs to a group so the group will take care to remove the renderer from its renderlist
		ClassRenderer* cRenderer = dynamic_cast <ClassRenderer*>(wichRenderer);
		if (cRenderer != NULL) {
			GroupRenderer* gRenderer = (GroupRenderer*)FindRenderer(cRenderer->Parent()) ;
			if (gRenderer != NULL)
				gRenderer->RemoveRenderer(wichRenderer);
		}
		if (renderer->HasItem(wichRenderer)){	
			renderer->RemoveItem(wichRenderer);
			delete wichRenderer;
			wichRenderer=NULL;
		}
	}
/*	delete rendersensitv;
	rendersensitv = new BRegion();
	renderer->DoForEach(ProceedRegion,rendersensitv);*/
	//**recalc Region
}

Renderer* GraphEditor::FindRenderer(BPoint where) {
	TRACE();
	Renderer *currentRenderer = NULL;
/*	if (rendersensitv->Contains(where))
	{*/
		bool found	=	false;
		for (int32 i=(renderer->CountItems()-1);((!found) && (i>=0));i--) {
			if (((Renderer*)renderer->ItemAt(i))->Caught(where)) {
				found			= true;
				currentRenderer	= (Renderer*)renderer->ItemAt(i);
			}
		}
//	}
	return currentRenderer;
}

Renderer* GraphEditor::FindNodeRenderer(BPoint where) {
	TRACE();
	Renderer *currentRenderer = NULL;
/*	if (rendersensitv->Contains(where))
	{*/
		bool found	=	false;
		for (int32 i=(renderer->CountItems()-1);((!found) && (i>=0));i--) {
			if (((Renderer*)renderer->ItemAt(i))->Caught(where)) {
				currentRenderer	= (Renderer*)renderer->ItemAt(i);
				if ((currentRenderer->GetMessage()->what == P_C_CLASS_TYPE) || (currentRenderer->GetMessage()->what == P_C_GROUP_TYPE) )
					found			= true;
			}
		}
//	}
	return currentRenderer;
}

Renderer* GraphEditor::FindConnectionRenderer(BPoint where) {
	TRACE();
	Renderer *currentRenderer = NULL;
/*	if (rendersensitv->Contains(where))
	{*/
		bool found	=	false;
		for (int32 i=(renderer->CountItems()-1);((!found) && (i>=0));i--) {
			if (((Renderer*)renderer->ItemAt(i))->Caught(where)) {
				currentRenderer	= (Renderer*)renderer->ItemAt(i);
				if (currentRenderer->GetMessage()->what == P_C_CONNECTION_TYPE)
					found			= true;
			}
		}
//	}
	return currentRenderer;
}

Renderer* GraphEditor::FindRenderer(BMessage *container) {
	Renderer	*currentRenderer	= NULL;
	if (container != NULL)
		if ( (container->FindPointer(renderString,(void **) &currentRenderer) == B_OK) 
			&& (currentRenderer) && renderer->HasItem(currentRenderer) )
			return currentRenderer;
		else
			return NULL;
	else
		return NULL;
}


void GraphEditor::BringToFront(Renderer *wichRenderer) {
	TRACE();
	BMessage	*parentNode		= NULL;
	BMessage	*tmpMessage		= NULL;
	BList		*groupAllNodeList	= new BList();
	Renderer	*tmpRenderer		= NULL;
	if (wichRenderer!=NULL)
		if ((tmpMessage = wichRenderer->GetMessage()) != NULL) {
	    /*if ((tmpMessage->FindPointer("Node::parent", (void **)&parentNode) == B_OK) && (parentNode != NULL) ) {
			parentNode->FindPointer(renderString,(void **)&tmpRenderer);
			if (tmpRenderer)
				((GroupRenderer *)tmpRenderer)->BringToFront(wichRenderer);
	    }*/
		DeleteFromList(wichRenderer);
		AddToList(wichRenderer,renderer->CountItems()+1);
		Invalidate();
		}
   }


void GraphEditor::SendToBack(Renderer *wichRenderer) {
	TRACE();
	BMessage	*parentNode	= NULL;
	Renderer	*painter	= NULL;
	int32		i			= 0;
	if (wichRenderer->GetMessage()->FindPointer("Node::parent",(void **) &parentNode)==B_OK) {
		painter	= FindRenderer(parentNode);
		if (painter)
			((GroupRenderer *)painter)->SendToBack(wichRenderer);
		i = renderer->IndexOf(painter);
	}
	renderer->RemoveItem(wichRenderer);
	renderer->AddItem(wichRenderer,i);
	/*TRACE();
	BMessage	*parentNode	= NULL;
	Renderer	*painter	= NULL;
	int32		i			= -1;
	renderer->RemoveItem(wichRenderer);
	if (wichRenderer->GetMessage()->FindPointer("Node::parent",(void **) &parentNode)==B_OK)
	{
		painter	= FindRenderer(parentNode);
		if (painter)
			((GroupRenderer *)tmpRenderer)->SendToBack(wichRenderer);
		i = renderer->IndexOf(painter);
		parentNode->FindPointer(renderString,(void **)&tmpRenderer);
		renderer->AddItem(wichRenderer,i+1);
	}
	else
		renderer->AddItem(wichRenderer,0);
	//**draw all wich are under the Thing redraw*/
	Invalidate();
}

BMessage *GraphEditor::GenerateInsertCommand(uint32 newWhat, bool connected)
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
	rgb_color	tmpColor			=colorItem->GetColor();
	BPoint      where;
	

    data->AddString("Name","Unbenannt");
    //insert new Node here*/
	newPattern->ReplaceInt32("FillColor",*(int32 *)&tmpColor);
	newPattern->ReplaceFloat("PenSize",penSize->GetValue());
    //** we need a good algorithm to find the best rect for this new node we just put it at 100,100**/
	where = BPoint(100,100);
	if (GridEnabled())
	{
		where.x = where.x-fmod(where.x,GridWidth());
		where.y = where.y-fmod(where.y,GridWidth());
	}
	newNode->what = newWhat;
	newNode->AddMessage("Node::Font",newFont);
	newNode->AddMessage("Node::Pattern",newPattern);
	commandMessage->AddString("Command::Name","Insert");
    subCommandMessage->AddString("Command::Name","Select");
	commandMessage->AddPointer("node",newNode);
    subCommandMessage->AddPointer("node",newNode);

	//if (connected == true){
    	while (i<selected->CountItems())
		{
			from	= (BMessage *)selected->ItemAt(i);
       		if (to != NULL && from!=NULL)
       	 	{
				connection		    = new BMessage(P_C_CONNECTION_TYPE);
            	err = from->FindRect("Node::frame",fromRect);
            	if (!selectRect)
                	selectRect = new BRect(*fromRect);
            	else
                	*selectRect = *selectRect | *fromRect;
            	err = B_OK;
            	connection->AddPointer("Node::from",from);
            	connection->AddPointer("Node::to",to);
            	uint	cType	= 1;
            	connection->AddInt8("Node::type", cType);

            	connection->AddMessage("Node::Data",data);
            
            	connection->AddPointer("ProjectConceptor::doc",doc);
            	//** add the connections to the Nodes :-)
            	commandMessage->AddPointer("node",connection);
        	}
			i++;
		}
		if (selectRect)
		{
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
			newNode->AddRect("Node::frame",BRect(where,where+BPoint(100,80)));
			commandMessage->AddMessage("PCommand::subPCommand",subCommandMessage);
		}
	//}
	return commandMessage;
}


bool GraphEditor::DrawRenderer(void *arg,void *editor)
{
	Renderer	*painter	=(Renderer *)arg;
	GraphEditor	*gEditor	= (GraphEditor*)editor;
	BRegion		region;
	gEditor->GetClippingRegion(&region);
	painter->Draw(gEditor,region.Frame());
	return false;
}

bool GraphEditor::ProceedRegion(void *arg,void *region)
{
	TRACE();
	Renderer *painter = (Renderer *)arg;
	((BRegion *)region)->Include(painter->Frame());
	return false;
}

void GraphEditor::DeleteFromList(Renderer *whichRenderer)
{
	BList		*connectionList	= NULL;
	int32		i				= 0;
	BMessage	*tmpNode		= NULL;
	if (renderer->HasItem(whichRenderer) == true) {
		renderer->RemoveItem(whichRenderer);
		//remove all Connections wich belongs to this node..
		if (whichRenderer->GetMessage()->FindPointer("Node::incoming",(void **)&connectionList) == B_OK) {
			for (i = 0; i< connectionList->CountItems();i++) {
				tmpNode = (BMessage *)connectionList->ItemAt(i);
				renderer->RemoveItem(FindRenderer(tmpNode));
			}
		}
		if (whichRenderer->GetMessage()->FindPointer("Node::outgoing",(void **)&connectionList) == B_OK) {
			for (i = 0; i< connectionList->CountItems();i++) {
				tmpNode = (BMessage *)connectionList->ItemAt(i);
				renderer->RemoveItem(FindRenderer(tmpNode));
			}
		}
		// if the deleted Rendere is a list we need to delete all subrendere too
		if (whichRenderer->GetMessage()->what == P_C_GROUP_TYPE) {
			GroupRenderer	*groupPainter	= dynamic_cast <GroupRenderer *>(whichRenderer);
			if (groupPainter != NULL){		
				int32 gcount= groupPainter->RenderList()->CountItems();
				for (int32 i = 0; i<gcount;i++)
					DeleteFromList((Renderer *)groupPainter->RenderList()->ItemAt(i));
			}
		}
	}
}

void GraphEditor::AddToList(Renderer *whichRenderer, int32 pos) {
	BList		*connectionList	= NULL;
	int32		i				= 0;
	BMessage	*tmpNode		= NULL;
	Renderer	*tmpRenderer	= NULL;
	if (pos>renderer->CountItems()) {
		if (whichRenderer->GetMessage()->FindPointer("Node::incoming",(void **)&connectionList) == B_OK) {
			for (i = 0; i< connectionList->CountItems();i++) {
				tmpNode = (BMessage *)connectionList->ItemAt(i);
				if (tmpNode->FindPointer(renderString,(void **) &tmpRenderer) == B_OK) {
					renderer->AddItem(tmpRenderer);
					pos++;
				}
			}
		}
		if (whichRenderer->GetMessage()->FindPointer("Node::outgoing",(void **)&connectionList) == B_OK) {
			for (i = 0; i< connectionList->CountItems();i++) {
				tmpNode = (BMessage *)connectionList->ItemAt(i);
				if (tmpNode->FindPointer(renderString,(void **) &tmpRenderer) == B_OK) {
					renderer->AddItem(tmpRenderer);
					pos++;
				}
			}
		}
		renderer->AddItem(whichRenderer);
	}
	else {
		if (whichRenderer->GetMessage()->FindPointer("Node::incoming",(void **)&connectionList) == B_OK) {
			for (i = 0; i< connectionList->CountItems();i++) {
				tmpNode = (BMessage *)connectionList->ItemAt(i);
				if (FindRenderer(tmpNode)){
					renderer->AddItem(FindRenderer(tmpNode),pos);
					pos++;
				}
			}
		}
		if (whichRenderer->GetMessage()->FindPointer("Node::outgoing",(void **)&connectionList) == B_OK) {
			for (i = 0; i< connectionList->CountItems();i++) {
				tmpNode = (BMessage *)connectionList->ItemAt(i);
				if (FindRenderer(tmpNode)) {
					renderer->AddItem(FindRenderer(tmpNode),pos);
					pos++;
				}
			}
		}		
		renderer->AddItem(whichRenderer,pos);
	}
	if (whichRenderer->GetMessage()->what == P_C_GROUP_TYPE) {
		//should we dynamic cast this??
		GroupRenderer	*groupPainter	= (GroupRenderer *)whichRenderer;
		for (int32 i = 0; i<groupPainter->RenderList()->CountItems();i++)
				AddToList((Renderer *)groupPainter->RenderList()->ItemAt(i),pos+1);
	}
}

void GraphEditor::UpdateScrollBars()
{
	if (doc != NULL)
	{
		BRect		docRect		= doc->Bounds();
		BRect		scrollRect	= myScrollParent->Bounds();
		if ((myScrollParent) && (doc))
		{
			float heightDiff	= docRect.Height()-scrollRect.Height();
			float widthDiff		= docRect.Width()-scrollRect.Width();
			float docWidth		= docRect.Width()*scale;
			float docHeight		= docRect.Height()*scale;
			if (widthDiff<0)
				widthDiff = 0;
			if (heightDiff<0)
					heightDiff = 0;					
			BScrollBar	*sb	= myScrollParent->ScrollBar(B_HORIZONTAL);
			sb->SetRange(docRect.left,widthDiff*scale);
			sb->SetProportion(scrollRect.Width()/docWidth);
			// Steps are 1/8 visible window for small steps
			//   and 1/2 visible window for large steps
			sb->SetSteps(docWidth / 8.0, docWidth / 2.0);
	
			sb	= myScrollParent->ScrollBar(B_VERTICAL);
			sb->SetRange(docRect.top,heightDiff*scale);
			sb->SetProportion(scrollRect.Height()/docHeight);
			sb->SetSteps(docHeight / 8.0, docHeight / 2.0);
		}
	}

}

void GraphEditor::SetShortCutFilter(ShortCutFilter *_shortCutFilter)
{
	if (LockLooper())
	{
			AddFilter(_shortCutFilter);
			UnlockLooper();
	}
}
