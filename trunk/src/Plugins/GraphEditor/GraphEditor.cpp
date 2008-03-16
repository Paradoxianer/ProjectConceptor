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

GraphEditor::GraphEditor(image_id newId):PEditor(),BViewSplitter(BRect(0,0,400,400),B_VERTICAL,B_FOLLOW_ALL_SIDES,0)
{
	TRACE();
	pluginID	= newId;
	Init();

	/*
	BView::SetDoubleBuffering(1);
	SetDrawingMode(B_OP_ALPHA);*/
}

void GraphEditor::Init(void)
{
	TRACE();
	nodeEditor		= NULL;
	showMessage		= NULL;
	renderString	= new char[30];
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
	patternMessage->AddData("Node::Pattern",B_PATTERN_TYPE,(const void *)&B_SOLID_HIGH,sizeof(B_SOLID_HIGH));

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
	patternItem	= new PatternToolItem(_T("Node::Pattern"),B_SOLID_HIGH, new BMessage(G_E_PATTERN_CHANGED));

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

	//sentTo				= new BMessenger(doc);
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
	shortcuts->AddPointer("handler",new BMessenger(nodeEditor,NULL,&err));

	BMessage	*insertNode = new BMessage(G_E_INSERT_NODE);
	shortcuts->AddInt32("key",B_INSERT);
	shortcuts->AddInt32("modifiers",0);
	shortcuts->AddMessage("message",insertNode);
	shortcuts->AddPointer("handler",new BMessenger(nodeEditor,NULL,&err));

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

BView* GraphEditor::GetView(void)
{
	return this;
}


BList* GraphEditor::GetPCommandList(void)
{
	TRACE();
	//at the Moment we dont support special Commands :-)
	return NULL;
}
void GraphEditor::InitAll()
{
	if (showMessage)
		showMessage->SetDoc(doc);
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

void GraphEditor::PreprocessAfterLoad(BMessage *container)
{
	//**nothing to do jet as i know
	//container=container; outcomment from stargater
}

void GraphEditor::SetShortCutFilter(ShortCutFilter *_shortCutFilter)
{
	if (nodeEditor->LockLooper())
	{
			nodeEditor->AddFilter(_shortCutFilter);
			nodeEditor->UnlockLooper();
	}
}

void GraphEditor::ValueChanged()
{
	TRACE();
	BList		*selected		= doc->GetSelected();
	nodeEditor->ValueChanged();
	//showMessage->ValueChanged();
	if (selected->CountItems() == 1)
		showMessage->SetContainer((BMessage*)selected->FirstItem());
}

void GraphEditor::SetDirty(BRegion *region)
{
	TRACE();
	BView::Invalidate(region);
}


void GraphEditor::AttachedToWindow(void)
{
	TRACE();
	BRect		rect(0,0,400,400);
	if (Parent())
	{
		rect = Parent()->Bounds();
		rect.InsetBy(2,2);
		MoveTo(rect.left,rect.top);
		ResizeTo(rect.Width(),rect.Height());
	}
	if (nodeEditor == NULL)
	{
		rect.InsetBy(2,2);
		rect.bottom = rect.bottom-B_H_SCROLL_BAR_HEIGHT;
		rect.right -= (B_V_SCROLL_BAR_WIDTH);
		nodeEditor	= new NodeEditor(this,rect); 
		AddChild(new BScrollView("NodeScroller",nodeEditor,B_FOLLOW_ALL_SIDES,0 ,true,true));
	}
	if (showMessage == NULL)
	{
		//rect.right -= (B_V_SCROLL_BAR_WIDTH);
		rect.bottom = rect.bottom+B_H_SCROLL_BAR_HEIGHT;
		rect.right = (rect.Width()/2.1)-B_V_SCROLL_BAR_WIDTH;
		showMessage	= new MessageListView(doc,rect,NULL);
		BMessage *invoked 			= new BMessage(G_E_INVOKATION);
		invoked->AddPointer("ListView",showMessage);
		showMessage->SetInvocationMessage(invoked);
		showMessage->SetTarget(this);	
		AddChild(new BScrollView("MessageScroller",showMessage,B_FOLLOW_TOP_BOTTOM | B_FOLLOW_RIGHT,0 ,false,true));
		SetDivPos(0,Parent()->Bounds().Width()*0.7);
	}

	PWindow 	*pWindow	= (PWindow *)Window();
	BMenuBar	*menuBar	= (BMenuBar *)pWindow->FindView(P_M_STATUS_BAR);
	menuBar->AddItem(scaleMenu);
	scaleMenu->SetTargetForItems(nodeEditor);
	if (doc)
	{
		InitAll();
		showMessage->ValueChanged();
		nodeEditor->ValueChanged();
	}
	toolBar->ResizeTo(30,pWindow->P_M_MAIN_VIEW_BOTTOM-pWindow->P_M_MAIN_VIEW_TOP);
	pWindow->AddToolBar(toolBar);
	addGroup->SetTarget(nodeEditor);
	addBool->SetTarget(nodeEditor);
	addText->SetTarget(nodeEditor);
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

	grid->SetTarget(nodeEditor);
	penSize->SetTarget(nodeEditor);
	colorItem->SetTarget(nodeEditor);
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
			nodeEditor->UpdateScrollBars();
			//** send infos to nodeEditor
			break;
		}
		default:
			BViewSplitter::MessageReceived(message);
			break;
	}
}
