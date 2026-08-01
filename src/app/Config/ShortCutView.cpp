#include "ShortCutView.h"

#include <interface/Alert.h>
#include <interface/Button.h>
#include <interface/GroupLayout.h>
#include <interface/InterfaceDefs.h>
#include <interface/MenuItem.h>
#include <interface/PopUpMenu.h>
#include <interface/ScrollView.h>
#include <interface/StringView.h>

#include "ConfigManager.h"
#include "KeyCaptureWindow.h"
#include "PCommandManager.h"
#include "PDocument.h"
#include "PDocumentManager.h"
#include "ProjectConceptor.h"
#include "ProjectConceptorDefs.h"


static void FormatKeyBinding(BString *out,int32 key,int32 modifiers)
{
	out->SetTo("");
	if (modifiers & B_COMMAND_KEY)
		*out << "Cmd+";
	if (modifiers & B_SHIFT_KEY)
		*out << "Shift+";
	if (modifiers & B_CONTROL_KEY)
		*out << "Ctrl+";
	if (modifiers & B_OPTION_KEY)
		*out << "Option+";
	switch (key) {
		case B_DELETE:		*out << "Delete";	break;
		case B_INSERT:		*out << "Insert";	break;
		case B_ESCAPE:		*out << "Esc";		break;
		case B_TAB:			*out << "Tab";		break;
		case B_SPACE:		*out << "Space";	break;
		case B_ENTER:		*out << "Enter";	break;
		default:
			if ((key >= 0x20) && (key < 0x7f))
				*out << (char)key;
			else
				*out << "<0x" << BString().SetToFormat("%x",(unsigned)key) << ">";
			break;
	}
}


ActionShortcutItem::ActionShortcutItem(const char *action,int32 key,int32 modifiers)
	:BStringItem(""),fAction(action),fKey(key),fModifiers(modifiers)
{
	_UpdateText();
}

void ActionShortcutItem::SetBinding(int32 key,int32 modifiers)
{
	fKey		= key;
	fModifiers	= modifiers;
	_UpdateText();
}

void ActionShortcutItem::_UpdateText(void)
{
	BString	binding;
	FormatKeyBinding(&binding,fKey,fModifiers);
	BString	text(fAction);
	text << ": " << binding;
	SetText(text.String());
}


MacroShortcutItem::MacroShortcutItem(const char *macroName,int32 key,int32 modifiers)
	:BStringItem(""),fMacroName(macroName),fKey(key),fModifiers(modifiers)
{
	_UpdateText();
}

void MacroShortcutItem::SetBinding(int32 key,int32 modifiers)
{
	fKey		= key;
	fModifiers	= modifiers;
	_UpdateText();
}

void MacroShortcutItem::_UpdateText(void)
{
	BString	binding;
	FormatKeyBinding(&binding,fKey,fModifiers);
	BString	text(fMacroName);
	text << ": " << binding;
	SetText(text.String());
}


ShortCutView::ShortCutView():BView("shortCutView",0)
{
	TRACE();
	fDoc	= NULL;
	Init();
}

void ShortCutView::Init(void)
{
	BGroupLayout	*layout	= new BGroupLayout(B_VERTICAL,5);
	SetLayout(layout);

	fActionList	= new BListView("actionShortcuts",B_SINGLE_SELECTION_LIST);
	fActionList->SetInvocationMessage(new BMessage(MSG_ACTION_LIST_INVOKED));
	fActionList->SetTarget(this);

	fMacroList	= new BListView("macroShortcuts",B_SINGLE_SELECTION_LIST);
	fMacroList->SetInvocationMessage(new BMessage(MSG_MACRO_LIST_INVOKED));
	fMacroList->SetTarget(this);

	BButton	*addMacroShortcut		= new BButton("addMacroShortcut","New...",new BMessage(MSG_ADD_MACRO_SHORTCUT));
	addMacroShortcut->SetTarget(this);
	BButton	*removeMacroShortcut	= new BButton("removeMacroShortcut","Remove",new BMessage(MSG_REMOVE_MACRO_SHORTCUT));
	removeMacroShortcut->SetTarget(this);

	BView	*macroButtonRow	= new BView("macroButtonRow",0);
	macroButtonRow->SetLayout(new BGroupLayout(B_HORIZONTAL,5));
	macroButtonRow->AddChild(addMacroShortcut);
	macroButtonRow->AddChild(removeMacroShortcut);

	AddChild(new BStringView("actionShortcutsLabel","Editor Shortcuts (double-click to change)"));
	AddChild(new BScrollView("actionShortcutsScroller",fActionList,0,false,true));

	AddChild(new BStringView("macroShortcutsLabel","Macro Shortcuts (double-click to change the key)"));
	AddChild(new BScrollView("macroShortcutsScroller",fMacroList,0,false,true));
	AddChild(macroButtonRow);

	_LoadActionShortcuts();
	_LoadMacroShortcuts();
}

