#ifndef P_C_SAVE_PANEL_H
#define P_C_SAVE_PANEL_H

//#include "PluginManager.h"

#include <app/Application.h>
#include <interface/Menu.h>
#include <interface/MenuItem.h>
#include <interface/MenuField.h>
#include <FilePanel.h>
#include <interface/Button.h>
#include <TranslatorRoster.h>
#include <String.h>

#include <vector>

class FormatChangedHandler;

/** Save panel with a menu of the document formats the translators offer.
 * Saving lists the document's own formats, exporting the others. The file name follows the chosen format's extension.
 */
class PCSavePanel : public BFilePanel
{
public:
					PCSavePanel(BMessage *msg,  BMessenger* target = NULL,
						bool exporting = false);
					~PCSavePanel();
		BMenu		*BuildFormatsMenu(void);
	/** the formats a document is saved in, or those it is exported in,
	 * each a message with the P_C_FORMAT_* fields */
static	std::vector<BMessage>	Formats(bool exporting);
	/** the save or export format whose file name extension (without the
	 * dot, any case) is extension */
static	status_t	FindFormat(const char *extension, BMessage *format);
	//virtual void SelectionChanged();
virtual	void		SendMessage(const BMessenger*, BMessage*);
		void		FormatChanged(void);
protected:
	BMenuField		*formatMenu;
	entry_ref 		entryRef;
	BButton			*settings;
	bool			exporting;
	FormatChangedHandler	*formatHandler;
	/** the extension FormatChanged() added last, replaced on the next */
	BString			lastExtension;
};

#endif
