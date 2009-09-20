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

class ProjektConceptor :	public BApplication {
    public:
								ProjektConceptor();
								~ProjektConceptor();

	virtual	void				ReadyToRun(void);
	virtual	bool				QuitRequested(void);
	virtual	void				MessageReceived(BMessage *message);
	virtual	void				RefsReceived(BMessage *message);
	virtual	void				AboutRequested(void);
	virtual void				ArgvReceived(int32 argc, char **argv);
	virtual	void				RegisterMime(void);
	virtual PDocumentManager		*GetPDocumentManager(void){return documentManager;}
	virtual ConfigManager			*GetConfigManager(void){return configManager;}
    private:
			PDocumentManager	*documentManager;
                        ConfigManager           *configManager;
                        BFilePanel		*openPanel;
};
#endif
