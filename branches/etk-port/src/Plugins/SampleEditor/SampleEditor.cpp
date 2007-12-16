#include <support/List.h>
#include <interface/Font.h>
#include <interface/ScrollView.h>
#include <interface/GraphicsDefs.h>
#include <translation/TranslationUtils.h>
#include <translation/TranslatorFormats.h>

#include <string.h>
#include "SampleEditor.h"
#include "PCommandManager.h"
#include "Renderer.h"
#include "ClassRenderer.h"
#include "ConnectionRenderer.h"

#include "ToolBar.h"

SampleEditor::SampleEditor():PEditor(),BView(BRect(0,0,200,200),"SampleEditor",B_FOLLOW_ALL_SIDES,B_WILL_DRAW |B_NAVIGABLE|B_NAVIGABLE_JUMP|B_FRAME_EVENTS)
{
	TRACE();
	Init();
	BView::SetDoubleBuffering(1);
	SetDrawingMode(B_OP_ALPHA);
}

void SampleEditor::Init(void)
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
	nodeMessage->AddMessage("Node::Data",dataMessage);
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
	patternMessage->AddData("Node::Pattern",B_PATTERN_TYPE,(const void *)&B_SOLID_HIGH,sizeof(B_SOLID_HIGH));
	
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
	
	colorItem	= new ColorToolItem(_T("Fill"),fillColor,new BMessage(B_E_COLOR_CHANGED));
}

void SampleEditor::AttachedToManager(void)
{
	TRACE();
	sentTo				= new BMessenger(doc);
	id					= manager->IndexOf(this);
	sprintf(renderString,"SampleEditor%ld::Renderer",id);
	//put this in a seperate function??
	nodeMessage->AddPointer("ProjectConceptor::doc",doc);
	
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

void SampleEditor::DetachedFromManager(void)
{
	TRACE();
}

BList* SampleEditor::GetPCommandList(void)
{
	TRACE();
	//at the Moment we dont support special Commands :-)
	return NULL;
}

void SampleEditor::PreprocessBeforSave(BMessage *container)
{
	TRACE();
	char	*name; 
	uint32	type; 
	int32	count; 
	int32	i		= 0;
	//remove all the Pointer to the Renderer so that on the next load a new Renderer are added
	while (container->GetInfo(B_POINTER_TYPE,i ,(const char **)&name, &type, &count) == B_OK)
	{
		if ((strstr(name,"SampleEditor") != NULL) || 
			(strcasecmp(name,"Node::outgoing") == B_OK) ||
			(strcasecmp(name,"Node::incoming") == B_OK) ||
			(strcasecmp(name,"ProjectConceptor::doc") == B_OK) )
		{
			container->RemoveName(name);
			i--;
		}
		i++;
	}
}

void SampleEditor::PreprocessAfterLoad(BMessage *container)
{
	//**nothing to do jet as i know
	container=container;
}

void SampleEditor::InitAll()
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

void SampleEditor::ValueChanged()
{
	TRACE();
	BList		*changedNodes	= doc->GetChangedNodes();
//	BList		*allTrashed		= doc->GetTrash();

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
/*	for (int32 i=0;i<allTrashed->CountItems();i++)
	{
		node = (BMessage *)allTrashed->ItemAt(i);
//		DeleteRenderObject(node);
		RemoveRenderer(FindRenderer(node));
	}*/
	if (BView::LockLooper())
	{
		Invalidate();
		BView::UnlockLooper();
	}
}

void SampleEditor::SetDirty(BRegion *region)
{
	TRACE();
	BView::Invalidate(region);
}


void SampleEditor::Draw(BRect updateRect)
{
	SetHighColor(230,230,230,255);
	BView::Draw(BRect(updateRect.left/scale,updateRect.top/scale,updateRect.right/scale,updateRect.bottom/scale));
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

void SampleEditor::MouseDown(BPoint where)
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


void SampleEditor::MouseMoved(	BPoint where, uint32 code, const BMessage *a_message)
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

void SampleEditor::MouseUp(BPoint where)
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
	else if (mouseReciver != NULL)
	{
		mouseReciver->MouseUp(scaledWhere);
	}

}

void SampleEditor::KeyDown(const char *bytes, int32 numBytes)
{
	TRACE();
}

void SampleEditor::KeyUp(const char *bytes, int32 numBytes)
{
	TRACE();
}
void SampleEditor::AttachedToWindow(void)
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

	toolBar->AddItem(colorItem);
	colorItem->SetTarget(this);
	
}



