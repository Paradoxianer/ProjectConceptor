#ifndef MACRO_EDITOR_H
#define MACRO_EDITOR_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include <app/Message.h>
#include <interface/Button.h>
#include <interface/ListView.h>
#include <interface/StringView.h>
#include <interface/TextView.h>
#include <interface/View.h>
#include <storage/FilePanel.h>
#include <support/List.h>

#include "PEditor.h"
#include "PDocument.h"
#include "ShortCutFilter.h"

const uint32	M_E_MACRO_SELECTED	= 'meMS';
const uint32	M_E_APPLY			= 'meAP';
const uint32	M_E_EXPORT			= 'meEX';
const uint32	M_E_IMPORT			= 'meIM';

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

	virtual	void			ValueChanged(void);

	virtual	void			SetShortCutFilter(ShortCutFilter *_shortCutFilter);
	//----------------PEditor

	//++++++++++++++++BView
	virtual void			AttachedToWindow(void);
	virtual void			FrameResized(float width, float height);
	virtual	void			MessageReceived(BMessage *message);
	//----------------BView

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
			void			ApplyEdits(void);
			void			ExportToFile(void);
			void			ImportFromFile(void);
			void			SetStatus(const char *text, bool isError);
			void			LayoutChildren(void);

			BMessage		*configMessage;

			BListView		*fMacroList;
			BScrollView		*fMacroListScroll;
			BTextView		*fTextView;
			BScrollView		*fTextScroll;
			BStringView		*fStatus;
			BButton			*fApplyButton;
			BButton			*fExportButton;
			BButton			*fImportButton;

			BFilePanel		*fExportPanel;
			BFilePanel		*fImportPanel;

			/** The macro BMessage* (owned by commandManager->GetMacroList(),
			 * not by this view) currently shown/edited - NULL if none
			 * selected. */
			BMessage		*fSelectedMacro;

private:
};
#endif
