#include "TestDocument.h"

#include <app/Message.h>

#include "PDocument.h"
#include "PDocumentManager.h"

PDocument* NewHeadlessTestDocument(void)
{
	// the archive constructor's Init(BMessage*) only creates documents for
	// "document" sub-messages it finds - an empty archive means none, so
	// this skips PDocumentManager()'s default-constructor path
	// (Init() + CreateDocument(), which would build a real window).
	PDocumentManager	*documentManager	= new PDocumentManager(new BMessage());
	return new PDocument(documentManager,true);
}
