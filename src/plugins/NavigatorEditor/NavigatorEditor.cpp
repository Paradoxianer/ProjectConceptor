#include "ToolBar.h"
#include "ToolItem.h"
#include "NavigatorEditor.h"
#include "NavigatorCommands.h"
#include "PCommandManager.h"
#include "MessageListView.h"
#include "NodeItem.h"
#include "PEditorManager.h"
#include "PWindow.h"

#include <interface/Bitmap.h>
#include <interface/Font.h>
#include <interface/GraphicsDefs.h>
#include <interface/ScrollView.h>
#include <support/List.h>
#include <translation/TranslationUtils.h>
#include <translation/TranslatorFormats.h>
#include <Catalog.h>
#include <string.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "NavigatorEditor"

// A plain, filled "+" or "-" tile, drawn once instead of shipped as a
// PNG resource. Deliberately just the bare symbol at near-maximum size -
// an earlier version combined the symbol with a letter ("+N"/"+A") to
// tell add-node/add-attribute apart, which nobody could actually read
// at toolbar size. Both buttons fire immediately (add a node / delete
// the selection) rather than opening a menu - picking a field *type* is
// still right-click only, since that inherently needs a choice a single
// toolbar icon can't make for you.
static BBitmap* MakeSymbolIcon(char symbol, rgb_color tint)
{
	BRect	bounds(0,0,19,19);
	BBitmap	*bmp	= new BBitmap(bounds,B_RGBA32,true);
	BView	*view	= new BView(bounds,"symbol",B_FOLLOW_NONE,B_WILL_DRAW);
	bmp->AddChild(view);
	bmp->Lock();
	view->SetHighColor(0,0,0,0);
	view->FillRect(bounds,B_SOLID_HIGH);
	view->SetHighColor(tint);
	view->SetDrawingMode(B_OP_ALPHA);
	view->FillRoundRect(bounds.InsetByCopy(0.5,0.5),4,4);
	view->SetHighColor(255,255,255,255);
	view->SetPenSize(2.5);
	BPoint	center((bounds.left+bounds.right)/2,(bounds.top+bounds.bottom)/2);
	view->StrokeLine(BPoint(center.x-5,center.y),BPoint(center.x+5,center.y));
	if (symbol == '+')
		view->StrokeLine(BPoint(center.x,center.y-5),BPoint(center.x,center.y+5));
	view->Sync();
	bmp->Unlock();
	return bmp;
}


NavigatorEditor::NavigatorEditor():PEditor(),BView(BRect(0,0,200,200),"NavigatorEditor",B_FOLLOW_ALL_SIDES,B_WILL_DRAW|B_NAVIGABLE|B_NAVIGABLE_JUMP)
{
	TRACE();
	Init();
#ifdef __ZETA__
	BView::SetDoubleBuffering(1);
#endif
//	SetDrawingMode(B_OP_ALPHA);
}

void NavigatorEditor::Init(void)
{
	TRACE();
	renderString	= new char[30];
	configMessage 	= new BMessage();
	toolBar			= NULL;
	myScrollParent	= NULL;
	focusedList		= NULL;
	font_family		family;
	font_style		style;
	


/*	BMessage		*dataMessage	= new BMessage();
	dataMessage->AddString(P_C_NODE_NAME,"Untitled");
	//preparing the standart ObjectMessage
	nodeMessage	= new BMessage(P_C_CLASS_TYPE);
	nodeMessage->AddMessage(P_C_NODE_DATA,dataMessage);
	//Preparing the standart FontMessage
	fontMessage		= new BMessage();
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
	patternMessage->AddData("Pattern",B_PATTERN_TYPE,(const void *)&B_SOLID_HIGH,sizeof(B_SOLID_HIGH));*/
}

void NavigatorEditor::InitGraph()
{
		BList		*allNodes		= doc->GetAllNodes();
		BList		*allConnections	= doc->GetAllConnections();
		BRect		rootrect		= Bounds();
		rootrect.right				= 200;
		root						= new NodeListView(rootrect,allNodes,doc,this);
		focusedList					= root;
		BMessage *selected 			= new BMessage(N_A_INVOKATION);
		selected->AddPointer("ListView",root);
		root->SetSelectionMessage(selected);
		root->SetTarget(this);
		AddChild(new BScrollView("root",root,B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM,0,false,true));
		SetViewColor(255,255,255,255);
		UpdateScrollBars();
		Invalidate();
}

