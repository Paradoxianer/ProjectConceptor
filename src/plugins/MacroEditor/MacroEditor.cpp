#include "MacroEditor.h"

#include <Catalog.h>
#include <interface/Alert.h>
#include <interface/ListItem.h>
#include <interface/MenuItem.h>
#include <interface/ScrollView.h>
#include <storage/Directory.h>
#include <storage/File.h>
#include <storage/Path.h>
#include <support/String.h>

#include <stdlib.h>
#include <string.h>

#include "MacroText.h"
#include "PCommandManager.h"
#include "ProjectConceptorDefs.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MacroEditor"


static void AppendDefaultValueFor(type_code type, BString *out)
{
	switch (type) {
		case B_BOOL_TYPE:		*out << "false"; break;
		case B_INT8_TYPE:
		case B_INT16_TYPE:
		case B_INT32_TYPE:
		case B_INT64_TYPE:		*out << "0"; break;
		case B_FLOAT_TYPE:
		case B_DOUBLE_TYPE:	*out << "0.0"; break;
		case B_POINT_TYPE:		*out << "(0,0)"; break;
		case B_RECT_TYPE:		*out << "[0,0,0,0]"; break;
		case B_POINTER_TYPE:	*out << "@1"; break;
		case B_STRING_TYPE:
		default:				*out << "\"\""; break;
	}
}


/** valueContainer's own fields have no declared schema (see MacroText.h -
 * arbitrary node/value data, not a command) - ChangeValue/AddAttribute/
 * RemoveAttribute all shape it the same specific way their own Do()
 * expects (confirmed by reading each one), spelled out here since nothing
 * else in this file otherwise knows it. "type" needs the exact type_code
 * as a decimal int32 - the single most confusing part of hand-writing one
 * of these three commands (user report: got it wrong trying to add a bool
 * attribute) - so a cheat sheet for the common ones goes right into the
 * generated snippet as a comment line (ParseCommands() skips "#" lines). */
static void AppendValueContainerSnippet(const char *commandName, BString *out)
{
	*out << "  ~valueContainer\n";
	*out << "    name=\"\"\n";
	if (strcmp(commandName,"RemoveAttribute") == 0) {
		*out << "    index=0\n";
	} else {
		*out << "    # type: bool int8 int16 int32 int64 float double "
			"string point rect message (or the raw number)\n";
		*out << "    type=string\n";
		const char	*valueField	= (strcmp(commandName,"AddAttribute") == 0)
			? "newAttribute" : "newValue";
		*out << "    " << valueField << "=\"\"\n";
	}
	*out << "    # subgroup (optional, repeatable): nests into a sub-"
		"BMessage first, e.g. subgroup=\"Node::Data\"\n";
}


/** Builds a ready-to-drop DSL snippet for `command` - its own name line
 * plus one line per declared top-level field, pre-filled with a type-
 * appropriate placeholder instead of left empty (AppendDefaultValueFor()).
 * A B_MESSAGE_TYPE field has no declared schema of its own to fill in
 * generically - AppendValueContainerSnippet() covers the one shape
 * (valueContainer) this codebase actually needs a real template for;
 * "included_node" is left as a bare header, since its own content is
 * always machine-recorded node/connection data nobody hand-writes. */
