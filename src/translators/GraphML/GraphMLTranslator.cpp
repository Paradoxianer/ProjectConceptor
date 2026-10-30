#include <TranslatorAddOn.h>
#include <TranslationKit.h>

#include <algorithm>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "AttributeText.h"
#include "ConfigView.h"
#include "GraphDocument.h"
#include "ProjectConceptorDefs.h"
#include "tinyxml.h"

/** Reads and writes GraphML (#122), the exchange format of yEd, Gephi,
 * NetworkX, Cytoscape and others. Besides the plain GraphML data the
 * export carries yEd's graphics (places, colors, labels), so yEd shows the
 * graph as it looks here; attribute kinds ride along as pc:kind on the
 * keys. The import takes names, places and colors from either. */

static const char	*kGraphMLMime	= "application/graphml+xml";
static const char	*kKindAttribute	= "pc:kind";

char	translatorName[]	= "GraphMLTranslator";
char	translatorInfo[]	= "Read and write GraphML graphs";
int32	translatorVersion	= 100;

translation_format inputFormats[] = {
	{ P_C_DOCUMENT_RAW_TYPE, B_TRANSLATOR_NONE, 0.4, 0.8, P_C_DOCUMENT_MIMETYPE, "ProjectConceptor RAW Document" },
	{ P_C_GRAPHML_TYPE, B_TRANSLATOR_TEXT, 0.4, 0.8, "application/graphml+xml", "GraphML" },
	{ 0, 0, 0, 0, "\0", "\0" }
};

translation_format outputFormats[] = {
	{ P_C_DOCUMENT_RAW_TYPE, B_TRANSLATOR_NONE, 0.4, 0.8, P_C_DOCUMENT_MIMETYPE, "ProjectConceptor RAW Document" },
	{ P_C_GRAPHML_TYPE, B_TRANSLATOR_TEXT, 0.4, 0.8, "application/graphml+xml", "GraphML" },
	{ 0, 0, 0, 0, "\0", "\0" }
};

// an imported node without a place: a grid of these
static const float	kImportWidth	= 160;
static const float	kImportHeight	= 60;
static const float	kImportGap		= 40;
static const int32	kImportColumns	= 5;
static const float	kGroupPadding	= 16;


static BString Number(float value)
{
	BString	text;
	text.SetToFormat("%.1f", value);
	return text;
}


// ---------------------------------------------------------------- export

class GraphMLWriter {
public:
					GraphMLWriter(const GraphDocument &document)
						: document(document) {}
	BString			Write(void);
private:
	void			_DecideKeys(void);
	void			_WriteMembers(int32 parent, const BString &indent);
	void			_WriteNode(const GraphNode &node, const BString &indent);

	const GraphDocument				&document;
	/** attribute name -> key id, its GraphML type and its kind word */
	std::map<BString, BString>		keyIds;
	std::map<BString, BString>		keyTypes;
	std::map<BString, uint32>		keyKinds;
	BString							out;
};


void GraphMLWriter::_DecideKeys(void)
{
	for (size_t n = 0; n < document.nodes.size(); n++) {
		const GraphNode	&node	= document.nodes[n];
		for (size_t a = 0; a < node.attributes.size(); a++) {
			const GraphAttribute	&attribute	= node.attributes[a];
			BString	type("string");
			if (attribute.kind == B_BOOL_TYPE)
				type	= "boolean";
			else if ((attribute.kind == P_C_ATTRIBUTE_NUMBER)
				&& (attribute.value.FindFirst(' ') < 0))
				type	= "double";
			std::map<BString, BString>::iterator	found	= keyIds.find(attribute.name);
			if (found == keyIds.end()) {
				BString	id;
				id << "a" << (int32)keyIds.size();
				keyIds[attribute.name]		= id;
				keyTypes[attribute.name]	= type;
				keyKinds[attribute.name]	= attribute.kind;
			} else {
				// one name, several kinds: text holds them all
				if (keyTypes[attribute.name] != type)
					keyTypes[attribute.name]	= "string";
				if (keyKinds[attribute.name] != attribute.kind)
					keyKinds[attribute.name]	= B_STRING_TYPE;
			}
		}
	}
}