BView* NavigatorEditor::GetView(void)
{
	if (myScrollParent)
		return myScrollParent;
	myScrollParent	= new BScrollView("NEScrolly",this,B_FOLLOW_ALL_SIDES,0,true,false);
	return myScrollParent;
}

void NavigatorEditor::UpdateScrollBars(void)
{
	// myScrollParent can already be torn down by the time some deferred
	// update runs during window close - same guard GraphEditor's own
	// UpdateScrollBars() needs for the same reason.
	if (myScrollParent == NULL)
		return;
	BRect		contentRect	= Bounds();
	BRect		viewRect	= myScrollParent->Bounds();
	float		widthDiff	= contentRect.Width()-viewRect.Width();
	if (widthDiff < 0)
		widthDiff = 0;
	BScrollBar	*sb	= myScrollParent->ScrollBar(B_HORIZONTAL);
	if (sb != NULL) {
		sb->SetRange(0,widthDiff);
		sb->SetProportion(viewRect.Width()/contentRect.Width());
		sb->SetSteps(contentRect.Width()/8.0,contentRect.Width()/2.0);
	}
}

void NavigatorEditor::InitToolBar(void)
{
	PWindow	*pWindow	= (PWindow *)Window();

	toolBar			= new ToolBar(BRect(1,1,30,2800),"N_A_TOOL_BAR",B_ITEMS_IN_COLUMN);

	rgb_color	addTint		= {40,150,40,255};
	rgb_color	deleteTint	= {190,60,60,255};

	addItem			= new ToolItem("addItem",
		MakeSymbolIcon('+',addTint),new BMessage(N_A_ADD));
	addItem->BButton::SetToolTip(B_TRANSLATE(
		"Add a node (child of the selection, or top-level)"));
	toolBar->AddItem(addItem);

	deleteItem		= new ToolItem("deleteItem",
		MakeSymbolIcon('-',deleteTint),new BMessage(N_A_DELETE_NODE));
	deleteItem->BButton::SetToolTip(B_TRANSLATE(
		"Delete the selected node or field"));
	toolBar->AddItem(deleteItem);

	toolBar->ResizeTo(30,pWindow->P_M_MAIN_VIEW_BOTTOM-pWindow->P_M_MAIN_VIEW_TOP);
	pWindow->AddToolBar(toolBar);
	addItem->SetTarget(this);
	deleteItem->SetTarget(this);
}

void NavigatorEditor::AttachedToManager(void)
{
	TRACE();
	sentTo			= new BMessenger(doc);
	id				= manager->IndexOf(this);
	sprintf(renderString,"NavigatorEditor%ld::Renderer",id);
	InitGraph();
}

void NavigatorEditor::DetachedFromManager(void)
{
	TRACE();
}

void NavigatorEditor::PreprocessBeforSave(BMessage *container)
{
	TRACE();
	char	*name;
	uint32	type;
	int32	count;
	int32	i		= 0;
	//remove all the Pointer to the Renderer so that on the next load a new Renderer are added
	while (container->GetInfo(B_POINTER_TYPE,i ,(char **)&name, &type, &count) == B_OK)
	{
		if ((strstr(name,"GraphEditor") != NULL) ||
			(strcasecmp(name,P_C_NODE_OUTGOING) == B_OK) ||
			(strcasecmp(name,P_C_NODE_INCOMING) == B_OK) ||
			(strcasecmp(name,P_C_NODE_PARENT) == B_OK)  ||
			(strcasecmp(name,"ProjectConceptor::doc") == B_OK) )
		{
			container->RemoveName(name);
			i--;
		}
		i++;
	}
}

void NavigatorEditor::PreprocessAfterLoad(BMessage *container)
{
	//**nothing to do jet as i know
	container=container;
}

void NavigatorEditor::SetShortCutFilter(ShortCutFilter *_shortCutFilter)
{
	if (LockLooper())
	{
		AddFilter(_shortCutFilter);
		UnlockLooper();
	}
}

BList* NavigatorEditor::GetPCommandList(void)
{
	TRACE();
	//at the Moment we dont support special Commands :-)
	return NULL;
}


