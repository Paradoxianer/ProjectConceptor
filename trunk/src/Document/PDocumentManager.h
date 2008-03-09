#ifndef P_DOCUMENT_MANAGER_H
#define P_DOCUMENT_MANAGER_H

#include "PDocument.h"
#include "PWindow.h"
#include "ConfigWindow.h"

#include <app/Roster.h>
#include <support/List.h>
#include <storage/Entry.h>
#include <storage/Path.h>
 /**
 * @class PDocumentManager
 *
 * @brief  Manges all dokuments opend by the ProjectConceptor
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2007/10/04
 * @Contact: mail@projectconceptor.de
 *
 *
 * @see BMessage
 */

class PDocumentManager : public BArchivable
{

public:
							PDocumentManager(void);
							PDocumentManager(BMessage *archive);
	virtual					~PDocumentManager();

	virtual	status_t		Archive(BMessage *into, bool deep = true) const;
	static	BArchivable		*Instantiate(BMessage *from);

	virtual	void			AddDocument(PDocument *doc);
	virtual	void			RemoveDocument(PDocument *doc);
	/**
	 * Creates a New Document
	 */
	virtual	PDocument*		CreateDocument(void);
	virtual	int32			CountPDocuments(void){return documentList->CountItems();};
	virtual	PDocument*		PDocumentAt(int32 index){return (PDocument *)documentList->ItemAt(index);};
	virtual PluginManager*	GetPluginManager(void){return pluginManager;};
	virtual ConfigWindow*	GetConfigWindow(void);
	//neeeed QuitRequested!!!!!
protected:
			void			Init(void);
			void			Init(BMessage *archive);

			BList*			documentList;
			PluginManager	*pluginManager;
			ConfigWindow	*configWindow;

private:
};
#endif
