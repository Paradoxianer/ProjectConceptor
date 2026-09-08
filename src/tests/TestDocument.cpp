#include "TestDocument.h"

#include <app/Message.h>
#include <support/List.h>

#include "PDocument.h"
#include "PDocumentManager.h"

namespace {
	// every PDocumentManager NewHeadlessTestDocument() has created so far -
	// see CleanupTestDocuments()
	BList	sManagers;
}

PDocument* NewHeadlessTestDocument(void)
{
	// the archive constructor's Init(BMessage*) only creates documents for
	// "document" sub-messages it finds - an empty archive means none, so
	// this skips PDocumentManager()'s default-constructor path
	// (Init() + CreateDocument(), which would build a real window).
	PDocumentManager	*documentManager	= new PDocumentManager(new BMessage());
	sManagers.AddItem(documentManager);
	PDocument	*doc	= new PDocument(documentManager,true);
	// the headless constructor doesn't go through PDocumentManager::
	// CreateDocument(), the only place that otherwise registers a new
	// document into the manager's own list - without this,
	// ~PDocumentManager()'s cleanup loop (see CleanupTestDocuments()) has
	// nothing to iterate and never reaches this document's looper at all
	documentManager->AddDocument(doc);
	return doc;
}

void CleanupTestDocuments(void)
{
	for (int32 i=0; i<sManagers.CountItems(); i++)
		delete (PDocumentManager *)sManagers.ItemAt(i);
	sManagers.MakeEmpty();
}
