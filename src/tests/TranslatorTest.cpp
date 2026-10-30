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


void TranslatorTest::HtmlExportIsOneSelfContainedPage(void)
{
	BMessage	document	= GroupedDocument();
	// a name that would end the page's script block if left as it is
	BMessage	allNodes;
	document.FindMessage(P_C_DOC_ALLNODES, &allNodes);
	BMessage	node;
	allNodes.FindMessage(P_C_DOC_ENTRY, 1, &node);
	BMessage	data;
	node.FindMessage(P_C_NODE_DATA, &data);
	data.ReplaceString(P_C_NODE_NAME, "</script><b>x");
	node.ReplaceMessage(P_C_NODE_DATA, &data);
	allNodes.ReplaceMessage(P_C_DOC_ENTRY, 1, &node);
	document.ReplaceMessage(P_C_DOC_ALLNODES, &allNodes);

	BString	html	= Exported(document, P_C_HTML_TYPE);
	CPPUNIT_ASSERT(html.StartsWith("<!DOCTYPE html>"));
	// nothing loaded from elsewhere
	CPPUNIT_ASSERT(html.FindFirst(" src=") < 0);
	CPPUNIT_ASSERT(html.FindFirst("<link") < 0);
	CPPUNIT_ASSERT(html.FindFirst("class=\"pc-group\" id=\"n3\"") >= 0);
	CPPUNIT_ASSERT(html.FindFirst("class=\"pc-node\" id=\"n1\"") >= 0);
	CPPUNIT_ASSERT(html.FindFirst("data-from=\"1\" data-to=\"2\"") >= 0);
	// on the card (shortened to fit) and in full for the details panel
	CPPUNIT_ASSERT(html.FindFirst(">Owner</text>") >= 0);
	CPPUNIT_ASSERT(html.FindFirst("[\"Owner\",\"Ann & Bob\",\"text\"]") >= 0);
	// the name only escaped: once in the SVG, once in the data
	CPPUNIT_ASSERT(html.FindFirst("</script><b>") < 0);
	CPPUNIT_ASSERT(html.FindFirst("&lt;/script&gt;&lt;b&gt;x") >= 0);
	CPPUNIT_ASSERT(html.FindFirst("<\\/script><b>x") >= 0);
}


void TranslatorTest::SvgExportStandsAlone(void)
{
	BString	svg	= Exported(GroupedDocument(), P_C_SVG_TYPE);
	CPPUNIT_ASSERT(svg.StartsWith("<?xml"));
	CPPUNIT_ASSERT(svg.FindFirst("<svg xmlns=\"http://www.w3.org/2000/svg\"") >= 0);
	// its own style, the colors of both schemes
	CPPUNIT_ASSERT(svg.FindFirst("<style>") >= 0);
	CPPUNIT_ASSERT(svg.FindFirst("prefers-color-scheme:dark") >= 0);
	CPPUNIT_ASSERT(svg.FindFirst("<script") < 0);
	CPPUNIT_ASSERT(svg.EndsWith("</svg>\n"));
}


void TranslatorTest::MermaidExportWritesAFlowchart(void)
{
	BString	mermaid	= Exported(GroupedDocument(), P_C_MERMAID_TYPE);
	CPPUNIT_ASSERT(mermaid.StartsWith("flowchart "));
	// quotes in a label as Mermaid's own entity
	CPPUNIT_ASSERT(mermaid.FindFirst("subgraph g3 [\"Team #quot;A#quot;\"]") >= 0);
	CPPUNIT_ASSERT(mermaid.FindFirst("n1[\"<b>first</b><br/>Owner: Ann & Bob\"]") >= 0);
	CPPUNIT_ASSERT(mermaid.FindFirst("n1 --> n2") >= 0);
	// both nodes inside the subgraph
	CPPUNIT_ASSERT(mermaid.FindFirst("n2[") < mermaid.FindFirst("  end\n"));

	BString	markdown	= Exported(GroupedDocument(), P_C_MARKDOWN_TYPE);
	CPPUNIT_ASSERT(markdown.StartsWith("# "));
	CPPUNIT_ASSERT(markdown.FindFirst("```mermaid\nflowchart ") >= 0);
	CPPUNIT_ASSERT(markdown.EndsWith("```\n"));
}