BString GraphMLWriter::Write(void)
{
	_DecideKeys();
	out << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
		<< "<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\" "
			"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
			"xmlns:y=\"http://www.yworks.com/xml/graphml\" "
			"xmlns:pc=\"http://github.com/Paradoxianer/ProjectConceptor\" "
			"xsi:schemaLocation=\"http://graphml.graphdrawing.org/xmlns "
			"http://www.yworks.com/xml/schema/graphml/1.1/ygraphml.xsd\">\n"
		<< "  <key id=\"name\" for=\"node\" attr.name=\"name\" attr.type=\"string\"/>\n"
		<< "  <key id=\"x\" for=\"node\" attr.name=\"x\" attr.type=\"double\"/>\n"
		<< "  <key id=\"y\" for=\"node\" attr.name=\"y\" attr.type=\"double\"/>\n"
		<< "  <key id=\"width\" for=\"node\" attr.name=\"width\" attr.type=\"double\"/>\n"
		<< "  <key id=\"height\" for=\"node\" attr.name=\"height\" attr.type=\"double\"/>\n"
		<< "  <key id=\"color\" for=\"node\" attr.name=\"color\" attr.type=\"string\"/>\n"
		<< "  <key id=\"fill\" for=\"node\" attr.name=\"fill\" attr.type=\"string\"/>\n"
		<< "  <key id=\"arrows\" for=\"edge\" attr.name=\"pc.arrows\" attr.type=\"int\"/>\n"
		<< "  <key id=\"edgecolor\" for=\"edge\" attr.name=\"color\" attr.type=\"string\"/>\n";
	for (std::map<BString, BString>::iterator i = keyIds.begin(); i != keyIds.end(); i++) {
		const char	*kind	= AttributeKindName(keyKinds[i->first]);
		out << "  <key id=\"" << i->second << "\" for=\"node\" attr.name=\""
			<< XmlEscaped(i->first.String()) << "\" attr.type=\"" << keyTypes[i->first]
			<< "\" " << kKindAttribute << "=\"" << (kind != NULL ? kind : "text") << "\"/>\n";
	}
	out << "  <key id=\"ng\" for=\"node\" yfiles.type=\"nodegraphics\"/>\n"
		<< "  <key id=\"eg\" for=\"edge\" yfiles.type=\"edgegraphics\"/>\n"
		<< "  <graph id=\"G\" edgedefault=\"directed\">\n";
	_WriteMembers(-1, "    ");
	for (size_t i = 0; i < document.connections.size(); i++) {
		const GraphConnection	&connection	= document.connections[i];
		if ((document.FindNode(connection.from) == NULL)
			|| (document.FindNode(connection.to) == NULL)) {
			fprintf(stderr, "GraphMLTranslator: connection %ld to a missing node\n",
				(long)connection.id);
			continue;
		}
		BString	color(connection.hasColor ? HexColor(connection.color) : BString("#8c8c8c"));
		out << "    <edge id=\"e" << connection.id << "\" source=\"n" << connection.from
			<< "\" target=\"n" << connection.to << "\" directed=\""
			<< (connection.arrows == 0 ? "false" : "true") << "\">\n"
			<< "      <data key=\"arrows\">" << (int32)connection.arrows << "</data>\n";
		if (connection.hasColor)
			out << "      <data key=\"edgecolor\">" << color << "</data>\n";
		out << "      <data key=\"eg\"><y:PolyLineEdge><y:LineStyle color=\"" << color
			<< "\" type=\"line\" width=\"1.5\"/><y:Arrows source=\""
			<< ((connection.arrows & 2) != 0 ? "standard" : "none") << "\" target=\""
			<< ((connection.arrows & 1) != 0 ? "standard" : "none")
			<< "\"/></y:PolyLineEdge></data>\n"
			<< "    </edge>\n";
	}
	out << "  </graph>\n</graphml>\n";
	return out;
}


