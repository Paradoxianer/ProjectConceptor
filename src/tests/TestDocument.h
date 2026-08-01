#ifndef TEST_DOCUMENT_H
#define TEST_DOCUMENT_H

class PDocument;

/** Builds a PDocumentManager (no auto-created document - the archive
 * constructor with an empty BMessage skips CreateDocument()) and a
 * headless PDocument on top of it, for tests that need Indexer/PCommand
 * without an app_server. Caller does not own the returned PDocument's
 * lifetime in any special way - it's fine to just leak it for the
 * duration of a short test process. See docs/notes.md.
 */
PDocument* NewHeadlessTestDocument(void);

#endif
