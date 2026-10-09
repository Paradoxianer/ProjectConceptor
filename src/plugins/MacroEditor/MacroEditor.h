#ifndef MACRO_EDITOR_H
#define MACRO_EDITOR_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <app/Message.h>
#include <interface/ListView.h>
#include <interface/MenuItem.h>
#include <interface/OutlineListView.h>
#include <interface/StringView.h>
#include <interface/View.h>
#include <storage/FilePanel.h>
#include <support/List.h>

#include "MacroOutlineView.h"
#include "PEditor.h"
#include "PDocument.h"
#include "ShortCutFilter.h"

const uint32	M_E_MACRO_SELECTED	= 'meMS';
/** Same import as Macro > Open, but by plain path string instead of a real
 * file panel selection - a scripting/smoke-test hook (see
 * ImportMacroFromOpenFile()), not reached from any menu. */
const uint32	M_E_IMPORT_MACRO_FILE	= 'meIF';

class BScrollView;
class PCommandManager;

/**
 * @class MacroEditor
 *
 * @brief View/edit already-recorded macros (#55) as a direct structural
 * tree (see MacroOutlineView) of a macro's "Macro::Commmand" list - no
 * text/DSL surface to edit anymore (still used for Save/Open, see
 * ExportToFile()/ImportFromFile()). Recording itself (Start/Stop Macro) is
 * untouched; this is a viewer/editor for what's already in
 * PCommandManager::GetMacroList().
 */
class MacroEditor : public PEditor, public BView
{

public:
							MacroEditor();

	//++++++++++++++++PEditor
	virtual	void			AttachedToManager(void);
	virtual	void			DetachedFromManager(void);

	virtual void			PreprocessBeforSave(BMessage *container) {};
	virtual void			PreprocessAfterLoad(BMessage *container) {};

	virtual	BView*			GetView(void) {return this;};
	virtual	const char*		TabLabel(void);
	virtual BHandler*		GetHandler(void) {return this;};

	virtual	BMessage*		GetConfiguration(void) {return configMessage;};
	virtual	void			SetConfiguration(BMessage *message)
								{delete configMessage;configMessage=message;};

	virtual	void			ValueChanged(BMessage *changedNodes);

	virtual	void			SetShortCutFilter(ShortCutFilter *_shortCutFilter);
	//----------------PEditor

	//++++++++++++++++BView
	virtual void			AttachedToWindow(void);
	virtual void			FrameResized(float width, float height);
	virtual	void			MessageReceived(BMessage *message);
	//----------------BView

	/** Writes fOutlineView's current command list into the selected macro's
	 * "Macro::Commmand" field and updates the status line - called by
	 * MacroOutlineView right after any edit (a field commits, a row is
	 * added/deleted/moved, a command is dropped in). There is no draft to
	 * apply: fOutlineView's own working copy is always already a fully
	 * valid, typed BMessage tree, so this never fails. Public so
	 * MacroOutlineView can call it. */
			void			CommitOutlineChange(void);
	/** Shows `text` as an error in the status line - MacroOutlineView calls
	 * this when a field's typed-in value doesn't parse (e.g. a string typed
	 * into a float field), instead of silently keeping the old value with
	 * no explanation. Public for the same reason CommitOutlineChange() is. */
			void			ShowOutlineError(const char *text);
	/** doc->GetCommandManager(), or NULL if there is no doc yet - what
	 * MacroOutlineView needs for every command-name/schema lookup, without
	 * needing to know about PDocument itself. Public for the same reason
	 * CommitOutlineChange() is. */
			PCommandManager*	CommandManagerForOutline(void);

protected:
			void			Init(void);
			/** Rebuilds the macro-name BListView from
			 * commandManager->GetMacroList(), preserving the selection by
			 * name if the previously selected macro still exists. */
			void			RefreshMacroList(bool reloadText = true);
			/** Registers an empty macro named `name` in the command
			 * manager's macro list and the Macro > Play submenu. Does not
			 * select it or touch the list view. */
			BMessage*		AddNewMacro(const BString &name, const BMessage *contentFrom = NULL);
			/** "New macro", "New macro 2", ... - first one no macro uses. */
			BString			UniqueMacroName(void);
			/** Macro > New: a fresh empty macro, selected. If text was typed
			 * with no macro selected, that text becomes the new macro
			 * instead of being thrown away. */
			void			NewMacro(void);
			/** Macro > Rename / Duplicate / Delete, on the selected macro. Keep
			 * the Macro > Play submenu in step with the macro list. */
			void			RenameSelectedMacro(void);
			void			DuplicateSelectedMacro(void);
			void			DeleteSelectedMacro(void);
			/** the Play submenu's entry for `macro` (its menu message is the
			 * macro itself), NULL if there is none */
			BMenuItem*		PlayItemFor(BMessage *macro);
			/** Loads the selected macro's "Macro::Commmand" list into
			 * fOutlineView. Clears the view (and the selected macro) if
			 * nothing is selected. */
			void			ShowSelectedMacro(void);
			/** Exports the currently selected macro's DSL text (#55) -
			 * reached from Macro > Save (MENU_MACRO_SAVE), not a button
			 * (see #55 follow-up: macro-management actions belong in the
			 * Macro menu, not duplicated as panel buttons). Shows a BAlert
			 * instead of the save panel if nothing is selected. Still text
			 * (SerializeCommands()) - a good diffable/shareable file format,
			 * even though nothing in the editor itself is text anymore. */
			void			ExportToFile(void);
			/** Imports a file as a brand new macro entry - reached from
			 * Macro > Open (MENU_MACRO_OPEN). Never touches whatever is
			 * currently selected; parses the file with ParseCommands() and,
			 * on success, installs the result directly as a new, selected
			 * macro - on failure, nothing is created and the error is
			 * shown in the status line. */
			void			ImportFromFile(void);
			/** Shared by the real B_REFS_RECEIVED (file panel/Tracker drop)
			 * and M_E_IMPORT_MACRO_FILE (scripting hook) paths: reads
			 * `file`, parses it, and on success installs the result as a
			 * new macro named `name` (deduplicated the same way
			 * UniqueMacroName() would if `name` is already taken - a
			 * repeated smoke-test import shouldn't pile up "Name",
			 * "Name 2", ... every run, it should just replace). On a parse
			 * error nothing is created and the error goes to the status
			 * line - never a half-imported macro. */
			void			ImportMacroFromOpenFile(BFile *file, const BString &name);
			void			SetStatus(const char *text, bool isError);
			void			LayoutChildren(void);
			/** Fills fCommandList from doc->GetCommandManager()'s registry -
			 * one top-level item per registered command (Name()), with its
			 * declared fields (PropertyInfo()/ctypes) as child items, so the
			 * DSL's command/field names are visible without leaving the
			 * editor. Built once, on first AttachedToWindow() - the command
			 * registry is loaded at startup and never changes afterward. */
			void			BuildCommandList(void);

			BMessage		*configMessage;

			BListView		*fMacroList;
			BScrollView		*fMacroListScroll;
			MacroOutlineView	*fOutlineView;
			BScrollView		*fOutlineScroll;
			BStringView		*fStatus;
			BOutlineListView	*fCommandList;
			BScrollView		*fCommandListScroll;

			BFilePanel		*fExportPanel;
			BFilePanel		*fImportPanel;

			/** The macro BMessage* (owned by commandManager->GetMacroList(),
			 * not by this view) currently shown/edited - NULL if none
			 * selected. */
			BMessage		*fSelectedMacro;
			/** macro list as of the last RefreshMacroList() - see ValueChanged(). */
			BList			fKnownMacros;

private:
};
#endif
