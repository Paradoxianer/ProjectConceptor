#include "LayoutEditor.h"

#include <support/Autolock.h>

#include "VectorIcon.h"

#include <string.h>

#include <Alert.h>
#include <Bitmap.h>
#include <Catalog.h>
#include <DataIO.h>
#include <Resources.h>
#include <String.h>
#include <TranslationUtils.h>

#include "BaseItem.h"
#include "ChoiceToolItem.h"
#include "DotLayouter.h"
#include "LayoutCommandBuilder.h"
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
	applyItem			= NULL;
	directionItem		= NULL;
	topologyItem		= NULL;
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

	// in the window's toolbar, view group; not called from the window's
	// thread
	BAutolock	windowLock(pWindow);
	if (!windowLock.IsLocked())
		return;
	toolBar	= (ToolBar *)pWindow->FindView(P_M_STANDART_TOOL_BAR);
	if (toolBar == NULL)
		return;

	// icons from this plugin's own resources (see LayoutEditor.rdef);
	// ToolItem/AddChoice() both fall back gracefully to a plain
	// icon-less entry if a given LoadIcon() call returns NULL.
	BResources	res;
	bool		haveRes	= (pluginID >= 0) && (res.SetToImage(pluginID) == B_OK);

	ToolItem	*apply	= new ToolItem(B_TRANSLATE("Auto-Layout"),
		haveRes ? LoadVectorIcon(&res,"layout",kToolIconSize) : NULL,new BMessage(L_E_APPLY_LAYOUT));
	apply->BButton::SetToolTip(B_TRANSLATE("Automatically arrange the graph"));
	toolBar->AddItem(apply,P_TOOL_GROUP_VIEW);
	applyItem	= apply;
	// BMessenger(this) resolves via GetHandler()'s own Looper() (doc's,
	// already set by RegisterPEditor()) - plain SetTarget(this) would
	// default to the button's own looper (pWindow) instead.
	apply->SetTarget(BMessenger(this));

	ChoiceToolItem	*direction	= new ChoiceToolItem(B_TRANSLATE("Direction"),
		new BMessage(L_E_SET_DIRECTION),ITEM_WIDTH*2);
	direction->SetIconOnly(true);
	direction->AddChoice(B_TRANSLATE("Top " "\xE2\x86\x92" " Bottom"),"TB",
		haveRes ? LoadVectorIcon(&res,"dir-tb",kToolIconSize) : NULL);
	direction->AddChoice(B_TRANSLATE("Left " "\xE2\x86\x92" " Right"),"LR",
		haveRes ? LoadVectorIcon(&res,"dir-lr",kToolIconSize) : NULL);
	direction->AddChoice(B_TRANSLATE("Right " "\xE2\x86\x92" " Left"),"RL",
		haveRes ? LoadVectorIcon(&res,"dir-rl",kToolIconSize) : NULL);
	direction->AddChoice(B_TRANSLATE("Bottom " "\xE2\x86\x92" " Top"),"BT",
		haveRes ? LoadVectorIcon(&res,"dir-bt",kToolIconSize) : NULL);
	direction->SetToolTip(B_TRANSLATE("Layout direction"));
	toolBar->AddItem(direction,P_TOOL_GROUP_VIEW);
	directionItem	= direction;
	direction->SetTarget(BMessenger(this));

	ChoiceToolItem	*topology	= new ChoiceToolItem(B_TRANSLATE("Topology"),
		new BMessage(L_E_SET_ENGINE),ITEM_WIDTH*2);
	topology->SetIconOnly(true);
	topology->AddChoice(B_TRANSLATE("Hierarchical"),"dot",
		haveRes ? LoadVectorIcon(&res,"topo-dot",kToolIconSize) : NULL);
	topology->AddChoice(B_TRANSLATE("Spring model"),"neato",
		haveRes ? LoadVectorIcon(&res,"topo-neato",kToolIconSize) : NULL);
	topology->AddChoice(B_TRANSLATE("Force-directed"),"fdp",
		haveRes ? LoadVectorIcon(&res,"topo-fdp",kToolIconSize) : NULL);
	topology->AddChoice(B_TRANSLATE("Force (large graphs)"),"sfdp",
		haveRes ? LoadVectorIcon(&res,"topo-sfdp",kToolIconSize) : NULL);
	topology->AddChoice(B_TRANSLATE("Circular"),"circo",
		haveRes ? LoadVectorIcon(&res,"topo-circo",kToolIconSize) : NULL);
	topology->AddChoice(B_TRANSLATE("Radial"),"twopi",
		haveRes ? LoadVectorIcon(&res,"topo-twopi",kToolIconSize) : NULL);
	topology->SetToolTip(B_TRANSLATE("Layout topology"));
	toolBar->AddItem(topology,P_TOOL_GROUP_VIEW);
	topologyItem	= topology;
	topology->SetTarget(BMessenger(this));
}