void SampleEditor::DetachedFromWindow(void)
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
			toolBar->RemoveItem(colorItem);
			toolBar->RemoveSeperator();
			toolBar->RemoveSeperator();
			toolBar->RemoveSeperator();	
			toolBar->RemoveSeperator();
			toolBar->RemoveSeperator();	
		}
		Renderer	*nodeRenderer	= NULL;
		
		while(renderer->CountItems()>0)
		{
			nodeRenderer = (Renderer *)renderer->ItemAt(0);
			RemoveRenderer(nodeRenderer);
		}
	}
}	
void SampleEditor::MessageReceived(BMessage *message)
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
				message->FindPoint("Node::to",toPoint);
				message->FindPoint("Node::from",fromPoint);
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
			if (message->FindPointer("Node::from",(void **)&from) == B_OK)
			{
				message->FindPoint("Node::to",toPointer);
				foundRenderer = FindNodeRenderer(*toPointer);
				if (foundRenderer)
					to = foundRenderer->GetMessage();
				//to = doc->FindObject(toPointer);
			}
			else
			{
				message->FindPointer("Node::to",(void **)&to);
				message->FindPoint("Node::from",fromPointer);
				foundRenderer = FindNodeRenderer(*fromPointer);
				if (foundRenderer)
					from  = foundRenderer->GetMessage();
			//	from	= doc->FindObject(fromPointer);
			}
			data->AddString("Name","Unbenannt");
			if (to != NULL && from!=NULL)
			{
				connection->AddPointer("Node::from",from);
				connection->AddPointer("Node::to",to);
				connection->AddMessage("Node::Data",data);
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
		case B_E_NEW_SCALE:
		{
			message->FindFloat("scale",&scale);
			SetScale(scale);
			FrameResized(0,0);
			Invalidate();
			break;
		}
		case B_E_COLOR_CHANGED:
		{
			BMessage	*changeColorMessage	= new BMessage(P_C_EXECUTE_COMMAND);
			changeColorMessage->AddString("Command::Name","ChangeValue");
			changeColorMessage->AddBool("Node::selected",true);
			changeColorMessage->AddString("name","FillColor");
			changeColorMessage->AddString("subgroup","Node::Pattern");
			changeColorMessage->AddInt32("type",B_RGB_COLOR_TYPE);
			changeColorMessage->AddRGBColor("newValue",colorItem->GetColor()); 
			sentTo->SendMessage(changeColorMessage);
			break;
		}
		default:
			BView::MessageReceived(message);
			break;
	}
}

void SampleEditor::FrameResized(float width, float height)
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

void SampleEditor::InsertObject(BPoint where,bool deselect)
{
	TRACE();
	BMessage	*commandMessage	= new BMessage(P_C_EXECUTE_COMMAND);
	BMessage	*newObject		= new BMessage(*nodeMessage);
	BMessage	*newFont		= new BMessage(*fontMessage);
	BMessage	*newPattern		= new BMessage(*patternMessage);
	newPattern->ReplaceRGBColor("FillColor",colorItem->GetColor());
	BMessage	*selectMessage	= new BMessage();

		newObject->AddRect("Node::frame",BRect(where,where+BPoint(100,80)));
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

void SampleEditor::InsertRenderObject(BMessage *node)
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
}


void SampleEditor::AddRenderer(Renderer* newRenderer)
{
	TRACE();
	renderer->AddItem(newRenderer);
	activRenderer = newRenderer;
}

void SampleEditor::RemoveRenderer(Renderer *wichRenderer)
{
	TRACE();
	if (activRenderer == wichRenderer)
		activRenderer = NULL;
	if (mouseReciver == wichRenderer)
		mouseReciver = NULL;
	renderer->RemoveItem(wichRenderer);
	(wichRenderer->GetMessage())->RemoveName(renderString);
	delete wichRenderer;
}

Renderer* SampleEditor::FindRenderer(BPoint where)
{
	TRACE();
	Renderer *currentRenderer = NULL;
	bool found	=	false;
	for (int32 i=(renderer->CountItems()-1);((!found) && (i>=0));i--)
	{
		if (((Renderer*)renderer->ItemAt(i))->Caught(where))
		{
			found			= true;
			currentRenderer	= (Renderer*)renderer->ItemAt(i);
		}
	}
	return currentRenderer;
}

Renderer* SampleEditor::FindNodeRenderer(BPoint where)
{
	TRACE();
	Renderer *currentRenderer = NULL;
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
	return currentRenderer;
}

Renderer* SampleEditor::FindConnectionRenderer(BPoint where)
{
	TRACE();
	Renderer *currentRenderer = NULL;
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
	return currentRenderer;
}

Renderer* SampleEditor::FindRenderer(BMessage *container)
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

void SampleEditor::BringToFront(Renderer *wichRenderer)
{
	TRACE();
	renderer->RemoveItem(wichRenderer);
	renderer->AddItem(wichRenderer);
	//**draw the new "onTop" renderer
	Invalidate();
	
}

void SampleEditor::SendToBack(Renderer *wichRenderer)
{
	TRACE();
	renderer->RemoveItem(wichRenderer);
	renderer->AddItem(wichRenderer,0);
	//draw all
	Invalidate();
}

bool SampleEditor::DrawRenderer(void *arg,void *editor)
{
	Renderer *painter=(Renderer *)arg;
	painter->Draw((SampleEditor*)editor,BRect(0,0,0,0));
	return false;
}

bool SampleEditor::ProceedRegion(void *arg,void *region)
{
	TRACE();
	Renderer *painter = (Renderer *)arg;
	((BRegion *)region)->Include(painter->Frame());
	return false;
}

