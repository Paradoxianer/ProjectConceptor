#ifndef SHORTCUTVIEW_H
#define SHORTCUTVIEW_H

#include <app/Message.h>
#include <interface/ListView.h>
#include <interface/StringItem.h>
#include <interface/View.h>
#include <support/String.h>

class PDocument;

const uint32	MSG_ACTION_LIST_INVOKED		= 'SVai';
const uint32	MSG_MACRO_LIST_INVOKED		= 'SVmi';
const uint32	MSG_ADD_MACRO_SHORTCUT		= 'SVam';
const uint32	MSG_REMOVE_MACRO_SHORTCUT	= 'SVrm';

/** one row in the editor-action shortcuts list: a fixed action name (see
 * ProjectConceptorDefs.h P_C_SHORTCUT_ACTION_*) plus its current binding
 */
class ActionShortcutItem : public BStringItem
{
public:
						ActionShortcutItem(const char *action,int32 key,int32 modifiers);
			const char*	Action(void){return fAction.String();}
			int32		Key(void){return fKey;}
			int32		Modifiers(void){return fModifiers;}
			void		SetBinding(int32 key,int32 modifiers);
private:
			void		_UpdateText(void);
			BString		fAction;
			int32		fKey;
			int32		fModifiers;
};

/** one row in the macro shortcuts list: the bound macro name plus its
 * current key binding
 */
class MacroShortcutItem : public BStringItem
{
public:
						MacroShortcutItem(const char *macroName,int32 key,int32 modifiers);
			const char*	MacroName(void){return fMacroName.String();}
			int32		Key(void){return fKey;}
			int32		Modifiers(void){return fModifiers;}
			void		SetBinding(int32 key,int32 modifiers);
private:
			void		_UpdateText(void);
			BString		fMacroName;
			int32		fKey;
			int32		fModifiers;
};

/**
 * @class ShortCutView
 * @brief Settings tab for the app-wide editor action shortcuts
 * (ConfigManager's Shortcuts) and macro shortcuts (MacroShortcuts).
 */
class ShortCutView : public BView
{
public:
						ShortCutView();

			/** which document's macro list to offer when binding a new
			 * macro shortcut - the one whose "Project settings" menu
			 * item was actually used to open this dialog
			 */
			void		SetActiveDocument(PDocument *forDoc);
	virtual	void		AttachedToWindow(void);
	virtual	void		MessageReceived(BMessage *msg);

private:
			void		Init(void);
			void		_LoadActionShortcuts(void);
			void		_LoadMacroShortcuts(void);
			void		_SaveActionShortcuts(void);
			void		_SaveMacroShortcuts(void);
			void		_RebindAction(int32 index);
			void		_RebindMacroShortcut(int32 index);
			void		_AddMacroShortcut(void);
			void		_RemoveSelectedMacroShortcut(void);

			PDocument	*fDoc;
			BListView	*fActionList;
			BListView	*fMacroList;
};
#endif