static void BuildCommandSnippet(PCommand *command, BString *out)
{
	if (strcmp(command->Name(),"Insert") == 0) {
		InsertPrototypeText(out);
		return;
	}
	out->SetTo("");
	*out << command->Name() << "\n";
	int32				propCount		= 0;
	const property_info	*props			= command->PropertyInfo(&propCount);
	bool				hasNodePointer	= false;
	bool				hasNodeSelected	= false;
	for (int32 p = 0; p < propCount; p++) {
		for (int32 c = 0; c < 3; c++) {
			for (int32 f = 0; f < 5; f++) {
				const char	*fieldName	= props[p].ctypes[c].pairs[f].name;
				if (fieldName == NULL)
					continue;
				if (strcmp(fieldName,"node") == 0)
					hasNodePointer	= true;
				else if (strcmp(fieldName,"Node::selected") == 0)
					hasNodeSelected	= true;
			}
		}
	}
	// ChangeValue/AddAttribute/RemoveAttribute all declare both "node"
	// (an explicit pointer) and "Node::selected" (act on the whole
	// selection instead, #132's portable form) - a fresh snippet
	// otherwise fills in both at once, which looks like it's asking for
	// both when only one is ever meant to be there.
	if (hasNodePointer && hasNodeSelected)
		*out << "  # use ONE of node/Node::selected below, not both\n";
	for (int32 p = 0; p < propCount; p++) {
		for (int32 c = 0; c < 3; c++) {
			for (int32 f = 0; f < 5; f++) {
				const char	*fieldName	= props[p].ctypes[c].pairs[f].name;
				if (fieldName == NULL)
					continue;
				type_code	type	= props[p].ctypes[c].pairs[f].type;
				if (strcmp(fieldName,"valueContainer") == 0)
					AppendValueContainerSnippet(command->Name(),out);
				else if (type == B_MESSAGE_TYPE)
					*out << "  ~" << fieldName << "\n";
				else {
					*out << "  " << fieldName << "=";
					AppendDefaultValueFor(type,out);
					*out << "\n";
				}
			}
		}
	}
}


/** Word-wraps `text` at `width` columns - a BView tooltip doesn't wrap on
 * its own, so a command's one-line usage sentence otherwise becomes one
 * extremely wide line (user report). */
static void WrapText(const char *text, int32 width, BString *out)
{
	BString		word;
	int32		lineLen	= 0;
	for (const char *c = text; ; c++) {
		if ((*c == ' ') || (*c == '\0')) {
			if (word.Length() > 0) {
				if ((lineLen > 0) && (lineLen+1+word.Length() > width)) {
					*out << "\n";
					lineLen	= 0;
				} else if (lineLen > 0) {
					*out << " ";
					lineLen++;
				}
				*out << word;
				lineLen	+= word.Length();
				word.SetTo("");
			}
			if (*c == '\0')
				break;
		} else
			word.Append(c,1);
	}
}


/** Tooltip text for `commandName`: its usage sentence wrapped to a
 * readable width, then a blank line and the example (if there is one). */
static void BuildToolTipText(const char *commandName, const char *usage, BString *out)
{
	out->SetTo("");
	if ((usage != NULL) && (usage[0] != '\0'))
		WrapText(usage,48,out);
	const char	*example	= CommandExampleText(commandName);
	if (example != NULL) {
		if (out->Length() > 0)
			*out << "\n\n";
		*out << "Example:\n" << example;
	}
}


/** BStringItem carrying the two extra strings the reference list needs per
 * item (#55 follow-up, user report: "die Syntax komplett nicht
 * verstanden" - no way to see what a command does, or start from a
 * correctly-shaped example, without leaving the editor): the owning
 * command's own "usage" text, shown as a hover tooltip for both a
 * command's own top-level item and each of its field children, and - for
 * a top-level command item only - the drag-and-drop snippet
 * CommandReferenceListView::InitiateDrag() hands to MacroTextView (empty
 * for a field child item; only whole commands are draggable, not one
 * field on its own). */
class CommandListItem : public BStringItem
{
public:
	CommandListItem(const char *text, uint32 level, bool expanded,
		const BString &toolTip, const BString &snippet)
		: BStringItem(text,level,expanded), fToolTip(toolTip), fSnippet(snippet) {}
	const BString&	ToolTipText(void) const { return fToolTip; }
	const BString&	Snippet(void) const { return fSnippet; }
private:
	BString	fToolTip;
	BString	fSnippet;
};


/** Live per-row hover tooltips and drag-and-drop of a whole command's own
 * snippet into MacroTextView - see CommandListItem. BTextView already
 * accepts a dropped "text/plain" flavor as if it were dragged-in text
 * (BTextView::_MessageDropped(), confirmed against Haiku's own
 * TextView.cpp) - nothing needs to change on MacroTextView's own side at
 * all for the drop half of this, only the drag *source* here needs to
 * build that flavor. */