void GraphMLWriter::_WriteMembers(int32 parent, const BString &indent)
{
	for (size_t i = 0; i < document.nodes.size(); i++) {
		const GraphNode	&node	= document.nodes[i];
		// a parent that isn't in the document leaves the node on top
		int32			inside	= (document.FindNode(node.parent) != NULL)
			? node.parent : -1;
		if (inside == parent)
			_WriteNode(node, indent);
	}
}


void GraphMLWriter::_WriteNode(const GraphNode &node, const BString &indent)
{
	BRect	frame	= node.frame;
	out << indent << "<node id=\"n" << node.id << "\""
		<< (node.isGroup ? " yfiles.foldertype=\"group\"" : "") << ">\n"
		<< indent << "  <data key=\"name\">" << XmlEscaped(node.name.String()) << "</data>\n"
		<< indent << "  <data key=\"x\">" << Number(frame.left) << "</data>\n"
		<< indent << "  <data key=\"y\">" << Number(frame.top) << "</data>\n"
		<< indent << "  <data key=\"width\">" << Number(frame.Width()) << "</data>\n"
		<< indent << "  <data key=\"height\">" << Number(frame.Height()) << "</data>\n";
	if (node.hasBandColor)
		out << indent << "  <data key=\"color\">" << HexColor(node.bandColor) << "</data>\n";
	if (node.hasFillColor)
		out << indent << "  <data key=\"fill\">" << HexColor(node.fillColor) << "</data>\n";
	for (size_t a = 0; a < node.attributes.size(); a++) {
		const GraphAttribute	&attribute	= node.attributes[a];
		out << indent << "  <data key=\"" << keyIds[attribute.name] << "\">"
			<< XmlEscaped(attribute.value.String()) << "</data>\n";
	}
	BString	geometry;
	geometry << "<y:Geometry x=\"" << Number(frame.left) << "\" y=\"" << Number(frame.top)
		<< "\" width=\"" << Number(frame.Width()) << "\" height=\""
		<< Number(frame.Height()) << "\"/>";
	BString	fill(node.hasFillColor ? HexColor(node.fillColor) : BString("#ffffff"));
	if (node.isGroup) {
		out << indent << "  <data key=\"ng\"><y:ProxyAutoBoundsNode><y:Realizers active=\"0\">"
			<< "<y:GroupNode>" << geometry << "<y:Fill color=\""
			<< (node.hasFillColor ? fill : BString("#eef3f5")) << "\" transparent=\"false\"/>"
			<< "<y:BorderStyle color=\"#c8d8de\" type=\"line\" width=\"1.0\"/>"
			<< "<y:NodeLabel alignment=\"left\" modelName=\"internal\" modelPosition=\"tl\">"
			<< XmlEscaped(node.name.String()) << "</y:NodeLabel>"
			<< "<y:Shape type=\"roundrectangle\"/><y:State closed=\"false\"/></y:GroupNode>"
			<< "</y:Realizers></y:ProxyAutoBoundsNode></data>\n"
			<< indent << "  <graph id=\"n" << node.id << ":\" edgedefault=\"directed\">\n";
		BString	inner(indent);
		inner << "    ";
		_WriteMembers(node.id, inner);
		out << indent << "  </graph>\n";
	} else {
		static const char	*shapes[][2]	= {
			{ "rectangle", "rectangle" }, { "ellipse", "ellipse" },
			{ "diamond", "diamond" }, { "triangle", "triangle" },
			{ "hexagon", "hexagon" }, { "parallelogram", "parallelogram" },
			{ "note", "rectangle" } };
		const char	*shape		= "roundrectangle";
		const char	*shapeName	= node.shape.GetString(P_C_SHAPE_NAME, "rounded");
		for (size_t s = 0; s < sizeof(shapes) / sizeof(shapes[0]); s++) {
			if (strcmp(shapeName, shapes[s][0]) == 0)
				shape	= shapes[s][1];
		}
		// the label shows the attributes too, as the card does
		BString	label(node.name);
		for (size_t a = 0; a < node.attributes.size(); a++)
			label << "\n" << node.attributes[a].name << ": " << ShownValue(node.attributes[a]);
		out << indent << "  <data key=\"ng\"><y:ShapeNode>" << geometry
			<< "<y:Fill color=\"" << fill << "\" transparent=\"false\"/>"
			<< "<y:BorderStyle color=\""
			<< (node.hasBandColor ? HexColor(node.bandColor) : BString("#c4ccd0"))
			<< "\" type=\"line\" width=\"1.5\"/>"
			<< "<y:NodeLabel"
			<< (node.hasTextColor ? BString(" textColor=\"") << HexColor(node.textColor) << "\""
				: BString())
			<< ">" << XmlEscaped(label.String()) << "</y:NodeLabel>"
			<< "<y:Shape type=\"" << shape << "\"/></y:ShapeNode></data>\n";
	}
	out << indent << "</node>\n";
}


