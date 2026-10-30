#include <TranslatorAddOn.h>
#include <TranslationKit.h>

#include <algorithm>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <zlib.h>

#include "AttributeText.h"
#include "ConfigView.h"
#include "GraphDocument.h"
#include "ProjectConceptorDefs.h"
#include "tinyxml.h"

/** Reads and writes draw.io / diagrams.net files (#165). draw.io edits the
 * graph further in any browser and exports to Visio, PDF and PNG. Nodes
 * become shapes labelled like the cards (name in bold, attributes below)
 * and keep their attributes as the shape's data; groups become
 * containers. The import also reads the compressed diagrams draw.io used
 * to write. */

static const char	*kDrawioMime	= "application/vnd.jgraph.mxfile";
// draw.io keeps style keys it doesn't know: the band's color rides along
static const char	*kBandStyle		= "pcBand";
// and data it doesn't know: the attributes' real names and kinds, one
// "kind<tab>name" line each, as their data names are cut to XML's rules
static const char	*kAttributesData	= "pcAttributes";

char	translatorName[]	= "DrawioTranslator";
char	translatorInfo[]	= "Read and write draw.io diagrams";
int32	translatorVersion	= 100;

translation_format inputFormats[] = {
	{ P_C_DOCUMENT_RAW_TYPE, B_TRANSLATOR_NONE, 0.4, 0.8, P_C_DOCUMENT_MIMETYPE, "ProjectConceptor RAW Document" },
	{ P_C_DRAWIO_TYPE, B_TRANSLATOR_TEXT, 0.4, 0.8, "application/vnd.jgraph.mxfile", "draw.io diagram" },
	{ 0, 0, 0, 0, "\0", "\0" }
};

translation_format outputFormats[] = {
	{ P_C_DOCUMENT_RAW_TYPE, B_TRANSLATOR_NONE, 0.4, 0.8, P_C_DOCUMENT_MIMETYPE, "ProjectConceptor RAW Document" },
	{ P_C_DRAWIO_TYPE, B_TRANSLATOR_TEXT, 0.4, 0.8, "application/vnd.jgraph.mxfile", "draw.io diagram" },
	{ 0, 0, 0, 0, "\0", "\0" }
};

// GraphStyle::Card()'s light colors for what the document leaves standard
static const char	*kCardFill		= "#ffffff";
static const char	*kCardBorder	= "#c4ccd0";
static const char	*kGroupFill		= "#eef3f5";
static const char	*kGroupBorder	= "#c8d8de";
static const char	*kEdgeColor		= "#8c8c8c";
// an imported shape without a place: a grid of these
static const float	kImportWidth	= 160;
static const float	kImportHeight	= 60;
static const float	kImportGap		= 40;
static const int32	kImportColumns	= 5;


static BString Number(float value)
{
	BString	text;
	text.SetToFormat("%.1f", value);
	if (text.EndsWith(".0"))
		text.Truncate(text.Length() - 2);
	return text;
}


// an attribute name as an XML attribute name: letters, digits, _ - .
static BString DataName(const BString &name)
{
	BString	data;
	for (int32 i = 0; i < name.Length(); i++) {
		char	c	= name[i];
		bool	ok	= ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'))
			|| (c == '_') || ((uint8)c >= 0x80)
			|| ((i > 0) && (((c >= '0') && (c <= '9')) || (c == '-') || (c == '.')));
		data << (ok ? c : '_');
	}
	// names draw.io gives a meaning of its own
	if ((data.Length() == 0) || (data == "label") || (data == "id")
		|| (data == "placeholders") || (data == "tooltip") || (data == "link"))
		data.Prepend("_");
	return data;
}


// an XML attribute value; a line break would come back as a space
static BString AttributeValue(const char *text)
{
	BString	value(XmlEscaped(text));
	value.ReplaceAll("\n", "&#10;");
	value.ReplaceAll("\t", "&#9;");
	return value;
}


// ---------------------------------------------------------------- export

