#include "LayoutEditor.h"

#include <string.h>

#include <Alert.h>
#include <Bitmap.h>
#include <Catalog.h>
#include <DataIO.h>
#include <Resources.h>
#include <String.h>
#include <TranslationUtils.h>

#include "BaseItem.h"
#include "DotLayouter.h"
#include "PCommandManager.h"
#include "PWindow.h"
#include "ProjectConceptorDefs.h"
#include "ToolBar.h"
#include "ToolItem.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "LayoutEditor"

static const char	*L_E_TOOL_BAR	= "L_E_TOOL_BAR";


LayoutEditor::LayoutEditor(image_id newId):PEditor(),BHandler("LayoutEditor")
{
	configMessage		= new BMessage();
	layouter			= NULL;
	toolBar				= NULL;
	applyingLayout		= false;
	pluginID			= newId;
}


LayoutEditor::~LayoutEditor(void)
{
	delete layouter;
}


void LayoutEditor::SetLayouter(PLayouter *newLayouter)
{
	delete layouter;
	layouter	= newLayouter;
}


void LayoutEditor::AttachedToManager(void)
{
	if (layouter == NULL)
		SetLayouter(new DotLayouter());

	// no view -> reach PWindow via doc, not Window(). Valid this early
	// because PWindow's own ctor sets it via SetWindow() before Show().
	PWindow	*pWindow	= doc->GetWindow();
	if (pWindow == NULL)
		return;

	toolBar	= new ToolBar(BRect(0,0,50,ITEM_HEIGHT+4),L_E_TOOL_BAR,B_ITEMS_IN_ROW);

	// icon from this plugin's own resources ("layout", see LayoutEditor.rdef);
	// ToolItem falls back to a plain bordered button if bmp stays NULL.
	BBitmap		*bmp	= NULL;
	BResources	res;
	if ((pluginID >= 0) && (res.SetToImage(pluginID) == B_OK)) {
		size_t		size;
		const void	*data	= res.LoadResource((type_code)'PNG ',"layout",&size);
		if (data != NULL)
			bmp	= BTranslationUtils::GetBitmap(new BMemoryIO(data,size));
	}

	ToolItem	*applyItem	= new ToolItem(B_TRANSLATE("Auto-Layout"),
		bmp,new BMessage(L_E_APPLY_LAYOUT));
	applyItem->BButton::SetToolTip(B_TRANSLATE("Automatically arrange the graph"));
	toolBar->AddItem(applyItem);
	// BMessenger(this) resolves via GetHandler()'s own Looper() (doc's,
	// already set by RegisterPEditor()) - plain SetTarget(this) would
	// default to the button's own looper (pWindow) instead.
	applyItem->SetTarget(BMessenger(this));

	pWindow->AddToolBar(toolBar);
}


void LayoutEditor::DetachedFromManager(void)
{
	if (toolBar != NULL) {
		PWindow	*pWindow	= (doc != NULL) ? doc->GetWindow() : NULL;
		if (pWindow != NULL)
			pWindow->RemoveToolBar(L_E_TOOL_BAR);
		toolBar	= NULL;
	}
}


void LayoutEditor::ValueChanged(void)
{
	// no-op by design - only reacts to explicit ApplyLayout() triggers.
	if (applyingLayout)
		return;
}


void LayoutEditor::SetShortCutFilter(ShortCutFilter *_shortCutFilter)
{
	AddFilter(_shortCutFilter);
}


void LayoutEditor::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case L_E_APPLY_LAYOUT: {
			ApplyLayout();
			break;
		}
		default:
			BHandler::MessageReceived(message);
			break;
	}
}


