#include <support/List.h>
#include <interface/Font.h>
#include <interface/ScrollView.h>
#include <interface/GraphicsDefs.h>
#include <translation/TranslationUtils.h>
#include <translation/TranslatorFormats.h>
#include <storage/Resources.h>
#include <support/DataIO.h>

#include <string.h>
#include "NodeEditor.h"
#include "PCommandManager.h"
#include "Renderer.h"
#include "ClassRenderer.h"
#include "ConnectionRenderer.h"
#include "GroupRenderer.h"

#include "InputRequest.h"
#include "GraphEditor.h"


NodeEditor::NodeEditor(GraphEditor *_graphEditor,BRect rect):BView(rect,"NodeEditor",B_FOLLOW_ALL_SIDES,B_WILL_DRAW)
{
	TRACE();
	graphEditor = _graphEditor;
	Init();
#ifdef B_ZETA_VERSION_1_0_0
	BView::SetDoubleBuffering(1);
#endif
	SetDrawingMode(B_OP_ALPHA);
}

void NodeEditor::Init(void)
{
	TRACE();
	printRect		= NULL;
	selectRect		= NULL;
	startMouseDown	= NULL;
	activRenderer	= NULL;
	mouseReciver	= NULL;
	sentTo			= NULL;

	renderString	= new char[30];
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
#ifdef B_ZETA_VERSION_1_0_0
	fontMessage->AddRGBColor("Color",fontColor);
#endif

	//perparing Pattern Message
	patternMessage	=new BMessage();
	//standart Color
	rgb_color	fillColor			= {152, 180, 190, 255};
	rgb_color	borderColor			= {0, 0, 0, 255};
	rgb_color	highColor			= {0, 0, 0, 255};
	rgb_color 	lowColor			= {128, 128, 128, 255};
#ifdef B_ZETA_VERSION_1_0_0
	patternMessage->AddRGBColor("FillColor",fillColor);
	patternMessage->AddRGBColor("BorderColor",borderColor);
	patternMessage->AddRGBColor("HighColor",highColor);
	patternMessage->AddRGBColor("LowColor",lowColor);
#endif
	patternMessage->AddFloat("PenSize",1.0);
	patternMessage->AddInt8("DrawingMode",B_OP_ALPHA);
	patternMessage->AddData("Node::Pattern",B_PATTERN_TYPE,(const void *)&B_SOLID_HIGH,sizeof(B_SOLID_HIGH));
	sprintf(renderString,"NodeEditor::Renderer");

}

void NodeEditor::InitAll()
{
	TRACE();
	PDocument	*doc	= graphEditor->BelongTo();
	//id					= manager->IndexOf(this);
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

void NodeEditor::PreprocessBeforSave(BMessage *container)
{
	TRACE();
	PRINT(("NodeEditor::PreprocessAfterLoad:\n"));
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
		if ((strstr(name,"NodeEditor") != NULL) ||
			(strcasecmp(name,"Node::outgoing") == B_OK) ||
			(strcasecmp(name,"Node::incoming") == B_OK) ||
			(strcasecmp(name,"Parent") == B_OK)  ||
			(strcasecmp(name,"ProjectConceptor::doc") == B_OK) )
		{
			container->RemoveName(name);
			i--;
		}
		i++;
	}
}

void NodeEditor::PreprocessAfterLoad(BMessage *container)
{
	//**nothing to do jet as i know
	container=container;
}