class CommandReferenceListView : public BOutlineListView
{
public:
	CommandReferenceListView(BRect frame, const char *name,
			list_view_type type, uint32 resizingMode)
		: BOutlineListView(frame,name,type,resizingMode),
			fLastHoveredItem(NULL) {}

	virtual void MouseMoved(BPoint where, uint32 code, const BMessage *dragMessage)
	{
		BOutlineListView::MouseMoved(where,code,dragMessage);
		int32		index	= IndexOf(where);
		BListItem	*hovered	= (index >= 0) ? ItemAt(index) : NULL;
		if (hovered == fLastHoveredItem)
			return;
		fLastHoveredItem	= hovered;
		CommandListItem	*item	= dynamic_cast<CommandListItem*>(hovered);
		SetToolTip(item ? item->ToolTipText().String() : (const char*)NULL);
	}

	virtual bool InitiateDrag(BPoint where, int32 index, bool wasSelected)
	{
		CommandListItem	*item	= dynamic_cast<CommandListItem*>(ItemAt(index));
		if ((item == NULL) || item->Snippet().IsEmpty())
			return false;
		BMessage	drag(B_MIME_DATA);
		drag.AddBool(kCommandSnippetDragMarker,true);
		drag.AddData("text/plain",B_MIME_TYPE,item->Snippet().String(),
			item->Snippet().Length());
		DragMessage(&drag,ItemFrame(index));
		return true;
	}

private:
	BListItem	*fLastHoveredItem;
};


MacroEditor::MacroEditor()
	:PEditor(),BView(BRect(0,0,500,300),"MacroEditor",B_FOLLOW_ALL_SIDES,B_WILL_DRAW)
{
	Init();
}

void MacroEditor::Init(void)
{
	configMessage		= new BMessage();
	fMacroList			= NULL;
	fMacroListScroll	= NULL;
	fTextView			= NULL;
	fTextScroll			= NULL;
	fStatus				= NULL;
	fLineColStatus		= NULL;
	fExportPanel		= NULL;
	fImportPanel		= NULL;
	fSelectedMacro		= NULL;
	fCommandList		= NULL;
	fCommandListScroll	= NULL;
}