// ---------------------------------------------------------------- import

// "y:Geometry" -> "Geometry": other files may use other prefixes
static const char *LocalName(const TiXmlElement *element)
{
	const char	*name	= element->Value();
	const char	*colon	= strchr(name, ':');
	return (colon != NULL) ? colon + 1 : name;
}


static const TiXmlElement *FindDescendant(const TiXmlElement *parent, const char *name)
{
	for (const TiXmlElement *child = parent->FirstChildElement(); child != NULL;
			child = child->NextSiblingElement()) {
		if (strcmp(LocalName(child), name) == 0)
			return child;
		const TiXmlElement	*found	= FindDescendant(child, name);
		if (found != NULL)
			return found;
	}
	return NULL;
}


static bool ParseColor(const char *text, rgb_color *color)
{
	unsigned int	red, green, blue;
	if ((text == NULL) || (sscanf(text, "#%2x%2x%2x", &red, &green, &blue) != 3))
		return false;
	*color	= make_color(red, green, blue);
	return true;
}


static BString ElementText(const TiXmlElement *element)
{
	const char	*text	= element->GetText();
	return BString(text != NULL ? text : "");
}


struct GraphMLKey {
	BString		name;
	BString		type;
	BString		yfiles;
	uint32		kind;
};


class GraphMLReader {
public:
	status_t		Read(const char *xml, GraphDocument *document);
private:
	status_t		_ReadGraph(const TiXmlElement *graph, int32 parent, bool directed);
	status_t		_ReadNode(const TiXmlElement *element, int32 parent, bool directed);
	status_t		_ReadEdge(const TiXmlElement *element, bool directed);
	void			_ReadGraphics(const TiXmlElement *graphics, GraphNode *node,
						bool *hasPlace);
	int32			_IdFor(const char *xmlId);
	void			_PlaceUnplaced(void);

	GraphDocument					*document;
	std::map<BString, GraphMLKey>	keys;
	std::map<BString, int32>		ids;
	std::vector<int32>				unplaced;
	std::vector<std::pair<const TiXmlElement*, bool> >	edges;
	/** written by GraphMLWriter: its yEd graphics only repeat the data */
	bool							ownExport;
};


int32 GraphMLReader::_IdFor(const char *xmlId)
{
	std::map<BString, int32>::iterator	found	= ids.find(BString(xmlId));
	if (found != ids.end())
		return found->second;
	int32	id	= (int32)ids.size() + 1;
	ids[BString(xmlId)]	= id;
	return id;
}


