#include "ToolBar.h"
#include "ToolItem.h"
#include "NavigatorEditor.h"
#include "NavigatorCommands.h"
#include "PCommandManager.h"
#include "MessageListView.h"
#include "NodeItem.h"
#include "PEditorManager.h"
#include "PWindow.h"

#include <app/Cursor.h>
#include <interface/Bitmap.h>
#include <interface/ControlLook.h>
#include <interface/Font.h>
#include <interface/GraphicsDefs.h>
#include <interface/ScrollView.h>
#include <support/List.h>
#include <translation/TranslationUtils.h>
#include <translation/TranslatorFormats.h>
#include <Catalog.h>
#include <math.h>
#include <string.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "NavigatorEditor"

// column widths include the column's own vertical scroll bar
static const float	kRootColumnWidth	= 200+B_V_SCROLL_BAR_WIDTH;
static const float	kColumnWidth		= 400+B_V_SCROLL_BAR_WIDTH;
static const float	kMinColumnWidth		= 80;
static const float	kDividerWidth		= 6;

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
	draggedDivider	= -1;
	dragStartX		= 0;
	dragStartWidth	= 0;
	resizeCursor	= false;
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
		rootrect.right				= kRootColumnWidth-B_V_SCROLL_BAR_WIDTH;
		root						= new NodeListView(rootrect,allNodes,doc,this);
		focusedList					= root;
		BMessage *selected 			= new BMessage(N_A_INVOKATION);
		selected->AddPointer("ListView",root);
		root->SetSelectionMessage(selected);
		root->SetTarget(this);
		AddChild(new BScrollView("root",root,B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM,0,false,true));
		SetViewColor(255,255,255,255);
		LayoutColumns();
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

void NavigatorEditor::LayoutColumns(void)
{
	float	left	= 0;
	int32	index	= 0;
	for (BView *column = ChildAt(0); column != NULL; column = column->NextSibling()) {
		column->MoveTo(left,0);
		column->ResizeTo(ColumnWidth(index),Bounds().Height());
		left	+= ColumnWidth(index)+kDividerWidth;
		index++;
	}
	// never narrower than the tab, or the white area ends mid-window
	float	width	= left;
	if ((myScrollParent != NULL) && (myScrollParent->Bounds().Width() > width))
		width	= myScrollParent->Bounds().Width();
	if (width != Bounds().Width())
		ResizeTo(width,Bounds().Height());
	UpdateScrollBars();
	Invalidate();
}

float NavigatorEditor::ColumnWidth(int32 index) const
{
	if ((index >= 0) && (index < (int32)columnWidths.size()))
		return columnWidths[index];
	return (index == 0) ? kRootColumnWidth : kColumnWidth;
}

int32 NavigatorEditor::DividerAt(BPoint where) const
{
	int32	index	= 0;
	for (BView *column = ChildAt(0); column != NULL; column = column->NextSibling()) {
		float	right	= column->Frame().right;
		if ((where.x > right) && (where.x <= right+kDividerWidth))
			return index;
		index++;
	}
	return -1;
}

void NavigatorEditor::Draw(BRect updateRect)
{
	rgb_color	base	= ui_color(B_PANEL_BACKGROUND_COLOR);
	int32		index	= 0;
	for (BView *column = ChildAt(0); column != NULL; column = column->NextSibling()) {
		BRect	divider(column->Frame().right+1,Bounds().top,
			column->Frame().right+kDividerWidth,Bounds().bottom);
		uint32	flags	= (index == draggedDivider) ? BControlLook::B_CLICKED : 0;
		if (divider.Intersects(updateRect))
			be_control_look->DrawSplitter(this,divider,updateRect,base,
				B_HORIZONTAL,flags,0);
		index++;
	}
}

void NavigatorEditor::MouseDown(BPoint where)
{
	int32	divider	= DividerAt(where);
	if (divider < 0) {
		BView::MouseDown(where);
		return;
	}
	draggedDivider	= divider;
	// in screen coordinates: this view moves under the mouse while the
	// columns are laid out anew
	dragStartX		= ConvertToScreen(where).x;
	dragStartWidth	= ColumnWidth(divider);
	SetMouseEventMask(B_POINTER_EVENTS,B_NO_POINTER_HISTORY | B_LOCK_WINDOW_FOCUS);
	Invalidate();
}