class DrawioWriter {
public:
					DrawioWriter(const GraphDocument &document)
						: document(document) {}
	BString			Write(const char *title);
private:
	void			_WriteMembers(int32 parent, BPoint origin);
	void			_WriteNode(const GraphNode &node, BPoint origin);

	const GraphDocument	&document;
	BString			out;
};


BString DrawioWriter::Write(const char *title)
{
	out << "<mxfile host=\"ProjectConceptor\">\n"
		<< "  <diagram id=\"pc\" name=\"" << XmlEscaped(title) << "\">\n"
		<< "    <mxGraphModel grid=\"1\" gridSize=\"10\" guides=\"1\" connect=\"1\" "
			"arrows=\"1\" page=\"0\">\n"
		<< "      <root>\n"
		<< "        <mxCell id=\"0\"/>\n"
		<< "        <mxCell id=\"1\" parent=\"0\"/>\n";
	_WriteMembers(-1, BPoint(0, 0));
	for (size_t i = 0; i < document.connections.size(); i++) {
		const GraphConnection	&connection	= document.connections[i];
		if ((document.FindNode(connection.from) == NULL)
			|| (document.FindNode(connection.to) == NULL)) {
			fprintf(stderr, "DrawioTranslator: connection %ld to a missing node\n",
				(long)connection.id);
			continue;
		}
		out << "        <mxCell id=\"e" << connection.id << "\" style=\"html=1;"
			<< "endArrow=" << ((connection.arrows & 1) != 0 ? "classic" : "none")
			<< ";startArrow=" << ((connection.arrows & 2) != 0 ? "classic" : "none")
			<< ";strokeColor=" << (connection.hasColor ? HexColor(connection.color)
				: BString(kEdgeColor))
			<< ";strokeWidth=1.5;\" edge=\"1\" parent=\"1\" source=\"n" << connection.from
			<< "\" target=\"n" << connection.to << "\">\n"
			<< "          <mxGeometry relative=\"1\" as=\"geometry\"/>\n"
			<< "        </mxCell>\n";
	}
	out << "      </root>\n    </mxGraphModel>\n  </diagram>\n</mxfile>\n";
	return out;
}


void DrawioWriter::_WriteMembers(int32 parent, BPoint origin)
{
	for (size_t i = 0; i < document.nodes.size(); i++) {
		const GraphNode	&node	= document.nodes[i];
		// a parent that isn't in the document leaves the node on top
		int32			inside	= (document.FindNode(node.parent) != NULL)
			? node.parent : -1;
		if (inside == parent)
			_WriteNode(node, origin);
	}
}