// the document a translator makes of text
static BMessage Imported(const char *text)
{
	BMemoryIO	input(text, strlen(text));
	BMallocIO	output;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK, BTranslatorRoster::Default()->Translate(
		&input, NULL, NULL, &output, P_C_DOCUMENT_RAW_TYPE));
	output.Seek(0, SEEK_SET);
	BMessage	document;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK, document.Unflatten(&output));
	return document;
}


// the entry of a list whose Node::Data name is name
static BMessage Named(const BMessage &document, const char *list, const char *name)
{
	BMessage	all;
	document.FindMessage(list, &all);
	BMessage	entry;
	for (int32 i = 0; all.FindMessage(P_C_DOC_ENTRY, i, &entry) == B_OK; i++) {
		BMessage	data;
		entry.FindMessage(P_C_NODE_DATA, &data);
		if (strcmp(data.GetString(P_C_NODE_NAME, ""), name) == 0)
			return entry;
	}
	CPPUNIT_FAIL(BString("no node named ").Append(name).String());
	return BMessage();
}


void TranslatorTest::GraphMLRoundTrips(void)
{
	BString		graphml	= Exported(GroupedDocument(), P_C_GRAPHML_TYPE);
	CPPUNIT_ASSERT(graphml.FindFirst("<graphml ") >= 0);
	// yEd sees a group with both nodes inside
	CPPUNIT_ASSERT(graphml.FindFirst("<node id=\"n3\" yfiles.foldertype=\"group\">") >= 0);
	CPPUNIT_ASSERT(graphml.FindFirst("<graph id=\"n3:\"") < graphml.FindFirst("<node id=\"n1\">"));

	BMessage	back	= Imported(graphml.String());
	BMessage	first	= Named(back, P_C_DOC_ALLNODES, "first");
	BMessage	group	= Named(back, P_C_DOC_ALLNODES, "Team \"A\"");
	CPPUNIT_ASSERT_EQUAL((uint32)P_C_GROUP_TYPE, group.what);
	CPPUNIT_ASSERT_EQUAL(CountField(group, P_C_NODE_CHILD_IDS), (int32)2);
	CPPUNIT_ASSERT_EQUAL(group.GetInt32(P_C_NODE_ID, 0), first.GetInt32(P_C_NODE_PARENT, -1));
	CPPUNIT_ASSERT(first.GetRect(P_C_NODE_FRAME, BRect()) == BRect(10, 70, 110, 110));
	BMessage	data;
	first.FindMessage(P_C_NODE_DATA, &data);
	BMessage	owner;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK, data.FindMessage("Owner", &owner));
	CPPUNIT_ASSERT_EQUAL((uint32)B_STRING_TYPE, owner.what);
	CPPUNIT_ASSERT_EQUAL(BString("Ann & Bob"), BString(owner.GetString(P_C_ATTRIBUTE_VALUE, "")));
	BMessage	connections;
	back.FindMessage(P_C_DOC_ALLCONNECTIONS, &connections);
	CPPUNIT_ASSERT_EQUAL((int32)1, CountField(connections, P_C_DOC_ENTRY));
}