status_t GraphMLReader::Read(const char *xml, GraphDocument *_document)
{
	document	= _document;
	TiXmlDocument	doc;
	// values keep their spaces and line breaks (notes); the setting is
	// shared by everything using the library, so it is put back
	bool			condensed	= TiXmlBase::IsWhiteSpaceCondensed();
	TiXmlBase::SetCondenseWhiteSpace(false);
	doc.Parse(xml);
	TiXmlBase::SetCondenseWhiteSpace(condensed);
	if (doc.Error()) {
		fprintf(stderr, "GraphMLTranslator: %s (line %d)\n", doc.ErrorDesc(),
			doc.ErrorRow());
		return B_BAD_DATA;
	}
	const TiXmlElement	*root	= doc.RootElement();
	if ((root == NULL) || (strcmp(LocalName(root), "graphml") != 0))
		return B_NO_TRANSLATOR;
	ownExport	= false;
	for (const TiXmlElement *key = root->FirstChildElement(); key != NULL;
			key = key->NextSiblingElement()) {
		if ((strcmp(LocalName(key), "key") != 0) || (key->Attribute("id") == NULL))
			continue;
		GraphMLKey	entry;
		entry.name		= key->Attribute("attr.name") ? key->Attribute("attr.name") : "";
		entry.type		= key->Attribute("attr.type") ? key->Attribute("attr.type") : "string";
		entry.yfiles	= key->Attribute("yfiles.type") ? key->Attribute("yfiles.type") : "";
		entry.kind		= B_STRING_TYPE;
		if (entry.type == "boolean")
			entry.kind	= B_BOOL_TYPE;
		else if ((entry.type == "int") || (entry.type == "long")
			|| (entry.type == "float") || (entry.type == "double"))
			entry.kind	= P_C_ATTRIBUTE_NUMBER;
		uint32	kind;
		if ((key->Attribute(kKindAttribute) != NULL)
			&& AttributeKindFromName(key->Attribute(kKindAttribute), &kind))
			entry.kind	= kind;
		keys[BString(key->Attribute("id"))]	= entry;
		ownExport |= (entry.name == "pc.arrows");
	}
	const TiXmlElement	*graph	= root->FirstChildElement("graph");
	if (graph == NULL)
		return B_BAD_DATA;
	const char	*edgeDefault	= graph->Attribute("edgedefault");
	status_t	err	= _ReadGraph(graph, -1,
		(edgeDefault == NULL) || (strcmp(edgeDefault, "undirected") != 0));
	for (size_t i = 0; (i < edges.size()) && (err == B_OK); i++)
		err = _ReadEdge(edges[i].first, edges[i].second);
	if (err == B_OK)
		_PlaceUnplaced();
	return err;
}


status_t GraphMLReader::_ReadGraph(const TiXmlElement *graph, int32 parent, bool directed)
{
	const char	*edgeDefault	= graph->Attribute("edgedefault");
	if (edgeDefault != NULL)
		directed	= (strcmp(edgeDefault, "undirected") != 0);
	for (const TiXmlElement *child = graph->FirstChildElement(); child != NULL;
			child = child->NextSiblingElement()) {
		if (strcmp(LocalName(child), "node") == 0) {
			status_t	err	= _ReadNode(child, parent, directed);
			if (err != B_OK)
				return err;
		} else if (strcmp(LocalName(child), "edge") == 0)
			// once all nodes have their ids
			edges.push_back(std::make_pair(child, directed));
	}
	return B_OK;
}