void DrawioWriter::_WriteNode(const GraphNode &node, BPoint origin)
{
	// the label as HTML, the way the card shows it
	BString	label;
	label << "<b>" << XmlEscaped(node.name.String()) << "</b>";
	for (size_t a = 0; a < node.attributes.size(); a++) {
		BString	value(XmlEscaped(ShownValue(node.attributes[a]).String()));
		value.ReplaceAll("\n", "<br>");
		label << "<br>" << XmlEscaped(node.attributes[a].name.String()) << ": " << value;
	}
	BString	style("html=1;whiteSpace=wrap;");
	if (node.isGroup) {
		style << "rounded=1;arcSize=6;container=1;collapsible=0;verticalAlign=top;"
			<< "align=left;spacingLeft=8;fontStyle=1;fillColor="
			<< (node.hasFillColor ? HexColor(node.fillColor) : BString(kGroupFill))
			<< ";strokeColor=" << kGroupBorder << ";";
	} else {
		static const char	*shapes[][2]	= {
			{ "rectangle", "rounded=0;" }, { "ellipse", "ellipse;" },
			{ "diamond", "rhombus;" }, { "triangle", "triangle;direction=north;" },
			{ "hexagon", "shape=hexagon;perimeter=hexagonPerimeter2;" },
			{ "parallelogram", "shape=parallelogram;perimeter=parallelogramPerimeter;" },
			{ "note", "shape=note;size=12;" } };
		const char	*shape		= "rounded=1;arcSize=8;";
		const char	*shapeName	= node.shape.GetString(P_C_SHAPE_NAME, "rounded");
		for (size_t s = 0; s < sizeof(shapes) / sizeof(shapes[0]); s++) {
			if (strcmp(shapeName, shapes[s][0]) == 0)
				shape	= shapes[s][1];
		}
		style << shape << "verticalAlign=top;align=left;spacingLeft=8;spacingTop=4;"
			<< "fillColor=" << (node.hasFillColor ? HexColor(node.fillColor)
				: BString(kCardFill))
			<< ";strokeColor=" << (node.hasBorderColor ? HexColor(node.borderColor)
				: (node.hasBandColor ? HexColor(node.bandColor) : BString(kCardBorder)))
			<< ";strokeWidth=1.5;";
		if (node.hasBandColor)
			style << kBandStyle << "=" << HexColor(node.bandColor) << ";";
		if (node.hasTextColor)
			style << "fontColor=" << HexColor(node.textColor) << ";";
	}
	// a card grows with its rows, the stored height is its minimum; the
	// label wraps at draw.io's 12 px font
	BRect	frame	= node.frame;
	if (!node.isGroup) {
		int32	perLine	= std::max((int32)1, (int32)((frame.Width() - 16) / (12 * 0.55f)));
		int32	lines	= 1 + (node.name.CountChars() - 1) / perLine;
		for (size_t a = 0; a < node.attributes.size(); a++) {
			int32	chars	= node.attributes[a].name.CountChars() + 2
				+ ShownValue(node.attributes[a]).CountChars();
			lines	+= 1 + std::max((int32)0, chars - 1) / perLine;
		}
		float	needed	= 4 + lines * 14.5f + 10;
		if (frame.Height() < needed)
			frame.bottom	= frame.top + needed;
	}
	BString	parent(node.parent >= 0 && document.FindNode(node.parent) != NULL
		? BString("n") << node.parent : BString("1"));
	// draw.io places a container's members relative to it
	out << "        <object id=\"n" << node.id << "\" label=\"" << XmlEscaped(label.String())
		<< "\" placeholders=\"1\"";
	BString	names;
	for (size_t a = 0; a < node.attributes.size(); a++) {
		const char	*kind	= AttributeKindName(node.attributes[a].kind);
		names << (a > 0 ? "\n" : "") << (kind != NULL ? kind : "text") << "\t"
			<< node.attributes[a].name;
		out << " " << DataName(node.attributes[a].name) << "=\""
			<< AttributeValue(node.attributes[a].value.String()) << "\"";
	}
	if (names.Length() > 0)
		out << " " << kAttributesData << "=\"" << AttributeValue(names.String()) << "\"";
	out << ">\n"
		<< "          <mxCell style=\"" << XmlEscaped(style.String()) << "\" vertex=\"1\" parent=\""
		<< parent << "\">\n"
		<< "            <mxGeometry x=\"" << Number(frame.left - origin.x) << "\" y=\""
		<< Number(frame.top - origin.y) << "\" width=\"" << Number(frame.Width())
		<< "\" height=\"" << Number(frame.Height()) << "\" as=\"geometry\"/>\n"
		<< "          </mxCell>\n        </object>\n";
	if (node.isGroup)
		_WriteMembers(node.id, node.frame.LeftTop());
}


// ---------------------------------------------------------------- import

static BString Base64Decoded(const char *text)
{
	BString	decoded;
	uint32	bits	= 0;
	int32	count	= 0;
	for (const char *c = text; *c != '\0'; c++) {
		int32	value;
		if ((*c >= 'A') && (*c <= 'Z'))			value	= *c - 'A';
		else if ((*c >= 'a') && (*c <= 'z'))	value	= *c - 'a' + 26;
		else if ((*c >= '0') && (*c <= '9'))	value	= *c - '0' + 52;
		else if (*c == '+')						value	= 62;
		else if (*c == '/')						value	= 63;
		else continue;
		bits	= (bits << 6) | value;
		count	+= 6;
		if (count >= 8) {
			count	-= 8;
			decoded.Append((char)((bits >> count) & 0xff), 1);
		}
	}
	return decoded;
}