void TranslatorTest::GraphMLImportsOtherTools(void)
{
	// as NetworkX writes it: no places, an undirected graph, typed data
	BMessage	plain	= Imported(
		"<?xml version='1.0' encoding='utf-8'?>\n"
		"<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\">\n"
		"<key id=\"d0\" for=\"node\" attr.name=\"label\" attr.type=\"string\"/>\n"
		"<key id=\"d1\" for=\"node\" attr.name=\"weight\" attr.type=\"double\"/>\n"
		"<key id=\"d2\" for=\"node\" attr.name=\"done\" attr.type=\"boolean\"/>\n"
		"<graph edgedefault=\"undirected\">\n"
		"<node id=\"a\"><data key=\"d0\">Alpha</data><data key=\"d1\">2.5</data>"
			"<data key=\"d2\">true</data></node>\n"
		"<node id=\"b\"/>\n"
		"<edge source=\"a\" target=\"b\"/>\n"
		"</graph></graphml>\n");
	BMessage	alpha	= Named(plain, P_C_DOC_ALLNODES, "Alpha");
	// a node without a label is called by its id
	BMessage	b		= Named(plain, P_C_DOC_ALLNODES, "b");
	BRect		alphaFrame	= alpha.GetRect(P_C_NODE_FRAME, BRect());
	CPPUNIT_ASSERT(alphaFrame.IsValid());
	CPPUNIT_ASSERT(!alphaFrame.Intersects(b.GetRect(P_C_NODE_FRAME, BRect())));
	BMessage	data;
	alpha.FindMessage(P_C_NODE_DATA, &data);
	BMessage	weight;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK, data.FindMessage("weight", &weight));
	CPPUNIT_ASSERT_EQUAL((uint32)P_C_ATTRIBUTE_NUMBER, weight.what);
	CPPUNIT_ASSERT_EQUAL(2.5f, weight.GetFloat(P_C_ATTRIBUTE_VALUE, 0));
	BMessage	done;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK, data.FindMessage("done", &done));
	CPPUNIT_ASSERT(done.GetBool(P_C_ATTRIBUTE_VALUE, false));
	BMessage	connections;
	plain.FindMessage(P_C_DOC_ALLCONNECTIONS, &connections);
	BMessage	edge;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK, connections.FindMessage(P_C_DOC_ENTRY, &edge));
	CPPUNIT_ASSERT_EQUAL((int8)0, edge.GetInt8(P_C_NODE_CONNECTION_ARROWS, -1));

	// as yEd writes it: places, labels and a group in its graphics only
	BMessage	yEd	= Imported(
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\" "
			"xmlns:y=\"http://www.yworks.com/xml/graphml\">\n"
		"<key for=\"node\" id=\"d6\" yfiles.type=\"nodegraphics\"/>\n"
		"<graph edgedefault=\"directed\" id=\"G\">\n"
		"<node id=\"n0\" yfiles.foldertype=\"group\"><data key=\"d6\">"
			"<y:ProxyAutoBoundsNode><y:Realizers active=\"0\"><y:GroupNode>"
			"<y:Geometry height=\"200\" width=\"300\" x=\"0\" y=\"0\"/>"
			"<y:NodeLabel>Box</y:NodeLabel></y:GroupNode></y:Realizers>"
			"</y:ProxyAutoBoundsNode></data>\n"
		"<graph edgedefault=\"directed\" id=\"n0:\">\n"
		"<node id=\"n0::n0\"><data key=\"d6\"><y:ShapeNode>"
			"<y:Geometry height=\"30\" width=\"80\" x=\"20\" y=\"40\"/>"
			"<y:Fill color=\"#FFCC00\" transparent=\"false\"/>"
			"<y:NodeLabel>Inside</y:NodeLabel></y:ShapeNode></data></node>\n"
		"</graph></node>\n"
		"</graph></graphml>\n");
	BMessage	inside	= Named(yEd, P_C_DOC_ALLNODES, "Inside");
	BMessage	box		= Named(yEd, P_C_DOC_ALLNODES, "Box");
	CPPUNIT_ASSERT(inside.GetRect(P_C_NODE_FRAME, BRect()) == BRect(20, 40, 100, 70));
	CPPUNIT_ASSERT_EQUAL(box.GetInt32(P_C_NODE_ID, 0), inside.GetInt32(P_C_NODE_PARENT, -1));
	BMessage	pattern;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK, inside.FindMessage(P_C_NODE_PATTERN, &pattern));
	int32		packed	= 0;
	pattern.FindInt32(P_C_PATTERN_FILL_COLOR, &packed);
	rgb_color	fill;
	memcpy(&fill, &packed, sizeof(fill));
	CPPUNIT_ASSERT_EQUAL((int)0xcc, (int)fill.green);
}