void LayoutEditor::DetachedFromManager(void)
{
	if (toolBar == NULL)
		return;
	// while the window closes, its toolbar is torn down with it
	PWindow	*pWindow	= (doc != NULL) ? doc->GetWindow() : NULL;
	if ((pWindow != NULL) && !pWindow->IsClosing() && pWindow->Lock()) {
		toolBar->RemoveItem(applyItem);
		toolBar->RemoveItem(directionItem);
		toolBar->RemoveItem(topologyItem);
		pWindow->Unlock();
	}
	toolBar			= NULL;
	applyItem		= NULL;
	directionItem	= NULL;
	topologyItem	= NULL;
}


void LayoutEditor::ValueChanged(BMessage *changedNodes)
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
		case L_E_SET_DIRECTION: {
			const char	*value	= NULL;
			if (message->FindString("value",&value) == B_OK)
				SetRankDir(value);
			break;
		}
		case L_E_SET_ENGINE: {
			const char	*value	= NULL;
			if (message->FindString("value",&value) == B_OK)
				SetEngine(value);
			break;
		}
		default:
			BHandler::MessageReceived(message);
			break;
	}
}


BMessage* LayoutEditor::BuildLayoutCommand(BMessage *positions)
{
	// Shared with the Layout PCommand plugin (#55) - see
	// LayoutCommandBuilder.h for why (ChangeValue, not Move: Move applies
	// one dx/dy to doc->GetSelected() as a whole, no way to target one
	// node's absolute position).
	return LayoutBuildBatchCommand(positions);
}


void LayoutEditor::CenterOnOldBounds(const BList *nodes, BMessage *positions)
{
	// Shared with the Layout PCommand plugin (#55) - see LayoutCommandBuilder.h.
	LayoutCenterOnOldBounds(nodes,positions);
}


void LayoutEditor::SetRankDir(const char *rankdir)
{
	DotLayouter	*dotLayouter	= dynamic_cast<DotLayouter *>(layouter);
	if (dotLayouter != NULL)
		dotLayouter->SetRankDir(rankdir);
}


void LayoutEditor::SetEngine(const char *engine)
{
	DotLayouter	*dotLayouter	= dynamic_cast<DotLayouter *>(layouter);
	if (dotLayouter != NULL)
		dotLayouter->SetEngine(engine);
}


void LayoutEditor::ApplyLayout(void)
{
	if ((applyingLayout) || (doc == NULL))
		return;
	applyingLayout	= true;

	// Delegates to the "Layout" PCommand (#55) - same algorithm, but now
	// undoable/scriptable/macro-recordable through the normal command
	// registry instead of this toolbar button being the only way in.
	// `settings` is kept by this caller (not just handed off), so the
	// "error" field Layout::Do() may add is still readable afterwards -
	// Do()'s own non-interactive contract (see Layout.cpp) means it never
	// pops a BAlert itself; that stays here, the interactive caller.
	BMessage	*settings	= new BMessage(P_C_EXECUTE_COMMAND);
	settings->AddString("Command::Name","Layout");
	DotLayouter	*dotLayouter	= dynamic_cast<DotLayouter *>(layouter);
	if (dotLayouter != NULL) {
		settings->AddString("direction",dotLayouter->RankDir());
		settings->AddString("engine",dotLayouter->Engine());
	}
	doc->GetCommandManager()->Execute(settings);

	const char	*error	= NULL;
	if (settings->FindString("error",&error) == B_OK) {
		(new BAlert(B_TRANSLATE("Auto-Layout"),error,B_TRANSLATE("OK"),
			NULL,NULL,B_WIDTH_AS_USUAL,B_STOP_ALERT))->Go();
	}

	applyingLayout	= false;
}