void ShortCutView::AttachedToWindow(void)
{
	BView::AttachedToWindow();
	fActionList->SetTarget(this);
	fMacroList->SetTarget(this);
}

void ShortCutView::SetActiveDocument(PDocument *forDoc)
{
	fDoc	= forDoc;
	_LoadMacroShortcuts();
}

void ShortCutView::_LoadActionShortcuts(void)
{
	while (fActionList->CountItems() > 0)
		delete fActionList->RemoveItem((int32)0);

	// same fallback values as ConfigManager::_SetDefaults()/
	// GraphEditor::AttachedToManager() - a config file saved before an
	// action existed (or a stale one from an older build) shouldn't make
	// this list lie and show "no shortcut" for it
	static const char *actions[] = {
		P_C_SHORTCUT_ACTION_DELETE,
		P_C_SHORTCUT_ACTION_ADD_BOOL,
		P_C_SHORTCUT_ACTION_INSERT_NODE,
	};
	static const int32 defaultKeys[] = { B_DELETE, 'b', B_INSERT };
	static const int32 defaultModifiers[] = { 0, B_COMMAND_KEY, 0 };

	ConfigManager	*configManager	= ((ProjektConceptor*)be_app)->GetConfigManager();
	BMessage		*shortcuts		= configManager->GetConfigMessage(P_C_CONFIG_SHORTCUTS_FIELD);
	for (uint32 i = 0; i < (sizeof(actions)/sizeof(actions[0])); i++) {
		int32		key			= defaultKeys[i];
		int32		modifiers	= defaultModifiers[i];
		BMessage	binding;
		if ((shortcuts != NULL) && (shortcuts->FindMessage(actions[i],&binding) == B_OK)) {
			binding.FindInt32(P_C_SHORTCUT_KEY_FIELD,&key);
			binding.FindInt32(P_C_SHORTCUT_MODIFIERS_FIELD,&modifiers);
		}
		fActionList->AddItem(new ActionShortcutItem(actions[i],key,modifiers));
	}
	delete shortcuts;
}

void ShortCutView::_LoadMacroShortcuts(void)
{
	while (fMacroList->CountItems() > 0)
		delete fMacroList->RemoveItem((int32)0);

	ConfigManager	*configManager	= ((ProjektConceptor*)be_app)->GetConfigManager();
	BMessage		*macroShortcuts	= configManager->GetConfigMessage(P_C_CONFIG_MACRO_SHORTCUTS_FIELD);
	if (macroShortcuts == NULL)
		return;

	int32		key			= 0;
	int32		modifiers	= 0;
	const char	*macroName	= NULL;
	int32		i			= 0;
	while (macroShortcuts->FindInt32(P_C_SHORTCUT_KEY_FIELD,i,&key) == B_OK) {
		macroShortcuts->FindInt32(P_C_SHORTCUT_MODIFIERS_FIELD,i,&modifiers);
		macroShortcuts->FindString(P_C_MACRO_SHORTCUT_NAME_FIELD,i,&macroName);
		fMacroList->AddItem(new MacroShortcutItem(macroName,key,modifiers));
		i++;
	}
	delete macroShortcuts;
}

void ShortCutView::_SaveActionShortcuts(void)
{
	BMessage	shortcuts;
	for (int32 i = 0; i < fActionList->CountItems(); i++) {
		ActionShortcutItem	*item	= (ActionShortcutItem *)fActionList->ItemAt(i);
		BMessage	binding;
		binding.AddInt32(P_C_SHORTCUT_KEY_FIELD,item->Key());
		binding.AddInt32(P_C_SHORTCUT_MODIFIERS_FIELD,item->Modifiers());
		shortcuts.AddMessage(item->Action(),&binding);
	}
	ConfigManager	*configManager	= ((ProjektConceptor*)be_app)->GetConfigManager();
	configManager->SetConfigMessage(P_C_CONFIG_SHORTCUTS_FIELD,&shortcuts);
	configManager->SaveConfig();
	(new BAlert("Shortcuts","The new binding applies to documents opened from now on - already open windows keep the shortcuts they started with.","OK"))->Go();
}

