#ifndef TEST_DOCUMENT_H
#define TEST_DOCUMENT_H

class PDocument;

/** Builds a PDocumentManager (no auto-created document - the archive
 * constructor with an empty BMessage skips CreateDocument()) and a
 * headless PDocument on top of it, for tests that need Indexer/PCommand
 * without an app_server. The returned PDocument's own object lifetime
 * needs no special handling - it's fine to just leak the pointer for the
 * duration of a short test process, since nothing here ever dereferences
 * it after the fact. Its *looper thread* is a different matter - see
 * CleanupTestDocuments(). See also docs/notes.md.
 */
PDocument* NewHeadlessTestDocument(void);

/** Quits every PDocument looper thread started by NewHeadlessTestDocument()
 * so far (via deleting each one's PDocumentManager, whose own destructor
 * already does this correctly - PDocumentManager::~PDocumentManager()
 * locks and Quit()s each document it still owns).
 *
 * Call once, after every test has run, right before main() returns
 * (issue #117): PDocument::Run() really does spawn a live BLooper thread,
 * and nothing was ever telling it to stop. A message still in that
 * thread's queue - or mid-dispatch - when the process itself exits can
 * crash into memory that's already being torn down. Reproduced exactly
 * as the issue describes: CppUnit prints "OK", the process returns 0,
 * and a separate debug_server crash report appears a few seconds later.
 */
void CleanupTestDocuments(void);

#endif