void MacroEditor::AttachedToWindow(void)
{
	TRACE();
	if (fMacroList != NULL) {
		// already built (a tab detach/reattach shouldn't duplicate
		// children) - but LayoutChildren() again on every reattach: a
		// child positioned by explicit MoveTo()/ResizeTo() below doesn't
		// track the parent's size on its own, and the tab container can
		// hand back a different frame on a later reattach than it did the
		// first time this view was built - without this, children could
		// end up stranded at a stale position after a tab switch. Also
		// re-check the macro list every time the tab is switched to, not
		// just on the
		// P_C_VALUE_CHANGED broadcast path, since a macro recorded
		// elsewhere should show up on revisiting this tab even if that
		// broadcast is ever delayed/missed. Same reasoning applies to the
		// command reference list: this view's first attach happens while
		// PWindow::CreatEditorList() is still cycling through tabs, which
		// can run before command plugins finish registering - a first
		// build here can legitimately see an empty registry, so this has
		// to re-run on every visit rather than only once.
		LayoutChildren();
		RefreshMacroList();
		BuildCommandList();
		return;
	}

	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	// B_FOLLOW_LEFT_TOP (BListView's own constructor default) pins the
	// list to its original size when the BScrollView wrapping it is later
	// resized in LayoutChildren() - only the scrollview itself grew, the
	// list inside stayed at its construction-time size, leaving the
	// bottom half of the panel visibly empty and, since the scrollview's
	// own width shrinks to fit its wrapped 140px content but this list
	// never gave that width back either, the list rendering over/under
	// where the vertical scrollbar needs to be. B_FOLLOW_ALL_SIDES fixes
	// both - confirmed live (user report).
	fMacroList	= new BListView(BRect(0,0,140,280),"macroList",
		B_SINGLE_SELECTION_LIST,B_FOLLOW_ALL_SIDES);
	BMessage	*selMsg	= new BMessage(M_E_MACRO_SELECTED);
	fMacroList->SetSelectionMessage(selMsg);
	fMacroList->SetTarget(this);
	fMacroListScroll	= new BScrollView("macroListScroll",fMacroList,
		B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM,0,false,true);
	AddChild(fMacroListScroll);

	fTextView	= new MacroTextView(BRect(0,0,340,240),"macroText",BRect(4,4,336,236),
		B_FOLLOW_ALL_SIDES,B_WILL_DRAW | B_NAVIGABLE);
	fTextView->SetEditor(this);
	fTextScroll	= new BScrollView("macroTextScroll",fTextView,
		B_FOLLOW_ALL_SIDES,0,false,true,B_FANCY_BORDER);
	AddChild(fTextScroll);

	fStatus	= new BStringView(BRect(0,0,340,16),"status","");
	AddChild(fStatus);

	fLineColStatus	= new BStringView(BRect(0,0,100,16),"lineColStatus","");
	fLineColStatus->SetAlignment(B_ALIGN_RIGHT);
	AddChild(fLineColStatus);

	// Export/Import used to be buttons here too - moved to the Macro menu
	// (Save/Open, #55 follow-up) since they're macro-management actions.
	// No Apply button either (#55 follow-up) - MacroTextView auto-applies
	// at natural pause points instead (Enter, losing focus), see
	// ApplyEdits()/MacroTextView.

	// reference list of registered commands/fields (#55 follow-up) - read
	// only, built once below since the command registry never changes
	// after startup. CommandReferenceListView (not a plain
	// BOutlineListView): per-row hover tooltips + drag-and-drop of a
	// ready-made snippet into fTextView - see its own class comment.
	// see the same resize-mode fix/comment on fMacroList above
	fCommandList	= new CommandReferenceListView(BRect(0,0,160,280),"commandList",
		B_SINGLE_SELECTION_LIST,B_FOLLOW_ALL_SIDES);
	fCommandListScroll	= new BScrollView("commandListScroll",fCommandList,
		B_FOLLOW_RIGHT | B_FOLLOW_TOP_BOTTOM,0,false,true);
	AddChild(fCommandListScroll);
	BuildCommandList();

	LayoutChildren();
	RefreshMacroList();
}


void MacroEditor::LayoutChildren(void)
{
	if (fMacroList == NULL)
		return;
	BRect	bounds		= Bounds();
	float	listW		= 140;
	float	cmdListW	= 160;
	float	statusH		= 18;
	float	lineColW	= 100;

	// the status bar sits at the *bottom* now (user report: it used to
	// sit above fTextScroll only, pushing that one panel's own top edge
	// down by statusH while fMacroListScroll/fCommandListScroll both
	// started right at bounds.top - the three panels never lined up).
	// All three now share the same top and the same bottom (contentBottom).
	float	contentBottom	= bounds.bottom-statusH;

	fMacroListScroll->MoveTo(bounds.left,bounds.top);
	fMacroListScroll->ResizeTo(listW,contentBottom-bounds.top);

	fCommandListScroll->MoveTo(bounds.right-cmdListW,bounds.top);
	fCommandListScroll->ResizeTo(cmdListW,contentBottom-bounds.top);

	float	right	= bounds.right-cmdListW-1;
	float	left	= bounds.left+listW+1;

	fTextScroll->MoveTo(left,bounds.top);
	fTextScroll->ResizeTo(right-left,contentBottom-bounds.top);

	fLineColStatus->MoveTo(right-lineColW,contentBottom+2);
	fLineColStatus->ResizeTo(lineColW,statusH-4);

	fStatus->MoveTo(left+4,contentBottom+2);
	fStatus->ResizeTo(right-left-lineColW-8,statusH-4);
}