static BString PercentDecoded(const BString &text)
{
	BString	decoded;
	for (int32 i = 0; i < text.Length(); i++) {
		if ((text[i] == '%') && (i + 2 < text.Length())) {
			char	hex[3]	= { text[i + 1], text[i + 2], '\0' };
			decoded.Append((char)strtol(hex, NULL, 16), 1);
			i	+= 2;
		} else
			decoded.Append(text[i], 1);
	}
	return decoded;
}


// a diagram draw.io stored compressed: base64 of raw deflate of the
// URL-encoded XML
static status_t Inflated(const char *text, BString *xml)
{
	BString		packed	= Base64Decoded(text);
	z_stream	stream;
	memset(&stream, 0, sizeof(stream));
	if (inflateInit2(&stream, -15) != Z_OK)
		return B_ERROR;
	stream.next_in	= (Bytef *)packed.String();
	stream.avail_in	= packed.Length();
	BString		inflated;
	char		buffer[16384];
	int			result;
	do {
		stream.next_out		= (Bytef *)buffer;
		stream.avail_out	= sizeof(buffer);
		result	= inflate(&stream, Z_NO_FLUSH);
		if ((result != Z_OK) && (result != Z_STREAM_END)) {
			inflateEnd(&stream);
			return B_BAD_DATA;
		}
		inflated.Append(buffer, sizeof(buffer) - stream.avail_out);
	} while (result != Z_STREAM_END);
	inflateEnd(&stream);
	*xml	= PercentDecoded(inflated);
	return B_OK;
}


// the first line of an HTML label as plain text
static BString PlainFirstLine(const char *html)
{
	BString	text(html);
	text.ReplaceAll("<br>", "\n");
	text.ReplaceAll("<br/>", "\n");
	text.ReplaceAll("<div>", "\n");
	BString	plain;
	bool	inTag	= false;
	for (int32 i = 0; i < text.Length(); i++) {
		if (text[i] == '<')
			inTag	= true;
		else if (text[i] == '>')
			inTag	= false;
		else if (!inTag)
			plain.Append(text[i], 1);
	}
	plain.ReplaceAll("&nbsp;", " ");
	plain.ReplaceAll("&lt;", "<");
	plain.ReplaceAll("&gt;", ">");
	plain.ReplaceAll("&quot;", "\"");
	plain.ReplaceAll("&amp;", "&");
	plain.Trim();
	int32	newline	= plain.FindFirst('\n');
	if (newline >= 0)
		plain.Truncate(newline);
	return plain;
}


// a key's value in a draw.io style "a=1;b=2;ellipse;"
static BString StyleValue(const char *style, const char *key)
{
	BString	all(";");
	all << (style != NULL ? style : "") << ";";
	BString	needle(";");
	needle << key << "=";
	int32	at	= all.FindFirst(needle);
	if (at < 0)
		return BString();
	BString	value;
	all.CopyInto(value, at + needle.Length(), all.Length());
	value.Truncate(value.FindFirst(';'));
	return value;
}


static bool ParseColor(const BString &text, rgb_color *color)
{
	unsigned int	red, green, blue;
	if (sscanf(text.String(), "#%2x%2x%2x", &red, &green, &blue) != 3)
		return false;
	*color	= make_color(red, green, blue);
	return true;
}


struct DrawioCell {
	BString					id;
	BString					parent;
	BString					style;
	BString					label;
	BString					source;
	BString					target;
	bool					vertex;
	bool					edge;
	BRect					geometry;
	bool					hasGeometry;
	std::vector<GraphAttribute>	data;
};


class DrawioReader {
public:
	status_t		Read(const char *xml, GraphDocument *document);
private:
	void			_Collect(const TiXmlElement *root);
	BPoint			_Origin(const BString &parent, int32 depth);

	std::vector<DrawioCell>			cells;
	std::map<BString, size_t>		byId;
	std::map<BString, int32>		ids;
};


