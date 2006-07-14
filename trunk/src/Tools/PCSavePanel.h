#ifndef P_C_SAVE_PANEL_H
#define P_C_SAVE_PANEL_H

#include <Application.h>
#include <TranslatorRoster.h>
#include <Menu.h>
#include <MenuItem.h>
#include <MenuField.h>
#include <FilePanel.h>
#include <Button.h>

#ifdef B_ZETA_VERSION_1_0_0
	#include <locale/Locale.h>
	#include <locale/LanguageNotifier.h>
#else
	#define _T(a) a
#endif 

class PCSavePanel : public BFilePanel
{
public:
				PCSavePanel(BMessage *msg);
	BMenu		*BuildFormatsMenu(void);
	//virtual void SelectionChanged();

protected:
	BMenuField	*format;
	entry_ref old_dir_ref,the_active_ref;
};

#endif