static const char* TypeDisplayName(type_code type)
{
	switch (type) {
		case B_BOOL_TYPE:		return "bool";
		case B_INT8_TYPE:		return "int8";
		case B_INT16_TYPE:		return "int16";
		case B_INT32_TYPE:		return "int32";
		case B_INT64_TYPE:		return "int64";
		case B_FLOAT_TYPE:		return "float";
		case B_DOUBLE_TYPE:	return "double";
		case B_STRING_TYPE:	return "string";
		case B_POINT_TYPE:		return "point";
		case B_RECT_TYPE:		return "rect";
		case B_POINTER_TYPE:	return "@id";
		case B_MESSAGE_TYPE:	return "~block";
		default:				return "raw";
	}
}


void MacroEditor::BuildCommandList(void)
{
	if ((fCommandList == NULL) || (doc == NULL))
		return;
	fCommandList->MakeEmpty();

	PCommandManager	*commandManager	= doc->GetCommandManager();
	for (int32 i = 0; i < commandManager->CountPCommand(); i++) {
		PCommand	*command	= commandManager->PCommandAt(i);
		if (command == NULL)
			continue;

		int32				propCount	= 0;
		const property_info	*props	= command->PropertyInfo(&propCount);
		// usage is per-property_info-entry, not per-command, but every
		// registered command here only ever declares exactly one - see
		// each plugin's own kXxxProperties[] array (always {..., count=1}).
		BString	usage;
		BuildToolTipText(command->Name(),(propCount > 0) ? props[0].usage : NULL,&usage);
		BString	snippet;
		BuildCommandSnippet(command,&snippet);

		CommandListItem	*commandItem	= new CommandListItem(command->Name(),0,true,usage,snippet);
		fCommandList->AddItem(commandItem);

		for (int32 p = 0; p < propCount; p++) {
			for (int32 c = 0; c < 3; c++) {
				for (int32 f = 0; f < 5; f++) {
					const char	*fieldName	= props[p].ctypes[c].pairs[f].name;
					if (fieldName == NULL)
						continue;
					BString	label;
					label << fieldName << ": " << TypeDisplayName(props[p].ctypes[c].pairs[f].type);
					// no snippet of its own - only a whole command is
					// draggable (see CommandListItem's own doc comment)
					fCommandList->AddUnder(
						new CommandListItem(label.String(),1,true,usage,BString()),
						commandItem);
				}
			}
		}
	}
}


void MacroEditor::FrameResized(float width, float height)
{
	LayoutChildren();
}


void MacroEditor::AttachedToManager(void)
{
	TRACE();
	if (fMacroList != NULL)
		RefreshMacroList();
}


void MacroEditor::DetachedFromManager(void)
{
	TRACE();
}


void MacroEditor::ValueChanged(BMessage *changedNodes)
{
	TRACE();
	// every command anywhere in the document broadcasts here (a Select
	// all, a node moved, ...). RefreshMacroList() ends in ShowSelectedMacro(),
	// which reloads the text view from the stored macro - wiping anything
	// typed but not yet applied. Only a changed *set* of macros (a new
	// recording, an import) is worth that.
	BList	*macroList	= doc->GetCommandManager()->GetMacroList();
	bool	unchanged	= (macroList->CountItems() == fKnownMacros.CountItems());
	for (int32 i = 0; unchanged && (i < macroList->CountItems()); i++)
		unchanged	= (macroList->ItemAt(i) == fKnownMacros.ItemAt(i));
	if (unchanged)
		return;
	RefreshMacroList();
}


void MacroEditor::SetShortCutFilter(ShortCutFilter *_shortCutFilter)
{
	if (LockLooper()) {
		AddFilter(_shortCutFilter);
		UnlockLooper();
	}
}