void DrawioReader::_Collect(const TiXmlElement *root)
{
	for (const TiXmlElement *element = root->FirstChildElement(); element != NULL;
			element = element->NextSiblingElement()) {
		const TiXmlElement	*cell	= element;
		DrawioCell			entry;
		entry.vertex		= false;
		entry.edge			= false;
		entry.hasGeometry	= false;
		// <object>/<UserObject> carry the label and data, the cell inside
		if ((strcmp(element->Value(), "object") == 0)
			|| (strcmp(element->Value(), "UserObject") == 0)) {
			cell	= element->FirstChildElement("mxCell");
			if (cell == NULL)
				continue;
			entry.id	= element->Attribute("id") ? element->Attribute("id") : "";
			entry.label	= element->Attribute("label") ? element->Attribute("label") : "";
			// our own export's real names and kinds, by data name
			std::map<BString, std::pair<BString, uint32> >	real;
			BString	lines(element->Attribute(kAttributesData)
				? element->Attribute(kAttributesData) : "");
			lines << "\n";
			for (int32 end; (end = lines.FindFirst('\n')) >= 0; lines.Remove(0, end + 1)) {
				BString	line;
				lines.CopyInto(line, 0, end);
				int32	tab	= line.FindFirst('\t');
				if (tab < 0)
					continue;
				BString	kindName, name;
				line.CopyInto(kindName, 0, tab);
				line.CopyInto(name, tab + 1, line.Length());
				uint32	kind	= B_STRING_TYPE;
				AttributeKindFromName(kindName.String(), &kind);
				real[DataName(name)]	= std::make_pair(name, kind);
			}
			for (const TiXmlAttribute *attribute = element->FirstAttribute();
					attribute != NULL; attribute = attribute->Next()) {
				BString	name(attribute->Name());
				if ((name == "id") || (name == "label") || (name == "placeholders")
					|| (name == "tooltip") || (name == "link") || (name == kAttributesData))
					continue;
				GraphAttribute	data;
				data.name	= name;
				data.value	= attribute->Value();
				data.kind	= B_STRING_TYPE;
				std::map<BString, std::pair<BString, uint32> >::iterator	found
					= real.find(name);
				if (found != real.end()) {
					data.name	= found->second.first;
					data.kind	= found->second.second;
				}
				entry.data.push_back(data);
			}
		} else if (strcmp(element->Value(), "mxCell") == 0) {
			entry.id	= element->Attribute("id") ? element->Attribute("id") : "";
			entry.label	= element->Attribute("value") ? element->Attribute("value") : "";
		} else
			continue;
		entry.parent	= cell->Attribute("parent") ? cell->Attribute("parent") : "";
		entry.style		= cell->Attribute("style") ? cell->Attribute("style") : "";
		entry.source	= cell->Attribute("source") ? cell->Attribute("source") : "";
		entry.target	= cell->Attribute("target") ? cell->Attribute("target") : "";
		entry.vertex	= (cell->Attribute("vertex") != NULL)
			&& (strcmp(cell->Attribute("vertex"), "1") == 0);
		entry.edge		= (cell->Attribute("edge") != NULL)
			&& (strcmp(cell->Attribute("edge"), "1") == 0);
		const TiXmlElement	*geometry	= cell->FirstChildElement("mxGeometry");
		double				x = 0, y = 0, width = 0, height = 0;
		if (geometry != NULL) {
			geometry->Attribute("x", &x);
			geometry->Attribute("y", &y);
			entry.hasGeometry	= (geometry->Attribute("width", &width) != NULL)
				&& (geometry->Attribute("height", &height) != NULL);
			entry.geometry.Set(x, y, x + width, y + height);
		}
		if (entry.id.Length() == 0)
			continue;
		byId[entry.id]	= cells.size();
		cells.push_back(entry);
	}
}


