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

#include "MacroText.h"
#include "PCommandManager.h"
#include "ProjectConceptorDefs.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MacroEditor"

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
	fApplyButton		= NULL;
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
		// non-B_FOLLOW_ALL_SIDES child like fApplyButton only tracks the
		// parent's size automatically via its own resizing mode, not via
		// LayoutChildren()'s bottom-anchored placement, and the tab
		// container can hand back a different frame on a later reattach
		// than it did the first time this view was built - without this,
		// the button stayed at its very first position and could end up
		// stranded mid-textview after a tab switch. Also re-check the
		// macro list every time the tab is switched to, not just on the
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

	fMacroList	= new BListView(BRect(0,0,140,280),"macroList");
	BMessage	*selMsg	= new BMessage(M_E_MACRO_SELECTED);
	fMacroList->SetSelectionMessage(selMsg);
	fMacroList->SetTarget(this);
	fMacroListScroll	= new BScrollView("macroListScroll",fMacroList,
		B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM,0,false,true);
	AddChild(fMacroListScroll);

	fTextView	= new BTextView(BRect(0,0,340,240),"macroText",BRect(4,4,336,236),
		B_FOLLOW_ALL_SIDES,B_WILL_DRAW | B_NAVIGABLE);
	fTextScroll	= new BScrollView("macroTextScroll",fTextView,
		B_FOLLOW_ALL_SIDES,0,false,true,B_FANCY_BORDER);
	AddChild(fTextScroll);

	fStatus	= new BStringView(BRect(0,0,340,16),"status","");
	AddChild(fStatus);

	// Export/Import used to be buttons here too - moved to the Macro menu
	// (Save/Open, #55 follow-up) since they're macro-management actions,
	// not edits to the text currently on screen the way Apply is.
	fApplyButton	= new BButton(BRect(0,0,80,24),"apply",B_TRANSLATE("Apply"),
		new BMessage(M_E_APPLY));
	fApplyButton->SetTarget(this);
	AddChild(fApplyButton);

	// reference list of registered commands/fields (#55 follow-up) - read
	// only, built once below since the command registry never changes
	// after startup.
	fCommandList	= new BOutlineListView(BRect(0,0,160,280),"commandList");
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
	BRect	bounds	= Bounds();
	float	listW		= 140;
	float	cmdListW	= 160;
	float	buttonH	= 28;
	float	statusH	= 18;

	fMacroListScroll->MoveTo(bounds.left,bounds.top);
	fMacroListScroll->ResizeTo(listW,bounds.Height());

	fCommandListScroll->MoveTo(bounds.right-cmdListW,bounds.top);
	fCommandListScroll->ResizeTo(cmdListW,bounds.Height());

	float	right	= bounds.right-cmdListW-1;
	float	left	= bounds.left+listW+1;

	fStatus->MoveTo(left+4,bounds.top+2);
	fStatus->ResizeTo(right-left-8,statusH-4);

	fTextScroll->MoveTo(left,bounds.top+statusH);
	fTextScroll->ResizeTo(right-left,bounds.Height()-statusH-buttonH);

	float	buttonY	= bounds.bottom-buttonH+2;
	fApplyButton->MoveTo(left,buttonY);
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
		BStringItem	*commandItem	= new BStringItem(command->Name(),0,true);
		fCommandList->AddItem(commandItem);

		int32				propCount	= 0;
		const property_info	*props	= command->PropertyInfo(&propCount);
		for (int32 p = 0; p < propCount; p++) {
			for (int32 c = 0; c < 3; c++) {
				for (int32 f = 0; f < 5; f++) {
					const char	*fieldName	= props[p].ctypes[c].pairs[f].name;
					if (fieldName == NULL)
						continue;
					BString	label;
					label << fieldName << ": " << TypeDisplayName(props[p].ctypes[c].pairs[f].type);
					fCommandList->AddUnder(new BStringItem(label.String(),1,true),commandItem);
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


void MacroEditor::ValueChanged(void)
{
	TRACE();
	RefreshMacroList();
}


void MacroEditor::SetShortCutFilter(ShortCutFilter *_shortCutFilter)
{
	if (LockLooper()) {
		AddFilter(_shortCutFilter);
		UnlockLooper();
	}
}


void MacroEditor::RefreshMacroList(void)
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

	// BListView::Select() alone does not send the selection message (that
	// only happens for a real user click, or an explicit InvokeNotify) -
	// so this always calls ShowSelectedMacro() directly afterwards rather
	// than relying on a M_E_MACRO_SELECTED round-trip that may never
	// arrive. Falls back to showing the first macro (if any) when there
	// was nothing to preserve - never leaves a stale selection showing
	// after the underlying macro list changed under it.
	if (reselectIndex >= 0)
		fMacroList->Select(reselectIndex);
	else if (fMacroList->CountItems() > 0)
		fMacroList->Select(0);
	ShowSelectedMacro();
}


void MacroEditor::ShowSelectedMacro(void)
{
	if ((fTextView == NULL) || (doc == NULL))
		return;

	int32	index	= fMacroList->CurrentSelection();
	BList	*macroList	= doc->GetCommandManager()->GetMacroList();
	if ((index < 0) || (index >= macroList->CountItems())) {
		fSelectedMacro	= NULL;
		fTextView->SetText("");
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
	fTextView->SetText(text.String());
	SetStatus("",false);

	for (int32 c = 0; c < commands.CountItems(); c++)
		delete (BMessage*)commands.ItemAt(c);
}


void MacroEditor::ApplyEdits(void)
{
	if ((fSelectedMacro == NULL) || (doc == NULL)) {
		SetStatus(B_TRANSLATE("No macro selected."),true);
		return;
	}

	BString	text(fTextView->Text());
	BList		parsed;
	BString		error;
	status_t	err	= ParseCommands(text,&parsed,doc->GetCommandManager(),&error);
	if (err != B_OK) {
		SetStatus(error.String(),true);
		return;
	}

	fSelectedMacro->RemoveName("Macro::Commmand");
	for (int32 i = 0; i < parsed.CountItems(); i++) {
		BMessage	*cmd	= (BMessage*)parsed.ItemAt(i);
		fSelectedMacro->AddMessage("Macro::Commmand",cmd);
		delete cmd;
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
			ValueChanged();
			break;
		}
		case M_E_MACRO_SELECTED: {
			ShowSelectedMacro();
			break;
		}
		case M_E_APPLY: {
			ApplyEdits();
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

				BMessage	*newMacro	= new BMessage(P_C_MACRO_TYPE);
				newMacro->AddString("Name",name);
				BList	*macroList	= doc->GetCommandManager()->GetMacroList();
				macroList->AddItem(newMacro);
				BMenuItem	*item	= new BMenuItem(name.String(),newMacro);
				item->SetTarget(doc);
				doc->AddMenuItem(P_MENU_MACRO_PLAY,item);

				fSelectedMacro	= newMacro;
				RefreshMacroList();
				// RefreshMacroList()->ShowSelectedMacro() just serialized
				// the new (still empty) macro's real command list - this
				// overrides that with the raw imported DSL text instead,
				// which is only a draft until Apply commits it.
				fTextView->SetText(importedText.String());

				BString	status;
				status.SetToFormat(B_TRANSLATE("Imported as new macro \"%s\" - review, then Apply."),
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