void MacroEditor::RefreshMacroList(bool reloadText)
{
	if ((fMacroList == NULL) || (doc == NULL))
		return;

	BString	previouslySelected;
	if (fSelectedMacro != NULL)
		fSelectedMacro->FindString("Name",&previouslySelected);

	fMacroList->MakeEmpty();
	fSelectedMacro	= NULL;

	BList	*macroList	= doc->GetCommandManager()->GetMacroList();
	int32	reselectIndex	= -1;
	for (int32 i = 0; i < macroList->CountItems(); i++) {
		BMessage	*macro	= (BMessage*)macroList->ItemAt(i);
		const char	*name	= NULL;
		macro->FindString("Name",&name);
		fMacroList->AddItem(new BStringItem(name ? name : ""));
		if ((name != NULL) && (previouslySelected == name))
			reselectIndex	= i;
	}

	// BListView::Select() DOES post the selection message (InvokeNotify(
	// fSelectMessage), Haiku's own ListView.cpp) - and it arrives
	// asynchronously, after whatever the caller does next. ImportFromFile()
	// sets the imported text right after this returns, then that late
	// M_E_MACRO_SELECTED ran ShowSelectedMacro() and overwrote it with the
	// (still empty) new macro - an import always "loaded empty" (user
	// report). ShowSelectedMacro() is called directly below anyway, so the
	// message is simply switched off while selecting programmatically.
	// Falls back to showing the first macro (if any) when there was
	// nothing to preserve - never leaves a stale selection showing after
	// the underlying macro list changed under it.
	fMacroList->SetSelectionMessage(NULL);
	if (reselectIndex >= 0)
		fMacroList->Select(reselectIndex);
	else if (fMacroList->CountItems() > 0)
		fMacroList->Select(0);
	fMacroList->SetSelectionMessage(new BMessage(M_E_MACRO_SELECTED));
	fKnownMacros.MakeEmpty();
	for (int32 i = 0; i < macroList->CountItems(); i++)
		fKnownMacros.AddItem(macroList->ItemAt(i));
	if (reloadText) {
		ShowSelectedMacro();
		return;
	}
	// the text on screen stays (it is what the new macro was created from)
	int32	selected	= fMacroList->CurrentSelection();
	if ((selected >= 0) && (selected < macroList->CountItems()))
		fSelectedMacro	= (BMessage*)macroList->ItemAt(selected);
}


BMessage* MacroEditor::AddNewMacro(const BString &name)
{
	BMessage	*newMacro	= new BMessage(P_C_MACRO_TYPE);
	newMacro->AddString("Name",name);
	doc->GetCommandManager()->GetMacroList()->AddItem(newMacro);
	BMenuItem	*item	= new BMenuItem(name.String(),newMacro);
	item->SetTarget(doc);
	doc->AddMenuItem(P_MENU_MACRO_PLAY,item);
	return newMacro;
}


BString MacroEditor::UniqueMacroName(void)
{
	BList	*macroList	= doc->GetCommandManager()->GetMacroList();
	for (int32 number = 1; ; number++) {
		BString	candidate(B_TRANSLATE("New macro"));
		if (number > 1)
			candidate << " " << number;
		bool	taken	= false;
		for (int32 i = 0; !taken && (i < macroList->CountItems()); i++) {
			const char	*name	= NULL;
			((BMessage*)macroList->ItemAt(i))->FindString("Name",&name);
			taken	= (name != NULL) && (candidate == name);
		}
		if (!taken)
			return candidate;
	}
}


void MacroEditor::NewMacro(void)
{
	if ((doc == NULL) || (fTextView == NULL))
		return;
	// typed text with nothing selected turns into the new macro (ApplyEdits)
	BString	typed(fTextView->Text());
	typed.Trim();
	if ((fSelectedMacro == NULL) && (typed.Length() > 0)) {
		ApplyEdits(false);
		return;
	}
	if (fSelectedMacro != NULL)
		ApplyEdits(false);
	fSelectedMacro	= AddNewMacro(UniqueMacroName());
	RefreshMacroList();
	SetStatus(B_TRANSLATE("New macro created - type its commands, Enter applies them."),false);
	fTextView->MakeFocus(true);
}