// where a member's relative geometry starts: its containers' corners
BPoint DrawioReader::_Origin(const BString &parent, int32 depth)
{
	std::map<BString, size_t>::iterator	found	= byId.find(parent);
	if ((found == byId.end()) || !cells[found->second].vertex || (depth > 64))
		return BPoint(0, 0);
	const DrawioCell	&cell	= cells[found->second];
	return _Origin(cell.parent, depth + 1) + cell.geometry.LeftTop();
}


status_t DrawioReader::Read(const char *text, GraphDocument *document)
{
	TiXmlDocument	doc;
	doc.Parse(text);
	if (doc.Error()) {
		fprintf(stderr, "DrawioTranslator: %s (line %d)\n", doc.ErrorDesc(), doc.ErrorRow());
		return B_BAD_DATA;
	}
	const TiXmlElement	*root	= doc.RootElement();
	if (root == NULL)
		return B_BAD_DATA;
	// <mxfile><diagram>[<mxGraphModel> or compressed text]; the first page
	const TiXmlElement	*model	= NULL;
	TiXmlDocument		inflatedDoc;
	if (strcmp(root->Value(), "mxGraphModel") == 0)
		model	= root;
	else if (strcmp(root->Value(), "mxfile") == 0) {
		const TiXmlElement	*diagram	= root->FirstChildElement("diagram");
		if (diagram == NULL)
			return B_BAD_DATA;
		model	= diagram->FirstChildElement("mxGraphModel");
		if ((model == NULL) && (diagram->GetText() != NULL)) {
			BString	xml;
			status_t	err	= Inflated(diagram->GetText(), &xml);
			if (err != B_OK) {
				fprintf(stderr, "DrawioTranslator: the compressed diagram doesn't inflate\n");
				return err;
			}
			inflatedDoc.Parse(xml.String());
			if (inflatedDoc.Error())
				return B_BAD_DATA;
			model	= inflatedDoc.RootElement();
		}
	}
	if ((model == NULL) || (model->FirstChildElement("root") == NULL))
		return B_BAD_DATA;
	_Collect(model->FirstChildElement("root"));

	// vertices first, so edges find their ends
	std::vector<int32>	unplaced;
	float				bottom	= 0;
	for (size_t i = 0; i < cells.size(); i++) {
		const DrawioCell	&cell	= cells[i];
		if (!cell.vertex)
			continue;
		GraphNode	node;
		node.id		= (int32)ids.size() + 1;
		ids[cell.id]	= node.id;
		node.name	= PlainFirstLine(cell.label.String());
		if (node.name.Length() == 0)
			node.name	= cell.id;
		node.attributes	= cell.data;
		BPoint	origin	= _Origin(cell.parent, 0);
		node.frame	= cell.geometry.OffsetByCopy(origin);
		if (!cell.hasGeometry)
			unplaced.push_back(node.id);
		else
			bottom	= std::max(bottom, node.frame.bottom);
		rgb_color	color;
		if (ParseColor(StyleValue(cell.style.String(), "fillColor"), &color)
			&& !((color.red == 255) && (color.green == 255) && (color.blue == 255))) {
			node.fillColor		= color;
			node.hasFillColor	= true;
		}
		if (ParseColor(StyleValue(cell.style.String(), kBandStyle), &color)) {
			node.bandColor		= color;
			node.hasBandColor	= true;
		} else if (ParseColor(StyleValue(cell.style.String(), "strokeColor"), &color)) {
			node.borderColor	= color;
			node.hasBorderColor	= true;
		}
		if (ParseColor(StyleValue(cell.style.String(), "fontColor"), &color)) {
			node.textColor		= color;
			node.hasTextColor	= true;
		}
		document->nodes.push_back(node);
	}
	// what has members is a group; the members know their group
	for (size_t i = 0; i < document->nodes.size(); i++) {
		GraphNode	&node	= document->nodes[i];
		for (size_t c = 0; c < cells.size(); c++) {
			if (!cells[c].vertex || (ids[cells[c].id] != node.id))
				continue;
			std::map<BString, int32>::iterator	parent	= ids.find(cells[c].parent);
			if (parent != ids.end())
				node.parent	= parent->second;
		}
	}
	for (size_t i = 0; i < document->nodes.size(); i++) {
		for (size_t m = 0; m < document->nodes.size(); m++) {
			if (document->nodes[m].parent == document->nodes[i].id)
				document->nodes[i].isGroup	= true;
		}
	}
	for (size_t u = 0; u < unplaced.size(); u++) {
		float	left	= (u % kImportColumns) * (kImportWidth + kImportGap);
		float	top		= bottom + kImportGap + (u / kImportColumns) * (kImportHeight + kImportGap);
		for (size_t i = 0; i < document->nodes.size(); i++) {
			if (document->nodes[i].id == unplaced[u])
				document->nodes[i].frame.Set(left, top, left + kImportWidth, top + kImportHeight);
		}
	}

	for (size_t i = 0; i < cells.size(); i++) {
		const DrawioCell	&cell	= cells[i];
		if (!cell.edge)
			continue;
		std::map<BString, int32>::iterator	from	= ids.find(cell.source);
		std::map<BString, int32>::iterator	to		= ids.find(cell.target);
		if ((from == ids.end()) || (to == ids.end())) {
			// a free line or arrow drawn on the page: nothing to connect
			fprintf(stderr, "DrawioTranslator: edge %s without both ends left out\n",
				cell.id.String());
			continue;
		}
		GraphConnection	connection;
		connection.id	= (int32)(ids.size() + document->connections.size() + 1);
		connection.from	= from->second;
		connection.to	= to->second;
		BString	end		= StyleValue(cell.style.String(), "endArrow");
		BString	start	= StyleValue(cell.style.String(), "startArrow");
		// draw.io's defaults: an arrow at the end, none at the start
		connection.arrows	= ((end != "none") ? 1 : 0)
			| (((start.Length() > 0) && (start != "none")) ? 2 : 0);
		rgb_color	color;
		if (ParseColor(StyleValue(cell.style.String(), "strokeColor"), &color)
			&& (HexColor(color) != kEdgeColor)) {
			connection.color	= color;
			connection.hasColor	= true;
		}
		document->connections.push_back(connection);
	}
	return B_OK;
}