void NodeEditor::ValueChanged()
{
	TRACE();
	PDocument	*doc			= graphEditor->BelongTo();
	BList		*changedNodes	= doc->GetChangedNodes();
	BList		*allNodes		= doc->GetAllNodes();
	BList		*allConnections	= doc->GetAllConnections();
	BList		*selected		= doc->GetSelected();

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
			//**check if this node is in the node or connection list because if it is not it´s a nodd frome a subgroup or it was deleted
			if (((allConnections->HasItem(node))||(allNodes->HasItem(node))) && (node->FindPointer("Parent",&pointer) !=B_OK))
			{
				InsertRenderObject(node);
 				//if we procedded this node than it´s not changed anymore
//				changedNodes->RemoveItem(i);
//				i--;

			}
			else
			{
				/*if (node->FindPointer("Parent",&pointer) == B_OK)
				{
					//we have to find the father ... wich should be invalidatet :( :) maby there is a nother faster way to do this ..
					BMessage	*parent		= (BMessage *)pointer;
					while ( parent->FindPointer("Parent", &pointer)==B_OK)
					{
						parent = (BMessage *)pointer;
					}
					if ((parent!=NULL)&&(parent->FindPointer(renderString,(void **)&painter) == B_OK))
					 painter->ValueChanged();
				}
				else*/
					RemoveRenderer(FindRenderer(node));
				//if we procedded this node than it´s not changed anymore
//				changedNodes->RemoveItem(i);
//				i--;
			}
		}
	}
	/*if (selected->CountItems() == 1)
		showMessage->SetContainer((BMessage*)selected->FirstItem());*/
	if (BView::LockLooper())
	{
		Invalidate();
		BView::UnlockLooper();
	}
}


void NodeEditor::Draw(BRect updateRect)
{
	SetHighColor(230,230,230,255);
	SetScale(1.0);
	BView::Draw(updateRect);
//	BView::Draw(BRect(updateRect.left/scale,updateRect.top/scale,updateRect.right/scale,updateRect.bottom/scale));
	SetScale(scale);
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

void NodeEditor::MouseDown(BPoint where)
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


void NodeEditor::MouseMoved(	BPoint where, uint32 code, const BMessage *a_message)
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

void NodeEditor::MouseUp(BPoint where)
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
		mouseReciver = NULL;
	}
}



void NodeEditor::AttachedToWindow(void)
{
	TRACE();
	status_t	err;
	if (sentTo != NULL)
		delete sentTo;
	//sentTo				= new BMessenger(graphEditor->BelongTo(),NULL,&err);
	sentTo				= new BMessenger(NULL,graphEditor->BelongTo(),&err);
	if (err != B_OK)
		printf("Init BMessagener %s\n",strerror(err));
	SetViewColor(230,230,230,255);
}



