#ifndef MACRO_EDITOR_H
#define MACRO_EDITOR_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <app/Message.h>
#include <interface/ListView.h>
#include <interface/OutlineListView.h>
#include <interface/StringView.h>
#include <interface/View.h>
#include <storage/FilePanel.h>
#include <support/List.h>

#include "MacroTextView.h"
#include "PEditor.h"
#include "PDocument.h"
#include "ShortCutFilter.h"

const uint32	M_E_MACRO_SELECTED	= 'meMS';

class BScrollView;

/**
 * @class MacroEditor
 *
 * @brief View/edit already-recorded macros as text (#55) - a guided DSL
 * (see MacroText.h) round-tripping a macro's "Macro::Commmand" list.
 * Recording itself (Start/Stop Macro) is untouched; this is a viewer/
 * editor for what's already in PCommandManager::GetMacroList().
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

	/** Parses fTextView's current text and, on success, replaces the
	 * selected macro's stored commands with it - silently (see the status
	 * line for confirmation), never blocking further edits. On a parse
	 * error the stored macro is left untouched and the error is shown in
	 * the status line instead. No separate Apply button (#55 follow-up):
	 * MacroTextView calls this itself at natural pause points (Enter,
	 * losing focus) - public so it can. */
			void			ApplyEdits(void);

protected:
			void			Init(void);
			/** Rebuilds the macro-name BListView from
			 * commandManager->GetMacroList(), preserving the selection by
			 * name if the previously selected macro still exists. */
			void			RefreshMacroList(void);
			/** Serializes the selected macro's "Macro::Commmand" list into
			 * fTextView. Clears the view (and the selected macro) if
			 * nothing is selected. */
			void			ShowSelectedMacro(void);
			/** Exports the currently selected macro's text (#55) - reached
			 * from Macro > Save (MENU_MACRO_SAVE), not a button (see #55
			 * follow-up: macro-management actions belong in the Macro
			 * menu, not duplicated as panel buttons). Shows a BAlert
			 * instead of the save panel if nothing is selected. */
			void			ExportToFile(void);
			/** Imports a file as a brand new macro entry - reached from
			 * Macro > Open (MENU_MACRO_OPEN). Never touches whatever is
			 * currently selected; the import always becomes its own new
			 * list entry (named from the file), selected and shown for
			 * review - not committed into Macro::Commmand until the text
			 * is auto-applied (see ApplyEdits()). */
			void			ImportFromFile(void);
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
			MacroTextView	*fTextView;
			BScrollView		*fTextScroll;
			BStringView		*fStatus;
			BOutlineListView	*fCommandList;
			BScrollView		*fCommandListScroll;

			BFilePanel		*fExportPanel;
			BFilePanel		*fImportPanel;

			/** The macro BMessage* (owned by commandManager->GetMacroList(),
			 * not by this view) currently shown/edited - NULL if none
			 * selected. */
			BMessage		*fSelectedMacro;

private:
};
#endif
