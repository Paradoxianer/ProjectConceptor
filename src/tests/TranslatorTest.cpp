#include "TranslatorTest.h"

#include <DataIO.h>
#include <GraphicsDefs.h>
#include <Message.h>
#include <TranslatorRoster.h>

#include <string.h>

#include "ProjectConceptorDefs.h"

static const char *kFreeMindSample =
	"<map version=\"0.9.0\">\n"
	"<node ID=\"ID_100\" TEXT=\"Root\" BACKGROUND_COLOR=\"#ffcc00\">\n"
	"<node ID=\"ID_200\" POSITION=\"right\" TEXT=\"Child A\">\n"
	"<font NAME=\"SansSerif\" SIZE=\"12\"/>\n"
	"<node ID=\"ID_400\" TEXT=\"Grandchild\"/>\n"
	"</node>\n"
	"<node ID=\"ID_300\" POSITION=\"left\" TEXT=\"Child B\">\n"
	"<arrowlink DESTINATION=\"ID_200\" ID=\"Freemind_Arrow_Link_1\"/>\n"
	"</node>\n"
	"</node>\n"
	"</map>\n";


static int32 CountField(const BMessage &message, const char *name)
{
	type_code	type;
	int32		count	= 0;
	if (message.GetInfo(name, &type, &count) != B_OK)
		return 0;
	return count;
}


static BMessage TwoNodeDocument(void)
{
	BMessage	document;
	BMessage	allNodes;
	BMessage	allConnections;
	for (int32 i = 1; i <= 2; i++) {
		BMessage	node(P_C_CLASS_TYPE);
		BMessage	data;
		data.AddString(P_C_NODE_NAME, i == 1 ? "first" : "second");
		node.AddMessage(P_C_NODE_DATA, &data);
		node.AddRect(P_C_NODE_FRAME, BRect(10, 10 + i * 60, 110, 50 + i * 60));
		node.AddInt32("this", i);
		allNodes.AddMessage("node", &node);
	}
	document.AddInt32(P_C_DOC_FORMAT_VERSION_FIELD, P_C_DOC_FORMAT_VERSION);
	document.AddMessage("PDocument::allNodes", &allNodes);
	document.AddMessage("PDocument::allConnections", &allConnections);
	return document;
}


void TranslatorTest::FreeMindImportBuildsNodesAndConnections(void)
{
	BMemoryIO	input(kFreeMindSample, strlen(kFreeMindSample));
	BMallocIO	output;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK, BTranslatorRoster::Default()->Translate(
		&input, NULL, NULL, &output, P_C_DOCUMENT_RAW_TYPE));

	BMessage	document;
	output.Seek(0, SEEK_SET);
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK, document.Unflatten(&output));
	int32		version	= 0;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,
		document.FindInt32(P_C_DOC_FORMAT_VERSION_FIELD, &version));
	CPPUNIT_ASSERT_EQUAL((int32)P_C_DOC_FORMAT_VERSION, version);

	BMessage	allNodes;
	BMessage	allConnections;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,
		document.FindMessage("PDocument::allNodes", &allNodes));
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,
		document.FindMessage("PDocument::allConnections", &allConnections));
	CPPUNIT_ASSERT_EQUAL((int32)4, CountField(allNodes, "node"));
	// three tree edges plus the arrow link
	CPPUNIT_ASSERT_EQUAL((int32)4, CountField(allConnections, "node"));

	BMessage	root;
	BMessage	pattern;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK, allNodes.FindMessage("node", 3, &root));
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK, root.FindMessage(P_C_NODE_PATTERN, &pattern));
	int32		packed	= 0;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK, pattern.FindInt32("FillColor", &packed));
	rgb_color	fill;
	memcpy(&fill, &packed, sizeof(fill));
	CPPUNIT_ASSERT_EQUAL((int)0xff, (int)fill.red);
	CPPUNIT_ASSERT_EQUAL((int)0xcc, (int)fill.green);
	CPPUNIT_ASSERT_EQUAL((int)0x00, (int)fill.blue);
}


void TranslatorTest::TextExportRoundTrips(void)
{
	BMessage	document	= TwoNodeDocument();
	BMallocIO	native;
	document.Flatten(&native);
	BMallocIO	text;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK, BTranslatorRoster::Default()->Translate(
		&native, NULL, NULL, &text, P_C_DOCUMENT_TEXT_TYPE));
	CPPUNIT_ASSERT(text.BufferLength() > 5);
	CPPUNIT_ASSERT(memcmp(text.Buffer(), "<?xml", 5) == 0);

	BMallocIO	back;
	text.Seek(0, SEEK_SET);
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK, BTranslatorRoster::Default()->Translate(
		&text, NULL, NULL, &back, P_C_DOCUMENT_RAW_TYPE));
	BMessage	restored;
	back.Seek(0, SEEK_SET);
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK, restored.Unflatten(&back));
	BMessage	allNodes;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK,
		restored.FindMessage("PDocument::allNodes", &allNodes));
	CPPUNIT_ASSERT_EQUAL((int32)2, CountField(allNodes, "node"));
}


void TranslatorTest::UnknownOutputTypeIsRefused(void)
{
	BMessage	document	= TwoNodeDocument();
	BMallocIO	native;
	document.Flatten(&native);
	BMallocIO	output;
	CPPUNIT_ASSERT(BTranslatorRoster::Default()->Translate(
		&native, NULL, NULL, &output, 'PNG ') != B_OK);
	CPPUNIT_ASSERT_EQUAL((size_t)0, output.BufferLength());
}
