#ifndef TRANSLATOR_TEST_H
#define TRANSLATOR_TEST_H

#include <cppunit/extensions/HelperMacros.h>

/** The translators as other apps see them: through the system's
 * BTranslatorRoster, i.e. the built add-ons installed by ./dev.sh build.
 */
class TranslatorTest : public CppUnit::TestFixture
{
public:
	void FreeMindImportBuildsNodesAndConnections(void);
	void TextExportRoundTrips(void);
	void UnknownOutputTypeIsRefused(void);

	CPPUNIT_TEST_SUITE(TranslatorTest);
	CPPUNIT_TEST(FreeMindImportBuildsNodesAndConnections);
	CPPUNIT_TEST(TextExportRoundTrips);
	CPPUNIT_TEST(UnknownOutputTypeIsRefused);
	CPPUNIT_TEST_SUITE_END();
};

#endif
