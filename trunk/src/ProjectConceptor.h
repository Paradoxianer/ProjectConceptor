#ifndef _ProjektConceptor_H_
#define _ProjektConceptor_H_

#include <Application.h>
#include <storage/FilePanel.h>
#include "PluginManager.h"
#include "PDocumentManager.h"
/**
 * @class ProjektConceptor
 * @brief Main Application Class
 *
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2005/10/04
 *
 * @todo Register MimeType and MimeType for the Dokumentenst also register supported mimetypes
 * @todo import/export interface and Translator support
 * @bug on QuitRequested we shoud not quit all Windows we schoud pass the BQuitRequested to all Documents
 */

class ProjektConceptor :	public BApplication
{
public:
								ProjektConceptor();
								~ProjektConceptor();

	virtual	void				ReadyToRun(void);
	virtual	bool				QuitRequested(void);
	virtual	void				MessageReceived(BMessage *message);
	virtual	void				RefsReceived(BMessage *message);
	virtual	void				AboutRequested(void);
	virtual void				ArgvReceived(int32 argc, char **argv);
private:
			PDocumentManager	*documentManager;
			BFilePanel			*openPanel;
};
#endif
