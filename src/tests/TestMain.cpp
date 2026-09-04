#include <app/Application.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include "IndexerTest.h"
#include "LayoutEditorTest.h"
#include "PCommandTest.h"

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
	bool success = runner.run("", false);
	return success ? 0 : 1;
}