BMessage* LayoutEditor::BuildLayoutCommand(BMessage *positions)
{
	// One "Batch" wrapper, one ChangeValue subPCommand per node - single
	// undo step (see #116). ChangeValue not Move: Move applies one dx/dy
	// to doc->GetSelected() as a whole, no way to target one node's
	// absolute position; ChangeValue takes an explicit per-node value
	// directly (same technique ClassRenderer.cpp already uses).
	BMessage	*wrapper		= new BMessage(P_C_EXECUTE_COMMAND);
	wrapper->AddString("Command::Name","Batch");

	int32		i				= 0;
	void		*nodePtr		= NULL;
	BRect		newFrame;
	int32		subCommandCount	= 0;
	while (positions->FindPointer("node",i,&nodePtr) == B_OK) {
		if (positions->FindRect("frame",i,&newFrame) == B_OK) {
			BMessage	*subCommand		= new BMessage(P_C_EXECUTE_COMMAND);
			BMessage	*valueContainer	= new BMessage();
			subCommand->AddString("Command::Name","ChangeValue");
			subCommand->AddPointer("node",nodePtr);
			valueContainer->AddString("name",P_C_NODE_FRAME);
			valueContainer->AddInt32("type",(int32)B_RECT_TYPE);
			valueContainer->AddRect("newValue",newFrame);
			subCommand->AddMessage("valueContainer",valueContainer);
			wrapper->AddMessage("PCommand::subPCommand",subCommand);
			subCommandCount++;
		}
		i++;
	}

	if (subCommandCount == 0) {
		delete wrapper;
		return NULL;
	}
	return wrapper;
}


void LayoutEditor::CenterOnOldBounds(const BList *nodes, BMessage *positions)
{
	BRect	oldBounds;
	bool	haveOld		= false;
	for (int32 i = 0; i < nodes->CountItems(); i++) {
		BMessage	*node	= (BMessage*)nodes->ItemAt(i);
		BRect		nodeFrame;
		if ((node != NULL) && (node->FindRect(P_C_NODE_FRAME,&nodeFrame) == B_OK)) {
			oldBounds	= haveOld ? (oldBounds | nodeFrame) : nodeFrame;
			haveOld		= true;
		}
	}

	BRect	newBounds;
	bool	haveNew		= false;
	int32	i			= 0;
	BRect	positionFrame;
	while (positions->FindRect("frame",i,&positionFrame) == B_OK) {
		newBounds	= haveNew ? (newBounds | positionFrame) : positionFrame;
		haveNew		= true;
		i++;
	}

	if ((!haveOld) || (!haveNew))
		return;

	BPoint	oldCenter((oldBounds.left+oldBounds.right)/2,(oldBounds.top+oldBounds.bottom)/2);
	BPoint	newCenter((newBounds.left+newBounds.right)/2,(newBounds.top+newBounds.bottom)/2);
	BPoint	delta	= oldCenter-newCenter;
	if (delta == BPoint(0,0))
		return;

	i	= 0;
	while (positions->FindRect("frame",i,&positionFrame) == B_OK) {
		positionFrame.OffsetBy(delta);
		positions->ReplaceRect("frame",i,positionFrame);
		i++;
	}
}


void LayoutEditor::ApplyLayout(void)
{
	if ((applyingLayout) || (layouter == NULL) || (doc == NULL))
		return;

	BList	*nodes			= doc->GetAllNodes();
	BList	*connections	= doc->GetAllConnections();
	if ((nodes == NULL) || (nodes->CountItems() == 0))
		return;

	applyingLayout	= true;

	if (!layouter->IsAvailable()) {
		BString	text;
		text.SetToFormat(B_TRANSLATE("%s is not available - is it installed and on PATH?"),
			layouter->Name());
		(new BAlert(B_TRANSLATE("Auto-Layout"),text.String(),B_TRANSLATE("OK"),
			NULL,NULL,B_WIDTH_AS_USUAL,B_STOP_ALERT))->Go();
		applyingLayout	= false;
		return;
	}

	BMessage	positions;
	status_t	err	= layouter->Layout(nodes,connections,&positions);
	if (err != B_OK) {
		BString	text;
		text.SetToFormat(B_TRANSLATE("%s failed: %s"),layouter->Name(),strerror(err));
		(new BAlert(B_TRANSLATE("Auto-Layout"),text.String(),B_TRANSLATE("OK"),
			NULL,NULL,B_WIDTH_AS_USUAL,B_STOP_ALERT))->Go();
		applyingLayout	= false;
		return;
	}

	CenterOnOldBounds(nodes,&positions);

	BMessage	*wrapper	= BuildLayoutCommand(&positions);
	if (wrapper != NULL)
		(new BMessenger(doc))->SendMessage(wrapper);

	applyingLayout	= false;
}