void NavigatorEditor::MouseMoved(BPoint where, uint32 transit,
	const BMessage *dragMessage)
{
	if (draggedDivider >= 0) {
		while ((int32)columnWidths.size() <= draggedDivider)
			columnWidths.push_back(ColumnWidth(columnWidths.size()));
		float	width	= dragStartWidth+ConvertToScreen(where).x-dragStartX;
		columnWidths[draggedDivider]	= fmax(width,kMinColumnWidth);
		LayoutColumns();
		return;
	}
	bool	overDivider	= (transit != B_EXITED_VIEW) && (DividerAt(where) >= 0);
	if (overDivider != resizeCursor) {
		BCursor	cursor(overDivider ? B_CURSOR_ID_RESIZE_EAST_WEST : B_CURSOR_ID_SYSTEM_DEFAULT);
		SetViewCursor(&cursor);
		resizeCursor	= overDivider;
	}
	BView::MouseMoved(where,transit,dragMessage);
}

void NavigatorEditor::MouseUp(BPoint where)
{
	if (draggedDivider >= 0) {
		draggedDivider	= -1;
		Invalidate();
	}
	BView::MouseUp(where);
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
	sprintf(renderString,"NavigatorEditor%ld::Renderer",(long)id);
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
	if ((child = ChildAt(1)) != NULL)
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
	// the tab shrinking shrinks this view too, columns must stay reachable
	LayoutColumns();
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
		case N_A_SHOW_IN_GRAPH:
		{
			BMessage	*node	= NULL;
			if (message->FindPointer(P_C_DOC_ENTRY,(void **)&node) == B_OK)
				ShowInGraph(node);
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
		// the group headers (Node::Data, Node::outgoing...) are plain
		// BStringItems, nothing to open
		item = dynamic_cast<BaseListItem *>(source->ItemAt(selection));
		if (item == NULL)
			return;
		//vorher alle  überfüssigen Views löschen
		BView *sibling=source->Parent()->NextSibling();
		while (sibling != NULL)
		{
			RemoveChild(sibling);
			sibling=source->Parent()->NextSibling();
		}
		if (item->GetSupportedType() == P_C_CLASS_TYPE)
		{
			// LayoutColumns() places and sizes it
			BRect		listrect		= Bounds();
			listrect.right				= listrect.left+kColumnWidth-B_V_SCROLL_BAR_WIDTH;
			BListView	*list			= new MessageListView(doc,listrect,((NodeItem *)item)->GetNode(),this);
			BMessage *invoked 			= new BMessage(N_A_INVOKATION);
			invoked->AddPointer("ListView",list);
			list->SetInvocationMessage(invoked);
			list->SetTarget(this);
			AddChild(new BScrollView("root",list,B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM,0,false,true));
		}
		LayoutColumns();
	}
}

void NavigatorEditor::ShowInGraph(BMessage *node)
{
	BMessage	select(P_C_EXECUTE_COMMAND);
	select.AddPointer("node",node);
	select.AddString("Command::Name","Select");
	BMessenger(doc).SendMessage(&select);
	// before the broadcast: a tab that isn't shown has no looper
	PWindow	*window	= dynamic_cast<PWindow *>(Window());
	if ((window == NULL) || !window->SelectEditorTab("GraphEditor")) {
		PRINT(("NavigatorEditor::ShowInGraph: no GraphEditor tab\n"));
		return;
	}
	BMessage	reveal(P_C_REVEAL_NODES);
	reveal.AddPointer(P_C_DOC_ENTRY,node);
	doc->GetEditorManager()->BroadCast(&reveal);
}

void NavigatorEditor::DeleteRenderObject(BMessage *node)
{
	TRACE();
}



const char* NavigatorEditor::TabLabel(void)
{
	return B_TRANSLATE("Data");
}