void ShortCutView::_SaveMacroShortcuts(void)
{
	BMessage	macroShortcuts;
	for (int32 i = 0; i < fMacroList->CountItems(); i++) {
		MacroShortcutItem	*item	= (MacroShortcutItem *)fMacroList->ItemAt(i);
		macroShortcuts.AddInt32(P_C_SHORTCUT_KEY_FIELD,item->Key());
		macroShortcuts.AddInt32(P_C_SHORTCUT_MODIFIERS_FIELD,item->Modifiers());
		macroShortcuts.AddString(P_C_MACRO_SHORTCUT_NAME_FIELD,item->MacroName());
	}
	ConfigManager	*configManager	= ((ProjektConceptor*)be_app)->GetConfigManager();
	configManager->SetConfigMessage(P_C_CONFIG_MACRO_SHORTCUTS_FIELD,&macroShortcuts);
	configManager->SaveConfig();

	PDocumentManager	*docManager	= ((ProjektConceptor*)be_app)->GetPDocumentManager();
	for (int32 i = 0; i < docManager->CountPDocuments(); i++) {
		PDocument	*document	= docManager->PDocumentAt(i);
		if ((document != NULL) && (document->GetWindow() != NULL))
			document->GetWindow()->ReloadMacroShortcuts();
	}
}

void ShortCutView::_RebindAction(int32 index)
{
	ActionShortcutItem	*item	= (ActionShortcutItem *)fActionList->ItemAt(index);
	if (item == NULL)
		return;
	BString	prompt("Press the new key combination for \"");
	prompt << item->Action() << "\" (Esc to cancel).";
	KeyCaptureWindow	*capture	= new KeyCaptureWindow(prompt.String());
	int32	key			= 0;
	int32	modifiers	= 0;
	if (capture->Go(&key,&modifiers)) {
		item->SetBinding(key,modifiers);
		fActionList->InvalidateItem(index);
		_SaveActionShortcuts();
	}
}

void ShortCutView::_RebindMacroShortcut(int32 index)
{
	MacroShortcutItem	*item	= (MacroShortcutItem *)fMacroList->ItemAt(index);
	if (item == NULL)
		return;
	BString	prompt("Press the new key combination for the macro \"");
	prompt << item->MacroName() << "\" (Esc to cancel).";
	KeyCaptureWindow	*capture	= new KeyCaptureWindow(prompt.String());
	int32	key			= 0;
	int32	modifiers	= 0;
	if (capture->Go(&key,&modifiers)) {
		item->SetBinding(key,modifiers);
		fMacroList->InvalidateItem(index);
		_SaveMacroShortcuts();
	}
}

void ShortCutView::_AddMacroShortcut(void)
{
	if (fDoc == NULL) {
		(new BAlert("No document","Open Project settings from a document window to pick one of its macros.","OK"))->Go();
		return;
	}
	BList	*macroList	= fDoc->GetCommandManager()->GetMacroList();
	if ((macroList == NULL) || (macroList->CountItems() == 0)) {
		(new BAlert("No macros","This document has no recorded macros yet. Record one first (Macro menu > Start recording).","OK"))->Go();
		return;
	}

	BPopUpMenu	*menu	= new BPopUpMenu("macroNames");
	for (int32 i = 0; i < macroList->CountItems(); i++) {
		BMessage	*macro		= (BMessage *)macroList->ItemAt(i);
		const char	*macroName	= NULL;
		if (macro->FindString("Name",&macroName) == B_OK)
			menu->AddItem(new BMenuItem(macroName,NULL));
	}
	BPoint	where	= fMacroList->Frame().LeftBottom();
	ConvertToScreen(&where);
	BMenuItem	*chosen	= menu->Go(where);
	if (chosen == NULL) {
		delete menu;
		return;
	}
	BString	macroName(chosen->Label());
	delete menu;

	BString	prompt("Press the key combination for the macro \"");
	prompt << macroName << "\" (Esc to cancel).";
	KeyCaptureWindow	*capture	= new KeyCaptureWindow(prompt.String());
	int32	key			= 0;
	int32	modifiers	= 0;
	if (capture->Go(&key,&modifiers)) {
		fMacroList->AddItem(new MacroShortcutItem(macroName.String(),key,modifiers));
		_SaveMacroShortcuts();
	}
}

void ShortCutView::_RemoveSelectedMacroShortcut(void)
{
	int32	selection	= fMacroList->CurrentSelection();
	if (selection < 0)
		return;
	delete fMacroList->RemoveItem(selection);
	_SaveMacroShortcuts();
}

void ShortCutView::MessageReceived(BMessage *msg)
{
	switch (msg->what) {
		case MSG_ACTION_LIST_INVOKED: {
			_RebindAction(fActionList->CurrentSelection());
			break;
		}
		case MSG_MACRO_LIST_INVOKED: {
			_RebindMacroShortcut(fMacroList->CurrentSelection());
			break;
		}
		case MSG_ADD_MACRO_SHORTCUT: {
			_AddMacroShortcut();
			break;
		}
		case MSG_REMOVE_MACRO_SHORTCUT: {
			_RemoveSelectedMacroShortcut();
			break;
		}
		default:
			BView::MessageReceived(msg);
			break;
	}
}