void NavigatorEditor::ValueChanged(BMessage *changedNodes)
{
	TRACE();
	BView 			*child 				= NULL;
	NodeListView	*nodeListView		= NULL;
	MessageListView	*messageListView	= NULL;
	root->ValueChanged();
	if ( child = ChildAt(1) )
	{
		while ( child )
		{
			messageListView = dynamic_cast<MessageListView *>(((BScrollView *)child)->Target());
			if (messageListView)
				messageListView->ValueChanged();
			child = child->NextSibling();
		}
	} 
}


void NavigatorEditor::AttachedToWindow(void)
{
	TRACE();
	InitToolBar();
	//put this in a seperate function??
	if (doc)
		InitGraph();
}

void NavigatorEditor::FrameResized(float width, float height)
{
	TRACE();
	UpdateScrollBars();
}

void NavigatorEditor::DetachedFromWindow(void)
{
	TRACE();
	// The tab view detaches/reattaches this editor's BView on every tab
	// switch (not just once at document close) - GraphEditor's own
	// AttachedToWindow/DetachedFromWindow follow the same pattern for the
	// same reason. Without this, InitToolBar() re-adds a second toolbar
	// every time NavigatorEditor is switched back to, and the first one
	// is never removed while some other tab is active.
	if (Window() && toolBar != NULL) {
		PWindow	*pWindow	= (PWindow *)Window();
		if (!pWindow->IsClosing())
			pWindow->RemoveToolBar("N_A_TOOL_BAR");
		toolBar	= NULL;
	}
	while (ChildAt(0)!=NULL)
	{
		RemoveChild(ChildAt(0));
	}
}

bool NavigatorEditor::IsFocus(void) const
{
	TRACE();
	return	 BView::IsFocus();

}

void NavigatorEditor::MakeFocus(bool focus)
{
	TRACE();
	BView::MakeFocus(focus);
}

void NavigatorEditor::KeyDown(const char *bytes, int32 numBytes)
{
	TRACE();
}

void NavigatorEditor::KeyUp(const char *bytes, int32 numBytes)
{
	TRACE();
}


void NavigatorEditor::MessageReceived(BMessage *message)
{
	TRACE();
	switch(message->what) 
	{
		case P_C_VALUE_CHANGED:
		{
			ValueChanged(message);
			break;
		}
		case N_A_INVOKATION:
		{
			BListView	*source	= NULL;
			if (message->FindPointer("ListView",(void **) &source) == B_OK)
				InsertNewList(source);
			break;
		}
		case P_C_EDITOR_SWITCHED_ACTIV:
		{
			ValueChanged(message);
			break;
		}
		case N_A_ADD:
		{
			NavToolbarAddNode(doc,focusedList);
			break;
		}
		case N_A_DELETE_NODE:
		{
			NavToolbarDelete(doc,focusedList);
			break;
		}
		default:
			BView::MessageReceived(message);
			break;
	}
}

void NavigatorEditor::InsertNewList(BListView *source)
{
	TRACE();
	int32			selection	= -1;
	BaseListItem	*item		= NULL;
	selection = source->CurrentSelection(0);
	if (selection>=0)
	{
		item =(BaseListItem *) source->ItemAt(selection);
		//vorher alle  überfüssigen Views löschen
		BView *sibling=source->Parent()->NextSibling();
		while (sibling != NULL)
		{
			RemoveChild(sibling);
			sibling=source->Parent()->NextSibling();
		}
		if (item->GetSupportedType() == P_C_CLASS_TYPE)
		{
			BRect		listrect		= Bounds();
			listrect.left				= source->Parent()->Frame().right+5;
			listrect.right				= listrect.left	+400;
			if (listrect.right > Bounds().right)
			{
				ResizeTo(listrect.right+B_V_SCROLL_BAR_WIDTH+5,Bounds().bottom);
			}
			BListView	*list			= new MessageListView(doc,listrect,((NodeItem *)item)->GetNode(),this);
			BMessage *invoked 			= new BMessage(N_A_INVOKATION);
			invoked->AddPointer("ListView",list);
			list->SetInvocationMessage(invoked);
			list->SetTarget(this);
			AddChild(new BScrollView("root",list,B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM,0,false,true));
			UpdateScrollBars();
		}
		Invalidate();
	}
}

void NavigatorEditor::DeleteRenderObject(BMessage *node)
{
	TRACE();
}

