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
	void DotExportWritesClustersLabelsAndEdges(void);
	void ExportFormatsNeverOpenDocuments(void);
	void HtmlExportIsOneSelfContainedPage(void);
	void SvgExportStandsAlone(void);
	void MermaidExportWritesAFlowchart(void);
	void GraphMLRoundTrips(void);
	void GraphMLImportsOtherTools(void);
	void DrawioRoundTrips(void);
	void DrawioImportsCompressedDiagrams(void);
	void ExportWritesACopyOfItsOwnType(void);
	void ImportedFileIsNotTheDocumentsFile(void);
	void SaveAndExportOfferTheRightFormats(void);

	CPPUNIT_TEST_SUITE(TranslatorTest);
	CPPUNIT_TEST(FreeMindImportBuildsNodesAndConnections);
	CPPUNIT_TEST(TextExportRoundTrips);
	CPPUNIT_TEST(UnknownOutputTypeIsRefused);
	CPPUNIT_TEST(DotExportWritesClustersLabelsAndEdges);
	CPPUNIT_TEST(ExportFormatsNeverOpenDocuments);
	CPPUNIT_TEST(HtmlExportIsOneSelfContainedPage);
	CPPUNIT_TEST(SvgExportStandsAlone);
	CPPUNIT_TEST(MermaidExportWritesAFlowchart);
	CPPUNIT_TEST(GraphMLRoundTrips);
	CPPUNIT_TEST(GraphMLImportsOtherTools);
	CPPUNIT_TEST(DrawioRoundTrips);
	CPPUNIT_TEST(DrawioImportsCompressedDiagrams);
	CPPUNIT_TEST(ExportWritesACopyOfItsOwnType);
	CPPUNIT_TEST(ImportedFileIsNotTheDocumentsFile);
	CPPUNIT_TEST(SaveAndExportOfferTheRightFormats);
	CPPUNIT_TEST_SUITE_END();
};

#endif
