#include "MacroEditor.h"

#include <Catalog.h>
#include <interface/ListItem.h>
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
	fExportButton		= NULL;
	fImportButton		= NULL;
	fExportPanel		= NULL;
	fImportPanel		= NULL;
	fSelectedMacro		= NULL;
}


void MacroEditor::AttachedToWindow(void)
{
	TRACE();
	if (fMacroList != NULL)
		return;	// already built - a tab detach/reattach shouldn't duplicate children

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

	fApplyButton	= new BButton(BRect(0,0,80,24),"apply",B_TRANSLATE("Apply"),
		new BMessage(M_E_APPLY));
	fApplyButton->SetTarget(this);
	AddChild(fApplyButton);

	fExportButton	= new BButton(BRect(0,0,80,24),"export",B_TRANSLATE("Export..."),
		new BMessage(M_E_EXPORT));
	fExportButton->SetTarget(this);
	AddChild(fExportButton);

	fImportButton	= new BButton(BRect(0,0,80,24),"import",B_TRANSLATE("Import..."),
		new BMessage(M_E_IMPORT));
	fImportButton->SetTarget(this);
	AddChild(fImportButton);

	LayoutChildren();
	RefreshMacroList();
}


void MacroEditor::LayoutChildren(void)
{
	if (fMacroList == NULL)
		return;
	BRect	bounds	= Bounds();
	float	listW	= 140;
	float	buttonH	= 28;
	float	statusH	= 18;

	fMacroListScroll->MoveTo(bounds.left,bounds.top);
	fMacroListScroll->ResizeTo(listW,bounds.Height());

	float	right	= bounds.right;
	float	left	= bounds.left+listW+1;

	fStatus->MoveTo(left+4,bounds.top+2);
	fStatus->ResizeTo(right-left-8,statusH-4);

	fTextScroll->MoveTo(left,bounds.top+statusH);
	fTextScroll->ResizeTo(right-left,bounds.Height()-statusH-buttonH);

	float	buttonY	= bounds.bottom-buttonH+2;
	fApplyButton->MoveTo(left,buttonY);
	fExportButton->MoveTo(left+90,buttonY);
	fImportButton->MoveTo(left+180,buttonY);
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

	if (reselectIndex >= 0)
		fMacroList->Select(reselectIndex);
	else
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
		case M_E_MACRO_SELECTED: {
			ShowSelectedMacro();
			break;
		}
		case M_E_APPLY: {
			ApplyEdits();
			break;
		}
		case M_E_EXPORT: {
			ExportToFile();
			break;
		}
		case M_E_IMPORT: {
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
			entry_ref	ref;
			if (message->FindRef("refs",&ref) == B_OK) {
				BFile	file(&ref,B_READ_ONLY);
				off_t	size	= 0;
				file.GetSize(&size);
				char	*buffer	= new char[size+1];
				file.Read(buffer,size);
				buffer[size]	= '\0';
				fTextView->SetText(buffer);
				delete[] buffer;
				SetStatus(B_TRANSLATE("Imported - review, then Apply."),false);
			}
			break;
		}
		default:
			BView::MessageReceived(message);
			break;
	}
}
