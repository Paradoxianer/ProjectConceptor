#ifndef _ProjektConceptor_H_
#define _ProjektConceptor_H_

#include "PDocumentManager.h"
#include "ConfigManager.h"

#include <Application.h>
#include <storage/FilePanel.h>

/**
 * @class ProjektConceptor
 * @brief Main Application Class
 *
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2008/10/04
 *
 * @todo Register MimeType and MimeType for the Dokumentenst also register supported mimetypes
 * @todo import/export interface and Translator support
 * @bug on QuitRequested we shoud not quit all Windows we schoud pass the BQuitRequested to all Documents
 */

class ProjektConceptor : public BApplication {
    public:
									ProjektConceptor();
									~ProjektConceptor();

	virtual	bool					QuitRequested(void);
	virtual	void					MessageReceived(BMessage *message);
	virtual	void					RefsReceived(BMessage *message);
	virtual	void					AboutRequested(void);
	virtual	void					ArgvReceived(int32 argc, char **argv);
	virtual	void					RegisterMime(void);
	/** Publishes "Document" (by index) as a scripting specifier - the
	 * only way an external hey request reaches a PDocument at all, since
	 * PDocumentManager (which actually owns the document list) is a
	 * plain BArchivable, not a BHandler, and can't be a specifier hop
	 * itself. PDocument runs on its own BLooper, so this forwards the
	 * message via BMessenger and returns NULL rather than returning the
	 * PDocument pointer directly - the same pattern BApplication's own
	 * built-in "Window N" resolution uses to cross into a BWindow's
	 * looper. PDocument::ResolveSpecifier() then handles the next
	 * specifier itself (#55).
	 */
	virtual	status_t				GetSupportedSuites(BMessage *data);
	virtual	BHandler*				ResolveSpecifier(BMessage *message, int32 index,
										BMessage *specifier, int32 what, const char *property);
	virtual	PDocumentManager		*GetPDocumentManager(void){return documentManager;}
	virtual	ConfigManager			*GetConfigManager(void){return configManager;}
    private:
			PDocumentManager		*documentManager;
			ConfigManager			*configManager;
			BFilePanel				*openPanel;
};
#endif
