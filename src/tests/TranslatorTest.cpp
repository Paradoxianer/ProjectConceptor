#include "TranslatorTest.h"

#include <DataIO.h>
#include <Entry.h>
#include <File.h>
#include <NodeInfo.h>
#include <GraphicsDefs.h>
#include <Message.h>
#include <TranslatorRoster.h>

#include <String.h>

#include <string.h>

#include "PDocument.h"
#include "ProjectConceptorDefs.h"
#include "TestDocument.h"

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


// two nodes in a group, one with an attribute, connected
static BMessage GroupedDocument(void)
{
	BMessage	document	= TwoNodeDocument();
	BMessage	allNodes;
	document.FindMessage(P_C_DOC_ALLNODES, &allNodes);
	BMessage	node;
	allNodes.FindMessage(P_C_DOC_ENTRY, 0, &node);
	BMessage	data;
	node.FindMessage(P_C_NODE_DATA, &data);
	BMessage	attribute(B_STRING_TYPE);
	attribute.AddString(P_C_ATTRIBUTE_NAME, "Owner");
	attribute.AddString(P_C_ATTRIBUTE_VALUE, "Ann & Bob");
	data.AddMessage("Owner", &attribute);
	node.ReplaceMessage(P_C_NODE_DATA, &data);
	allNodes.ReplaceMessage(P_C_DOC_ENTRY, 0, &node);
	for (int32 i = 0; i < 2; i++) {
		allNodes.FindMessage(P_C_DOC_ENTRY, i, &node);
		node.AddInt32(P_C_NODE_PARENT, 3);
		allNodes.ReplaceMessage(P_C_DOC_ENTRY, i, &node);
	}
	BMessage	group(P_C_GROUP_TYPE);
	BMessage	groupData;
	groupData.AddString(P_C_NODE_NAME, "Team \"A\"");
	group.AddMessage(P_C_NODE_DATA, &groupData);
	group.AddRect(P_C_NODE_FRAME, BRect(0, 50, 120, 180));
	group.AddInt32(P_C_NODE_ID, 3);
	group.AddInt32(P_C_NODE_CHILD_IDS, 1);
	group.AddInt32(P_C_NODE_CHILD_IDS, 2);
	allNodes.AddMessage(P_C_DOC_ENTRY, &group);
	document.ReplaceMessage(P_C_DOC_ALLNODES, &allNodes);

	BMessage	allConnections;
	BMessage	connection(P_C_CONNECTION_TYPE);
	BMessage	connectionData;
	connectionData.AddString(P_C_NODE_NAME, "asks");
	connection.AddMessage(P_C_NODE_DATA, &connectionData);
	connection.AddInt32(P_C_NODE_ID, 4);
	connection.AddInt32(P_C_NODE_CONNECTION_FROM, 1);
	connection.AddInt32(P_C_NODE_CONNECTION_TO, 2);
	allConnections.AddMessage(P_C_DOC_ENTRY, &connection);
	document.ReplaceMessage(P_C_DOC_ALLCONNECTIONS, &allConnections);
	return document;
}


// the document written by the translator for outType, as text
static BString Exported(const BMessage &document, uint32 outType)
{
	BMallocIO	native;
	document.Flatten(&native);
	native.Seek(0, SEEK_SET);
	BMallocIO	output;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK, BTranslatorRoster::Default()->Translate(
		&native, NULL, NULL, &output, outType));
	return BString((const char *)output.Buffer(), output.BufferLength());
}


void TranslatorTest::DotExportWritesClustersLabelsAndEdges(void)
{
	BString	dot	= Exported(GroupedDocument(), P_C_DOT_TYPE);
	CPPUNIT_ASSERT(dot.StartsWith("digraph"));
	CPPUNIT_ASSERT(dot.FindFirst("subgraph cluster_3 {") >= 0);
	CPPUNIT_ASSERT(dot.FindFirst("label=\"Team \\\"A\\\"\"") >= 0);
	// the attribute as a row of the node's label, escaped for it
	CPPUNIT_ASSERT(dot.FindFirst("<B>first</B>") >= 0);
	CPPUNIT_ASSERT(dot.FindFirst("Ann &amp; Bob") >= 0);
	CPPUNIT_ASSERT(dot.FindFirst("<B>second</B>") >= 0);
	// both nodes inside the cluster, the edge after it
	CPPUNIT_ASSERT(dot.FindFirst("n2 [") < dot.FindFirst("\t}\n"));
	CPPUNIT_ASSERT(dot.FindFirst("n1 -> n2 [dir=forward]") >= 0);
}


void TranslatorTest::ExportFormatsNeverOpenDocuments(void)
{
	// opening a document asks every translator for the native format;
	// an export-only one must not answer
	BMessage		document	= TwoNodeDocument();
	BMallocIO		native;
	document.Flatten(&native);
	native.Seek(0, SEEK_SET);
	translator_info	info;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK, BTranslatorRoster::Default()->Identify(
		&native, NULL, &info, P_C_DOCUMENT_RAW_TYPE));
	const char		*name	= NULL;
	const char		*infoText	= NULL;
	int32			version	= 0;
	BTranslatorRoster::Default()->GetTranslatorInfo(info.translator, &name,
		&infoText, &version);
	CPPUNIT_ASSERT(strcmp(name, "DOTTranslator") != 0);
}


void TranslatorTest::ExportWritesACopyOfItsOwnType(void)
{
	PDocument	*doc	= NewHeadlessTestDocument();
	BMessage	format;
	format.AddInt32(P_C_FORMAT_TYPE, P_C_DOT_TYPE);
	format.AddString(P_C_FORMAT_MIME, "text/vnd.graphviz");
	BEntry		entry("/tmp/projectconceptor-export-test.dot");
	entry_ref	ref;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK, entry.GetRef(&ref));
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK, doc->WriteTo(ref, format, true));

	BFile		file(&ref, B_READ_ONLY);
	char		start[8]	= {0};
	CPPUNIT_ASSERT_EQUAL((ssize_t)7, file.Read(start, 7));
	CPPUNIT_ASSERT_EQUAL(BString("digraph"), BString(start));
	// opens in the app for its format, not in ProjectConceptor
	char		mime[B_MIME_TYPE_LENGTH];
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK, BNodeInfo(&file).GetType(mime));
	CPPUNIT_ASSERT_EQUAL(BString("text/vnd.graphviz"), BString(mime));
	entry.Remove();
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