void MacroEditor::ShowSelectedMacro(void)
{
	if ((fTextView == NULL) || (doc == NULL))
		return;

	int32	index	= fMacroList->CurrentSelection();
	BList	*macroList	= doc->GetCommandManager()->GetMacroList();
	if ((index < 0) || (index >= macroList->CountItems())) {
		fSelectedMacro	= NULL;
		fTextView->SetMacroText("");
		SetStatus("",false);
		return;
	}

	fSelectedMacro	= (BMessage*)macroList->ItemAt(index);

	BList	commands;
	BMessage	entry;
	int32	i	= 0;
	while (fSelectedMacro->FindMessage("Macro::Commmand",i,&entry) == B_OK) {
		commands.AddItem(new BMessage(entry));
		entry.MakeEmpty();
		i++;
	}

	BString	text;
	SerializeCommands(&commands,&text);
	fTextView->SetMacroText(text);
	SetStatus("",false);

	for (int32 c = 0; c < commands.CountItems(); c++)
		delete (BMessage*)commands.ItemAt(c);
}


void MacroEditor::ApplyEdits(bool revealErrorLine)
{
	if (doc == NULL)
		return;

	BString	text;
	fTextView->ExpandedText(&text);
	if (fSelectedMacro == NULL) {
		BString	typed(text);
		typed.Trim();
		if (typed.Length() == 0) {
			SetStatus("",false);
			return;
		}
	}
	BList		parsed;
	BString		error;
	status_t	err	= ParseCommands(text,&parsed,doc->GetCommandManager(),&error);
	if (err != B_OK) {
		// error messages are "line N: ...", counted against the expanded
		// text ExpandedText() just built, not whatever's on screen - a
		// folded chip collapses many of those lines into one, so "line 23"
		// means nothing to look at until the right line is actually
		// revealed (expanding whatever chip stands in for it, if any).
		// not when this comes from losing focus: that runs the moment the
		// user clicks/drags something else (the reference list, say), and
		// moving the selection to the error line then destroyed the
		// selection they were about to use (user report)
		if (revealErrorLine && error.StartsWith("line ")) {
			int32	numEnd	= error.FindFirst(":",5);
			if (numEnd > 5) {
				BString	numText;
				error.CopyInto(numText,5,numEnd-5);
				int32	lineNo	= atol(numText.String());
				fTextView->RevealCanonicalLine(lineNo);
			}
		}
		SetStatus(error.String(),true);
		return;
	}

	// text typed with no macro selected creates one - otherwise there is
	// nothing to save or play
	if (fSelectedMacro == NULL) {
		fSelectedMacro	= AddNewMacro(UniqueMacroName());
		RefreshMacroList(false);
	}

	fSelectedMacro->RemoveName("Macro::Commmand");
	for (int32 i = 0; i < parsed.CountItems(); i++) {
		BMessage	*cmd	= (BMessage*)parsed.ItemAt(i);
		fSelectedMacro->AddMessage("Macro::Commmand",cmd);
		delete cmd;
	}

	// a missing "~included_node" block (folded or not) doesn't fail
	// ParseCommands() - it just parses as one fewer node/connection than
	// this macro started with, silently. This still applies the edit (it
	// may well be an intentional whole-command deletion, not an accident -
	// LostFoldedBlocks() can't tell the two apart) but at least surfaces it
	// instead of leaving it to only ever show up as an unresolved-id error
	// buried in a debug log at replay time.
	BString	blockWarning;
	if (fTextView->LostFoldedBlocks(&blockWarning)) {
		SetStatus(blockWarning.String(),true);
		return;
	}

	BString	status;
	status.SetToFormat(B_TRANSLATE("Applied (%ld command(s))."),(long)parsed.CountItems());
	SetStatus(status.String(),false);
}


void MacroEditor::ExportToFile(void)
{
	if (fSelectedMacro == NULL) {
		(new BAlert(B_TRANSLATE("Export Macro"),
			B_TRANSLATE("No macro selected - select one in the MacroEditor tab first."),
			B_TRANSLATE("OK")))->Go();
		return;
	}
	if (fExportPanel == NULL)
		fExportPanel	= new BFilePanel(B_SAVE_PANEL,new BMessenger(this));
	fExportPanel->Show();
}


void MacroEditor::ImportFromFile(void)
{
	if (fImportPanel == NULL)
		fImportPanel	= new BFilePanel(B_OPEN_PANEL,new BMessenger(this));
	fImportPanel->Show();
}


