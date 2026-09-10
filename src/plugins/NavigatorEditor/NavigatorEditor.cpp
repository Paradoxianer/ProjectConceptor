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

// Small glyph-on-a-tile icon, drawn once instead of shipped as a PNG
// resource - the toolbar just needs three unambiguous, distinctly
// labelled buttons, not hand-authored pixel art.
static BBitmap* MakeGlyphIcon(const char *glyph, rgb_color tint)
{
	BRect	bounds(0,0,19,19);
	BBitmap	*bmp	= new BBitmap(bounds,B_RGBA32,true);
	BView	*view	= new BView(bounds,"glyph",B_FOLLOW_NONE,B_WILL_DRAW);
	bmp->AddChild(view);
	bmp->Lock();
	view->SetHighColor(0,0,0,0);
	view->FillRect(bounds,B_SOLID_HIGH);
	view->SetHighColor(tint);
	view->SetDrawingMode(B_OP_ALPHA);
	view->SetPenSize(2);
	view->StrokeRoundRect(bounds.InsetByCopy(1,1),4,4);
	BFont	font(be_bold_font);
	font.SetSize(11);
	view->SetFont(&font);
	float	width	= view->StringWidth(glyph);
	font_height	fh;
	font.GetHeight(&fh);
	view->MovePenTo((bounds.Width()-width)/2,
		(bounds.Height()+fh.ascent)/2-1);
	view->DrawString(glyph);
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
		Invalidate();
}

void NavigatorEditor::InitToolBar(void)
{
	PWindow	*pWindow	= (PWindow *)Window();

	toolBar			= new ToolBar(BRect(1,1,30,2800),"N_A_TOOL_BAR",B_ITEMS_IN_COLUMN);

	rgb_color	addNodeTint			= {40,150,40,255};
	rgb_color	addAttributeTint	= {40,110,190,255};
	rgb_color	deleteNodeTint		= {190,60,60,255};

	addNode			= new ToolItem("addNode",
		MakeGlyphIcon("+N",addNodeTint),new BMessage(N_A_ADD_NODE));
	addNode->BButton::SetToolTip(B_TRANSLATE(
		"Add a node (child of the selected node, or top-level)"));
	toolBar->AddItem(addNode);

	addAttribute	= new ToolItem("addAttribute",
		MakeGlyphIcon("+A",addAttributeTint),new BMessage(N_A_ADD_ATTRIBUTE));
	addAttribute->BButton::SetToolTip(B_TRANSLATE(
		"Add an attribute to the selected node"));
	toolBar->AddItem(addAttribute);

	toolBar->AddSeperator();

	deleteNode		= new ToolItem("deleteNode",
		MakeGlyphIcon("-",deleteNodeTint),new BMessage(N_A_DELETE_NODE));
	deleteNode->BButton::SetToolTip(B_TRANSLATE("Delete the selected node"));
	toolBar->AddItem(deleteNode);

	toolBar->ResizeTo(30,pWindow->P_M_MAIN_VIEW_BOTTOM-pWindow->P_M_MAIN_VIEW_TOP);
	pWindow->AddToolBar(toolBar);
	addNode->SetTarget(this);
	addAttribute->SetTarget(this);
	deleteNode->SetTarget(this);
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


void NavigatorEditor::ValueChanged()
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

void NavigatorEditor::DetachedFromWindow(void)
{
	TRACE();
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
			ValueChanged();
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
			ValueChanged();
			break;
		}
		case N_A_ADD_NODE:
		{
			NavToolbarAddNode(doc,focusedList);
			break;
		}
		case N_A_ADD_ATTRIBUTE:
		{
			BPoint	point	= addAttribute->Frame().LeftBottom();
			addAttribute->Parent()->ConvertToScreen(&point);
			NavToolbarAddAttribute(doc,focusedList,this,point);
			break;
		}
		case N_A_DELETE_NODE:
		{
			NavToolbarDeleteNode(doc,focusedList);
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
		}
		Invalidate();
	}
}

void NavigatorEditor::DeleteRenderObject(BMessage *node)
{
	TRACE();
}

