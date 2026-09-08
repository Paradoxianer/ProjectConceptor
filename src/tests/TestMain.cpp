#include <app/Application.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include "GroupBoundaryTest.h"
#include "IndexerTest.h"
#include "LayoutEditorTest.h"
#include "PCommandTest.h"
#include "TestDocument.h"

const char *TEST_APP_SIGNATURE = "application/x-vnd.ProjectConceptorTests";

int main(int argc, char **argv)
{
	// PDocumentManager::Init() calls be_app->GetAppInfo() unconditionally,
	// so Indexer/PCommand need a live BApplication to exist - this one
	// never Run()s or shows anything, it's here purely so be_app is valid.
	BApplication app(TEST_APP_SIGNATURE);

	CppUnit::TextUi::TestRunner runner;
	runner.addTest(IndexerTest::suite());
	runner.addTest(PCommandTest::suite());
	runner.addTest(LayoutEditorTest::suite());
	runner.addTest(GroupBoundaryTest::suite());
	bool success = runner.run("", false);
	// #117: every headless PDocument any test created is still running a
	// real BLooper thread at this point - quit them before main() returns,
	// or one still mid-dispatch crashes into memory this process is
	// already tearing down.
	CleanupTestDocuments();
	return success ? 0 : 1;
}