void NodeEditor::DetachedFromWindow(void)
{
	TRACE();
	if (Window())
	{
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
void NodeEditor::MessageReceived(BMessage *message)
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
			UpdateScrollBars();
			break;
		}
		case G_E_CONNECTING:
		{
				connecting = true;
				message->FindPoint("Node::to",toPoint);
				message->FindPoint("Node::from",fromPoint);
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
				connection->AddPointer("ProjectConceptor::doc",graphEditor->BelongTo());
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
			//FrameResized(0,0);
			UpdateScrollBars();
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
			changeColorMessage->AddBool("Node::selected",true);
			BMessage	*valueContainer	= new BMessage();
			valueContainer->AddString("name","FillColor");
			valueContainer->AddString("subgroup","Node::Pattern");
			valueContainer->AddInt32("type",B_RGB_COLOR_TYPE);
#ifdef B_ZETA_VERSION_1_0_0
			valueContainer->AddRGBColor("newValue",graphEditor->GetColor());
#endif
			changeColorMessage->AddMessage("valueContainer",valueContainer);
			sentTo->SendMessage(changeColorMessage);
			break;
		}
		case G_E_PEN_SIZE_CHANGED:
		{
			BMessage	*changePenSizeMessage	= new BMessage(P_C_EXECUTE_COMMAND);
			changePenSizeMessage->AddString("Command::Name","ChangeValue");
			changePenSizeMessage->AddBool("Node::selected",true);
			BMessage	*valueContainer	= new BMessage();
			valueContainer->AddString("name","PenSize");
			valueContainer->AddString("subgroup","Node::Pattern");
			valueContainer->AddInt32("type",B_FLOAT_TYPE);
			valueContainer->AddFloat("newValue",graphEditor->GetPenSize());
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
				addMessage->AddBool("Node::selected",true);
				BMessage	*valueContainer	= new BMessage();

				valueContainer->AddInt32("type",B_MESSAGE_TYPE);
				valueContainer->AddString("name",inputstr);
				valueContainer->AddString("subgroup","Node::Data");
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
			BMessage *generatedInsertCommand = GenerateInsertCommand(P_C_CLASS_TYPE);
			if (generatedInsertCommand)
	            sentTo->SendMessage(generatedInsertCommand);
			break;
		}
		case G_E_INSERT_SIBLING:
		{
			BMessage *generatedInsertCommand = GenerateInsertCommand(P_C_CLASS_TYPE);
			if (generatedInsertCommand)
				sentTo->SendMessage(generatedInsertCommand);
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
			if (commandMessage)
			{
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

void NodeEditor::FrameResized(float width, float height)
{
	TRACE();
	UpdateScrollBars();
}


void NodeEditor::InsertObject(BPoint where,bool deselect)
{
	TRACE();
	BMessage	*commandMessage	= new BMessage(P_C_EXECUTE_COMMAND);
	BMessage	*newObject		= new BMessage(*nodeMessage);
	BMessage	*newFont		= new BMessage(*fontMessage);
	BMessage	*newPattern		= new BMessage(*patternMessage);
#ifdef B_ZETA_VERSION_1_0_0
	newPattern->ReplaceRGBColor("FillColor",graphEditor->GetColor());
#endif
	newPattern->ReplaceFloat("PenSize",graphEditor->GetPenSize());

	BMessage	*selectMessage	= new BMessage();
	if (GridEnabled())
	{
		where.x=where.x-fmod(where.x,GridWidth());
		where.y=where.y-fmod(where.y,GridWidth());
	}

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

void NodeEditor::InsertRenderObject(BMessage *node)
{
	TRACE();
	Renderer *newRenderer = NULL;
	void	*tmpDoc	= NULL;
	if (node->FindPointer("ProjectConceptor::doc",&tmpDoc)==B_OK)
		node->ReplacePointer("ProjectConceptor::doc",graphEditor->BelongTo());
	else
		node->AddPointer("ProjectConceptor::doc",graphEditor->BelongTo());
	switch(node->what)
	{
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
	AddRenderer(newRenderer);
}


void NodeEditor::AddRenderer(Renderer* newRenderer)
{
	TRACE();
	renderer->AddItem(newRenderer);
	BringToFront(newRenderer);
	activRenderer = newRenderer;
}

void NodeEditor::RemoveRenderer(Renderer *wichRenderer)
{
	TRACE();
	if (wichRenderer != NULL)
	{
		if (activRenderer == wichRenderer)
			activRenderer = NULL;
		if (mouseReciver == wichRenderer)
			mouseReciver = NULL;
		renderer->RemoveItem(wichRenderer);
		if (wichRenderer->GetMessage())
			(wichRenderer->GetMessage())->RemoveName(renderString);

		delete wichRenderer;
	}
}

Renderer* NodeEditor::FindRenderer(BPoint where)
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

Renderer* NodeEditor::FindNodeRenderer(BPoint where)
{
	TRACE();
	Renderer *currentRenderer = NULL;
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
	return currentRenderer;
}

Renderer* NodeEditor::FindConnectionRenderer(BPoint where)
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

Renderer* NodeEditor::FindRenderer(BMessage *container)
{
	Renderer	*currentRenderer	= NULL;
	if ( (container->FindPointer(renderString,(void **) &currentRenderer) == B_OK) && (currentRenderer) )
		return currentRenderer;
	else
		return NULL;
}


void NodeEditor::BringToFront(Renderer *wichRenderer)
{
	//** need complete rewrite... need to make a sortet list... remove all renderer of the renderlist and then add them in the new sorted way.
	TRACE();
	BMessage	*parentNode			= NULL;
	BList		*tmpRenderList		= new BList();
	BList		*groupAllNodeList	= new BList();
	Renderer	*tmpRenderer		= NULL;
	
	if (wichRenderer->GetMessage()->FindPointer("Node::parent", (void **)&parentNode) == B_OK)
	{
		parentNode->FindPointer(renderString,(void **)&tmpRenderer);
		if (tmpRenderer)
			((GroupRenderer *)tmpRenderer)->BringToFront(wichRenderer);
	}
	DeleteFromList(wichRenderer);
	AddToList(wichRenderer,renderer->CountItems()+2);
	Invalidate();
	
}

void NodeEditor::SendToBack(Renderer *wichRenderer)
{
	BMessage	*parentNode	= NULL;
	Renderer	*painter	= NULL;
	int32		i			= 0;
	if (wichRenderer->GetMessage()->FindPointer("Node::parent",(void **) &parentNode)==B_OK)
	{
		painter	= FindRenderer(parentNode);
		if (painter)
			((GroupRenderer *)painter)->SendToBack(wichRenderer);
		i = renderer->IndexOf(painter);
	}
	renderer->RemoveItem(wichRenderer);
	renderer->AddItem(wichRenderer,i);
	Invalidate();
}

BMessage *NodeEditor::GenerateInsertCommand(uint32 newWhat)
{
	
	BList		*selected			= graphEditor->BelongTo()->GetSelected();
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
#ifdef B_ZETA_VERSION_1_0_0
	newPattern->ReplaceRGBColor("FillColor",graphEditor->GetColor());
#endif
	newPattern->ReplaceFloat("PenSize",graphEditor->GetPenSize());
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
            connection->AddMessage("Node::Data",data);
            connection->AddPointer("ProjectConceptor::doc",graphEditor->BelongTo());
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
	else
		commandMessage = NULL;
	return commandMessage;
}


bool NodeEditor::DrawRenderer(void *arg,void *editor)
{
	Renderer *painter=(Renderer *)arg;
	painter->Draw((NodeEditor*)editor,BRect(0,0,0,0));
	return false;
}


void NodeEditor::DeleteFromList(Renderer *whichRenderer)
{
	BList		*connectionList	= NULL;
	int32		i				= 0;
	BMessage	*tmpNode		= NULL;
	renderer->RemoveItem(whichRenderer);
	//remove all Connections wich belongs to this node.. so that also the connections  are able to come the from
	if (whichRenderer->GetMessage()->FindPointer("Node::incoming",(void **)&connectionList) == B_OK)
	{
		for (i = 0; i< connectionList->CountItems();i++)
		{
			tmpNode = (BMessage *)connectionList->ItemAt(i);
			renderer->RemoveItem(FindRenderer(tmpNode));
		}
	}
	if (whichRenderer->GetMessage()->FindPointer("Node::outgoing",(void **)&connectionList) == B_OK)
	{
		for (i = 0; i< connectionList->CountItems();i++)
		{
			tmpNode = (BMessage *)connectionList->ItemAt(i);
			renderer->RemoveItem(FindRenderer(tmpNode));
		}
	}
	if (whichRenderer->GetMessage()->what == P_C_GROUP_TYPE)
	{
		//should we dynamic cast this??
		GroupRenderer	*groupPainter	= (GroupRenderer *)whichRenderer;
		for (int32 i = 0; i<groupPainter->RenderList()->CountItems();i++)
			DeleteFromList((Renderer *)groupPainter->RenderList()->ItemAt(i));
	}
}
void NodeEditor::AddToList(Renderer *whichRenderer, int32 pos)
{
	BList		*connectionList	= NULL;
	int32		i				= 0;
	BMessage	*tmpNode		= NULL;
	if (pos>renderer->CountItems())
	{
		if (whichRenderer->GetMessage()->FindPointer("Node::incoming",(void **)&connectionList) == B_OK)
		{
			for (i = 0; i< connectionList->CountItems();i++)
			{
				tmpNode = (BMessage *)connectionList->ItemAt(i);
				if (FindRenderer(tmpNode))
				{
					renderer->AddItem(FindRenderer(tmpNode));
					pos++;
				}
			}
		}
		if (whichRenderer->GetMessage()->FindPointer("Node::outgoing",(void **)&connectionList) == B_OK)
		{
			for (i = 0; i< connectionList->CountItems();i++)
			{
				tmpNode = (BMessage *)connectionList->ItemAt(i);
				if (FindRenderer(tmpNode))
				{
					renderer->AddItem(FindRenderer(tmpNode));
					pos++;
				}
			}
		}
		renderer->AddItem(whichRenderer);
	}
	else
	{
		if (whichRenderer->GetMessage()->FindPointer("Node::incoming",(void **)&connectionList) == B_OK)
		{
			for (i = 0; i< connectionList->CountItems();i++)
			{
				tmpNode = (BMessage *)connectionList->ItemAt(i);
				if (FindRenderer(tmpNode))
				{
					renderer->AddItem(FindRenderer(tmpNode),pos);
					pos++;
				}
			}
		}
		if (whichRenderer->GetMessage()->FindPointer("Node::outgoing",(void **)&connectionList) == B_OK)
		{
			for (i = 0; i< connectionList->CountItems();i++)
			{
				tmpNode = (BMessage *)connectionList->ItemAt(i);
				if (FindRenderer(tmpNode))
				{
					renderer->AddItem(FindRenderer(tmpNode),pos);
					pos++;
				}
			}
		}
		renderer->AddItem(whichRenderer,pos);
	}
	if (whichRenderer->GetMessage()->what == P_C_GROUP_TYPE)
	{
		//should we dynamic cast this??
		GroupRenderer	*groupPainter	= (GroupRenderer *)whichRenderer;
		for (int32 i = 0; i<groupPainter->RenderList()->CountItems();i++)
				AddToList((Renderer *)groupPainter->RenderList()->ItemAt(i),pos+1);
	}
}

void NodeEditor::UpdateScrollBars()
{
	if (graphEditor->BelongTo() != NULL)
	{
		BRect		docRect			= graphEditor->BelongTo()->Bounds();
		BScrollView *myScrollParent	= dynamic_cast<BScrollView*> (Parent());
		if (myScrollParent)
		{
			BRect		scrollRect	= myScrollParent->Bounds();
			float docWidth		= docRect.Width()*scale;
			float docHeight		= docRect.Height()*scale;
			float heightDiff	= docWidth-scrollRect.Height();
			float widthDiff		= docHeight-scrollRect.Width();
			if (widthDiff<0)
				widthDiff = 0;
			if (heightDiff < 0)
				heightDiff = 0;					
			
			BScrollBar	*sb	= myScrollParent->ScrollBar(B_HORIZONTAL);
			sb->SetRange(0,widthDiff);
			sb->SetProportion(scrollRect.Width()/docWidth);
			// Steps are 1/8 visible window for small steps
			//   and 1/2 visible window for large steps
			sb->SetSteps(docWidth / 8.0, docWidth / 2.0);
	
			sb	= myScrollParent->ScrollBar(B_VERTICAL);
			sb->SetRange(0,heightDiff);
			sb->SetProportion(scrollRect.Height()/docHeight);
			sb->SetSteps(docHeight / 8.0, docHeight / 2.0);
		}
	}

}

image_id NodeEditor::PluginID(void)
{
	return graphEditor->PluginID();
};
PDocument *NodeEditor::BelongTo(void)
{
	return graphEditor->BelongTo();
};
	