void GraphMLReader::_ReadGraphics(const TiXmlElement *graphics, GraphNode *node,
	bool *hasPlace)
{
	const TiXmlElement	*geometry	= FindDescendant(graphics, "Geometry");
	double				x, y, width, height;
	if ((geometry != NULL) && (geometry->Attribute("x", &x) != NULL)
		&& (geometry->Attribute("y", &y) != NULL)
		&& (geometry->Attribute("width", &width) != NULL)
		&& (geometry->Attribute("height", &height) != NULL)) {
		node->frame.Set(x, y, x + width, y + height);
		*hasPlace	= true;
	}
	const TiXmlElement	*label	= FindDescendant(graphics, "NodeLabel");
	if ((label != NULL) && (node->name.Length() == 0)) {
		// our own label adds the attributes below the name
		node->name	= ElementText(label);
		int32	newline	= node->name.FindFirst('\n');
		if (newline >= 0)
			node->name.Truncate(newline);
	}
	const TiXmlElement	*fill	= FindDescendant(graphics, "Fill");
	rgb_color			color;
	if (!node->hasFillColor && (fill != NULL)
		&& ((fill->Attribute("transparent") == NULL)
			|| (strcmp(fill->Attribute("transparent"), "true") != 0))
		&& ParseColor(fill->Attribute("color"), &color)
		&& !((color.red == 255) && (color.green == 255) && (color.blue == 255))) {
		node->fillColor		= color;
		node->hasFillColor	= true;
	}
}


status_t GraphMLReader::_ReadNode(const TiXmlElement *element, int32 parent, bool directed)
{
	if (element->Attribute("id") == NULL) {
		fprintf(stderr, "GraphMLTranslator: node without id\n");
		return B_BAD_DATA;
	}
	GraphNode	node;
	node.id		= _IdFor(element->Attribute("id"));
	node.parent	= parent;
	const TiXmlElement	*nested		= NULL;
	const TiXmlElement	*graphics	= NULL;
	double		place[4]	= { 0, 0, 0, 0 };
	bool		placed[4]	= { false, false, false, false };
	static const char	*placeNames[4]	= { "x", "y", "width", "height" };
	for (const TiXmlElement *child = element->FirstChildElement(); child != NULL;
			child = child->NextSiblingElement()) {
		if (strcmp(LocalName(child), "graph") == 0) {
			nested	= child;
			continue;
		}
		if ((strcmp(LocalName(child), "data") != 0) || (child->Attribute("key") == NULL))
			continue;
		std::map<BString, GraphMLKey>::iterator	key	= keys.find(BString(child->Attribute("key")));
		if (key == keys.end()) {
			fprintf(stderr, "GraphMLTranslator: data for unknown key %s\n",
				child->Attribute("key"));
			continue;
		}
		if (key->second.yfiles == "nodegraphics") {
			graphics	= child;
			continue;
		}
		if (key->second.yfiles.Length() > 0)
			continue;
		BString	name(key->second.name);
		BString	text(ElementText(child));
		bool	isPlace	= false;
		for (int32 p = 0; p < 4; p++) {
			if (name == placeNames[p]) {
				place[p]	= atof(text.String());
				placed[p]	= true;
				isPlace		= true;
			}
		}
		if (isPlace)
			continue;
		if ((name.ICompare("name") == 0) || (name.ICompare("label") == 0))
			node.name	= text;
		else if (name == "color")
			node.hasBandColor	= ParseColor(text.String(), &node.bandColor);
		else if (name == "fill")
			node.hasFillColor	= ParseColor(text.String(), &node.fillColor);
		else {
			GraphAttribute	attribute;
			attribute.name	= name.Length() > 0 ? name : BString(child->Attribute("key"));
			attribute.value	= text;
			attribute.kind	= key->second.kind;
			node.attributes.push_back(attribute);
		}
	}
	bool	hasPlace	= placed[0] && placed[1] && placed[2] && placed[3];
	if (hasPlace)
		node.frame.Set(place[0], place[1], place[0] + place[2], place[1] + place[3]);
	if ((graphics != NULL) && !ownExport)
		_ReadGraphics(graphics, &node, &hasPlace);
	// GraphML needs no label; the id is what the file calls the node then
	if (node.name.Length() == 0)
		node.name	= element->Attribute("id");
	node.isGroup	= (nested != NULL);
	if (!hasPlace)
		unplaced.push_back(node.id);
	document->nodes.push_back(node);
	if (nested != NULL)
		return _ReadGraph(nested, node.id, directed);
	return B_OK;
}