void TranslatorTest::DrawioRoundTrips(void)
{
	BMessage	document	= GroupedDocument();
	// a note keeps its lines, a number its unit
	BMessage	allNodes;
	document.FindMessage(P_C_DOC_ALLNODES, &allNodes);
	BMessage	node;
	allNodes.FindMessage(P_C_DOC_ENTRY, 1, &node);
	BMessage	data;
	node.FindMessage(P_C_NODE_DATA, &data);
	BMessage	note(P_C_ATTRIBUTE_NOTE);
	note.AddString(P_C_ATTRIBUTE_NAME, "Next steps");
	note.AddString(P_C_ATTRIBUTE_VALUE, "call\tAnn\nwrite to Bob");
	data.AddMessage("Next steps", &note);
	BMessage	price(P_C_ATTRIBUTE_NUMBER);
	price.AddString(P_C_ATTRIBUTE_NAME, "Price");
	price.AddFloat(P_C_ATTRIBUTE_VALUE, 25.5);
	price.AddString(P_C_ATTRIBUTE_UNIT, "€");
	data.AddMessage("Price", &price);
	node.ReplaceMessage(P_C_NODE_DATA, &data);
	allNodes.ReplaceMessage(P_C_DOC_ENTRY, 1, &node);
	document.ReplaceMessage(P_C_DOC_ALLNODES, &allNodes);

	BString		drawio	= Exported(document, P_C_DRAWIO_TYPE);
	CPPUNIT_ASSERT(drawio.StartsWith("<mxfile"));
	// members of a container are placed relative to it
	CPPUNIT_ASSERT(drawio.FindFirst("vertex=\"1\" parent=\"n3\"") >= 0);
	CPPUNIT_ASSERT(drawio.FindFirst("Next_steps=\"call&#9;Ann&#10;write to Bob\"") >= 0);

	BMessage	back	= Imported(drawio.String());
	BMessage	second	= Named(back, P_C_DOC_ALLNODES, "second");
	BMessage	group	= Named(back, P_C_DOC_ALLNODES, "Team \"A\"");
	CPPUNIT_ASSERT_EQUAL((uint32)P_C_GROUP_TYPE, group.what);
	CPPUNIT_ASSERT_EQUAL(group.GetInt32(P_C_NODE_ID, 0), second.GetInt32(P_C_NODE_PARENT, -1));
	CPPUNIT_ASSERT(second.GetRect(P_C_NODE_FRAME, BRect()).LeftTop() == BPoint(10, 130));
	second.FindMessage(P_C_NODE_DATA, &data);
	BMessage	restored;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK, data.FindMessage("Next steps", &restored));
	CPPUNIT_ASSERT_EQUAL((uint32)P_C_ATTRIBUTE_NOTE, restored.what);
	CPPUNIT_ASSERT_EQUAL(BString("call\tAnn\nwrite to Bob"),
		BString(restored.GetString(P_C_ATTRIBUTE_VALUE, "")));
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK, data.FindMessage("Price", &restored));
	CPPUNIT_ASSERT_EQUAL((uint32)P_C_ATTRIBUTE_NUMBER, restored.what);
	CPPUNIT_ASSERT_EQUAL(25.5f, restored.GetFloat(P_C_ATTRIBUTE_VALUE, 0));
	CPPUNIT_ASSERT_EQUAL(BString("€"), BString(restored.GetString(P_C_ATTRIBUTE_UNIT, "")));
	BMessage	connections;
	back.FindMessage(P_C_DOC_ALLCONNECTIONS, &connections);
	CPPUNIT_ASSERT_EQUAL((int32)1, CountField(connections, P_C_DOC_ENTRY));
}