// ---------------------------------------------------------------- hooks

static bool LooksLikeDrawio(BPositionIO *in)
{
	char	start[1024];
	off_t	position	= in->Position();
	ssize_t	bytesRead	= in->Read(start, sizeof(start) - 1);
	in->Seek(position, SEEK_SET);
	if (bytesRead <= 0)
		return false;
	start[bytesRead]	= '\0';
	return (strstr(start, "<mxfile") != NULL) || (strstr(start, "<mxGraphModel") != NULL);
}


status_t Identify(BPositionIO *inSource, const translation_format *inFormat,
	BMessage *ioExtension, translator_info *outInfo, uint32 outType)
{
	if (((outType == 0) || (outType == P_C_DOCUMENT_RAW_TYPE))
		&& LooksLikeDrawio(inSource)) {
		outInfo->type		= P_C_DRAWIO_TYPE;
		outInfo->group		= B_TRANSLATOR_TEXT;
		outInfo->quality	= 0.4;
		outInfo->capability	= 0.8;
		strcpy(outInfo->name, "draw.io diagram");
		strcpy(outInfo->MIME, kDrawioMime);
		return B_OK;
	}
	if ((outType == P_C_DRAWIO_TYPE) && LooksLikeDocument(inSource)) {
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
	if (outType == P_C_DRAWIO_TYPE) {
		GraphDocument	document;
		status_t		err	= document.Read(inSource);
		if (err != B_OK)
			return err;
		const char	*title	= (ioExtension != NULL)
			? ioExtension->GetString(P_C_FORMAT_TITLE, "ProjectConceptor")
			: "ProjectConceptor";
		BString	text	= DrawioWriter(document).Write(title);
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
	DrawioReader	reader;
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
	ioExtension->AddInt32(P_C_FORMAT_TYPE, P_C_DRAWIO_TYPE);
	ioExtension->AddString(P_C_FORMAT_EXTENSION, "drawio");
	return B_OK;
}