status_t GraphMLReader::_ReadEdge(const TiXmlElement *element, bool directed)
{
	const char	*source	= element->Attribute("source");
	const char	*target	= element->Attribute("target");
	if ((source == NULL) || (target == NULL)
		|| (ids.find(BString(source)) == ids.end())
		|| (ids.find(BString(target)) == ids.end())) {
		fprintf(stderr, "GraphMLTranslator: edge %s to an unknown node\n",
			element->Attribute("id") ? element->Attribute("id") : "");
		return B_BAD_DATA;
	}
	GraphConnection	connection;
	connection.id	= (int32)(ids.size() + document->connections.size() + 1);
	connection.from	= ids[BString(source)];
	connection.to	= ids[BString(target)];
	if (element->Attribute("directed") != NULL)
		directed	= (strcmp(element->Attribute("directed"), "false") != 0);
	connection.arrows	= directed ? 1 : 0;
	for (const TiXmlElement *child = element->FirstChildElement("data"); child != NULL;
			child = child->NextSiblingElement("data")) {
		if (child->Attribute("key") == NULL)
			continue;
		std::map<BString, GraphMLKey>::iterator	key	= keys.find(BString(child->Attribute("key")));
		if (key == keys.end())
			continue;
		if (key->second.name == "pc.arrows")
			connection.arrows	= (int8)(atoi(ElementText(child).String()) & 3);
		else if (key->second.name == "color")
			connection.hasColor	= ParseColor(ElementText(child).String(), &connection.color);
		else if ((key->second.yfiles == "edgegraphics") && !ownExport) {
			const TiXmlElement	*line	= FindDescendant(child, "LineStyle");
			rgb_color			color;
			if (!connection.hasColor && (line != NULL)
				&& ParseColor(line->Attribute("color"), &color)
				&& !((color.red == 0) && (color.green == 0) && (color.blue == 0))) {
				connection.color	= color;
				connection.hasColor	= true;
			}
		}
	}
	document->connections.push_back(connection);
	return B_OK;
}


void GraphMLReader::_PlaceUnplaced(void)
{
	// plain nodes in a grid below everything placed, then each group
	// around its members, the innermost first
	float	top		= 0;
	bool	any		= false;
	for (size_t i = 0; i < document->nodes.size(); i++) {
		bool	isUnplaced	= false;
		for (size_t u = 0; u < unplaced.size(); u++)
			isUnplaced |= (unplaced[u] == document->nodes[i].id);
		if (!isUnplaced) {
			top	= any ? std::max(top, document->nodes[i].frame.bottom + kImportGap)
				: document->nodes[i].frame.bottom + kImportGap;
			any	= true;
		}
	}
	int32	cell	= 0;
	for (size_t u = 0; u < unplaced.size(); u++) {
		for (size_t i = 0; i < document->nodes.size(); i++) {
			GraphNode	&node	= document->nodes[i];
			if ((node.id != unplaced[u]) || node.isGroup)
				continue;
			float	left	= (cell % kImportColumns) * (kImportWidth + kImportGap);
			float	y		= top + (cell / kImportColumns) * (kImportHeight + kImportGap);
			node.frame.Set(left, y, left + kImportWidth, y + kImportHeight);
			cell++;
		}
	}
	for (int32 pass = 0; pass < 64; pass++) {
		bool	changed	= false;
		for (size_t u = 0; u < unplaced.size(); u++) {
			for (size_t i = 0; i < document->nodes.size(); i++) {
				GraphNode	&group	= document->nodes[i];
				if ((group.id != unplaced[u]) || !group.isGroup)
					continue;
				BRect	frame;
				for (size_t m = 0; m < document->nodes.size(); m++) {
					if (document->nodes[m].parent == group.id)
						frame	= frame.IsValid() ? (frame | document->nodes[m].frame)
							: document->nodes[m].frame;
				}
				if (!frame.IsValid())
					frame.Set(0, top, kImportWidth, top + kImportHeight);
				frame.InsetBy(-kGroupPadding, -kGroupPadding);
				frame.top	-= kGroupPadding;
				if (frame != group.frame) {
					group.frame	= frame;
					changed		= true;
				}
			}
		}
		if (!changed)
			break;
	}
}