void MacroEditor::SetStatus(const char *text, bool isError)
{
	if (fStatus == NULL)
		return;
	fStatus->SetText(text);
	fStatus->SetHighColor(isError ? ui_color(B_FAILURE_COLOR) : ui_color(B_PANEL_TEXT_COLOR));
	fStatus->Invalidate();
	// a BStringView clips rather than wraps - a longer error (field name +
	// what's wrong with it) can easily run past the status line's width and
	// just disappear with no way to read the rest. The tooltip always shows
	// it in full, hover away the mystery instead of guessing at a cut-off
	// sentence.
	fStatus->SetToolTip((text != NULL) && (text[0] != '\0') ? text : NULL);
}


void MacroEditor::UpdateCursorPosition(int32 line, int32 column)
{
	if (fLineColStatus == NULL)
		return;
	BString	text;
	text.SetToFormat(B_TRANSLATE("Line %" B_PRId32 ", Col %" B_PRId32),line,column);
	fLineColStatus->SetText(text.String());
}


void MacroEditor::MessageReceived(BMessage *message)
{
	switch (message->what) {
		// PEditorManager::BroadCast() sends this to every registered
		// editor's GetHandler() - ValueChanged() is documented on PEditor
		// as reacting to it, but nothing calls ValueChanged() for us
		// automatically (see PEditor.h), so it needs catching here like
		// every other PEditor does in its own MessageReceived().
		case P_C_VALUE_CHANGED: {
			ValueChanged(message);
			break;
		}
		case M_E_MACRO_SELECTED: {
			ShowSelectedMacro();
			break;
		}
		// Reached from the Macro menu (Save/Open), not a button here -
		// see ExportToFile()/ImportFromFile()'s own comments (#55 follow-up).
		// PDocument forwards its MENU_MACRO_SAVE/MENU_MACRO_OPEN handlers
		// straight to this editor's handler by view name.
		case MENU_MACRO_SAVE: {
			ExportToFile();
			break;
		}
		case MENU_MACRO_NEW: {
			NewMacro();
			break;
		}
		case MENU_MACRO_OPEN: {
			ImportFromFile();
			break;
		}
		case B_SAVE_REQUESTED: {
			entry_ref	dirRef;
			const char	*name	= NULL;
			if ((message->FindRef("directory",&dirRef) == B_OK) &&
					(message->FindString("name",&name) == B_OK)) {
				BDirectory	dir(&dirRef);
				BFile		file(&dir,name,B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
				BString		text(fTextView->Text());
				file.Write(text.String(),text.Length());
				SetStatus(B_TRANSLATE("Exported."),false);
			}
			break;
		}
		case B_REFS_RECEIVED: {
			// Import always creates a brand new macro list entry - it
			// never touches whatever happened to be selected before the
			// file panel opened (#55 follow-up: unambiguous regardless of
			// what's currently shown, unlike replacing the open macro's
			// text would be).
			entry_ref	ref;
			if ((message->FindRef("refs",&ref) == B_OK) && (doc != NULL)) {
				BFile	file(&ref,B_READ_ONLY);
				off_t	size	= 0;
				file.GetSize(&size);
				char	*buffer	= new char[size+1];
				file.Read(buffer,size);
				buffer[size]	= '\0';
				BString	importedText(buffer);
				delete[] buffer;

				BString	name(ref.name);
				int32	dot	= name.FindLast('.');
				if (dot > 0)
					name.Truncate(dot);

				BMessage	*newMacro	= AddNewMacro(name);

				fSelectedMacro	= newMacro;
				RefreshMacroList();
				// RefreshMacroList()->ShowSelectedMacro() just serialized
				// the new (still empty) macro's real command list - this
				// overrides that with the raw imported DSL text instead,
				// which is only a draft until Apply commits it.
				fTextView->SetText(importedText.String());

				BString	status;
				status.SetToFormat(B_TRANSLATE("Imported as new macro \"%s\" - review it, then press Enter or click elsewhere to apply."),
					name.String());
				SetStatus(status.String(),false);
			}
			break;
		}
		default:
			BView::MessageReceived(message);
			break;
	}
}