void TranslatorTest::DrawioImportsCompressedDiagrams(void)
{
	// as older draw.io versions saved: deflated, base64, URL-encoded
	BMessage	document	= Imported(
		"<mxfile host=\"app.diagrams.net\"><diagram id=\"x\" name=\"Page-1\">"
		"xVNBbsMgEHwNd2KkqNfYbXPqI0jYGCTsRWuc2L/vFkhSy03VSpV6MGJnd0bMYIRqumlPOtg3NOCF"
		"ehGqIcSYd93UgPeiks4I9SyqSvInqtcH3U3qyqAJ+vgTgs6Es/YjZGTng2WUaVvdBaHqvDLSYB4e"
		"4uzLMIu5MHBRn5z3DXqk1FBGw9PpmHhJHyjC9PCMCSoH3AN2EGnmkSshW5BzLosjeXEm2jJxxSy4"
		"1hbRbcH0kOv2JnzPgzclkq/jOaziqSHqVQyEY2/gg7P5G8uV/D/PsL7l3uyI8MJVjz3cLIJp4XuD"
		"LIMjHWHxs0VNLcRPAa9jIPA6uvNS/RemuLy/odRbPLF3"
		"</diagram></mxfile>");
	BMessage	alpha	= Named(document, P_C_DOC_ALLNODES, "Alpha & Co");
	Named(document, P_C_DOC_ALLNODES, "Beta");
	CPPUNIT_ASSERT(alpha.GetRect(P_C_NODE_FRAME, BRect()) == BRect(10, 20, 130, 80));
	BMessage	connections;
	document.FindMessage(P_C_DOC_ALLCONNECTIONS, &connections);
	BMessage	edge;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK, connections.FindMessage(P_C_DOC_ENTRY, &edge));
	CPPUNIT_ASSERT_EQUAL((int8)0, edge.GetInt8(P_C_NODE_CONNECTION_ARROWS, -1));
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
	CPPUNIT_ASSERT(strcmp(name, "HTMLTranslator") != 0);
	CPPUNIT_ASSERT(strcmp(name, "MermaidTranslator") != 0);
	CPPUNIT_ASSERT(strcmp(name, "GraphMLTranslator") != 0);
	CPPUNIT_ASSERT(strcmp(name, "DrawioTranslator") != 0);
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

	// a page is titled after the document
	BMessage	page;
	page.AddInt32(P_C_FORMAT_TYPE, P_C_HTML_TYPE);
	page.AddString(P_C_FORMAT_MIME, "text/html");
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK, doc->WriteTo(ref, page, true));
	BFile		html(&ref, B_READ_ONLY);
	off_t		size	= 0;
	html.GetSize(&size);
	BString		text;
	html.Read(text.LockBuffer(size), size);
	text.UnlockBuffer(size);
	BString		title;
	title << "<title>" << doc->Title() << "</title>";
	CPPUNIT_ASSERT(text.FindFirst(title) >= 0);
	entry.Remove();
}


void TranslatorTest::ImportedFileIsNotTheDocumentsFile(void)
{
	BFile		mindMap("/tmp/projectconceptor-import-test.mm",
		B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	mindMap.Write(kFreeMindSample, strlen(kFreeMindSample));
	BEntry		entry("/tmp/projectconceptor-import-test.mm");
	entry_ref	ref;
	CPPUNIT_ASSERT_EQUAL((status_t)B_OK, entry.GetRef(&ref));
	PDocument	*doc	= NewHeadlessTestDocument();
	doc->SetEntry(&ref, NULL);
	doc->Load();
	// Save asks for a new file instead of writing over the mind map,
	// offering its name
	CPPUNIT_ASSERT(doc->Entry() == NULL);
	CPPUNIT_ASSERT_EQUAL(BString("projectconceptor-import-test"), BString(doc->Title()));
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