// ---------------------------------------------------------------- hooks

static bool LooksLikeGraphML(BPositionIO *in)
{
	char	start[1024];
	off_t	position	= in->Position();
	ssize_t	bytesRead	= in->Read(start, sizeof(start) - 1);
	in->Seek(position, SEEK_SET);
	if (bytesRead <= 0)
		return false;
	start[bytesRead]	= '\0';
	return strstr(start, "<graphml") != NULL;
}


status_t Identify(BPositionIO *inSource, const translation_format *inFormat,
	BMessage *ioExtension, translator_info *outInfo, uint32 outType)
{
	if (((outType == 0) || (outType == P_C_DOCUMENT_RAW_TYPE))
		&& LooksLikeGraphML(inSource)) {
		outInfo->type		= P_C_GRAPHML_TYPE;
		outInfo->group		= B_TRANSLATOR_TEXT;
		outInfo->quality	= 0.4;
		outInfo->capability	= 0.8;
		strcpy(outInfo->name, "GraphML");
		strcpy(outInfo->MIME, kGraphMLMime);
		return B_OK;
	}
	if ((outType == P_C_GRAPHML_TYPE) && LooksLikeDocument(inSource)) {
		outInfo->type		= P_C_DOCUMENT_RAW_TYPE;
		outInfo->group		= B_TRANSLATOR_NONE;
		outInfo->quality	= 0.4;
		outInfo->capability	= 0.8;
		strcpy(outInfo->name, "ProjectConceptor RAW Document");
		strcpy(outInfo->MIME, P_C_DOCUMENT_MIMETYPE);
		return B_OK;
	}
	return B_NO_TRANSLATOR;
}


status_t Translate(BPositionIO *inSource, const translator_info *tInfo,
	BMessage *ioExtension, uint32 outType, BPositionIO *outDestination)
{
	inSource->Seek(0, SEEK_SET);
	if (outType == P_C_GRAPHML_TYPE) {
		GraphDocument	document;
		status_t		err	= document.Read(inSource);
		if (err != B_OK)
			return err;
		BString	text	= GraphMLWriter(document).Write();
		ssize_t	written	= outDestination->Write(text.String(), text.Length());
		if (written < 0)
			return (status_t)written;
		return (written == text.Length()) ? B_OK : B_IO_ERROR;
	}
	if ((outType != 0) && (outType != P_C_DOCUMENT_RAW_TYPE))
		return B_NO_TRANSLATOR;
	off_t	size	= inSource->Seek(0, SEEK_END);
	inSource->Seek(0, SEEK_SET);
	if (size <= 0)
		return B_NO_TRANSLATOR;
	char	*xml		= new char[size + 1];
	ssize_t	bytesRead	= inSource->Read(xml, size);
	xml[bytesRead > 0 ? bytesRead : 0]	= '\0';
	GraphDocument	document;
	GraphMLReader	reader;
	status_t		err	= reader.Read(xml, &document);
	delete[] xml;
	if (err != B_OK)
		return err;
	return document.Write(outDestination);
}


status_t MakeConfig(BMessage *ioExtension, BView **outView, BRect *outExtent)
{
	*outView	= new ConfigView(translatorName);
	return B_OK;
}


status_t GetConfigMessage(BMessage *ioExtension)
{
	ioExtension->AddInt32(P_C_FORMAT_TYPE, P_C_GRAPHML_TYPE);
	ioExtension->AddString(P_C_FORMAT_EXTENSION, "graphml");
	return B_OK;
}
