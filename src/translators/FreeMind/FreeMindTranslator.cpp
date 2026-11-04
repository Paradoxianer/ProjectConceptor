#include <TranslatorAddOn.h>
#include <TranslationKit.h>

#include <algorithm>
#include <ctype.h>
#include <map>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "AttributeText.h"
#include "ConfigView.h"
#include "FreeMindTranslator.h"
#include "GraphDocument.h"
#include "tinyxml.h"

/** Reads and writes FreeMind mind maps (.mm, also read by Freeplane).
 * A mind map is a tree: the export starts at the nodes nothing points to
 * and follows the connections, a connection to a node already written
 * becomes an arrow link; group members are children of their group.
 * The import lays the tree out the way a mind map looks, the root in the
 * middle and its branches to the left and right. */

static const char	*kFreeMindMime	= "application/x-freemind";
// ids of what only the export makes, so its import can take it back
static const char	*kRootId		= "PC_ROOT";
static const char	*kNodePrefix	= "PC_NODE_";
static const char	*kGroupPrefix	= "PC_GROUP_";
static const char	*kLinkPrefix	= "PC_LINK_";
static const char	*kNoteName		= "Note";

// imported cards: a name row and a row per attribute
static const float	kNodeWidth		= 160;
static const float	kNameHeight		= 40;
static const float	kRowHeight		= 20;
static const float	kLevelGap		= 60;
static const float	kSiblingGap		= 20;
static const float	kGroupPadding	= 16;
static const float	kMargin			= 20;


static bool ParseColor(const char *text, rgb_color *color)
{
	unsigned int	red, green, blue;
	if ((text == NULL) || (sscanf(text, "#%2x%2x%2x", &red, &green, &blue) != 3))
		return false;
	*color	= make_color(red, green, blue);
	return true;
}


// ---------------------------------------------------------------- export

class FreeMindWriter {
public:
					FreeMindWriter(const GraphDocument &document)
						: document(document) {}
	BString			Write(void);
private:
	struct Branch {
		int32					id;
		/** the connection leading here, -1 for a root or a group member */
		int32					connection;
		std::vector<Branch>		children;
		/** connections to nodes written elsewhere */
		std::vector<int32>		links;
	};
	Branch			_Grow(int32 id, int32 connection);
	/** position: the side of a child of the map's root, else NULL */
	void			_WriteBranch(const Branch &branch, const BString &indent,
						const char *position, bool isRoot);
	const char		*_Side(int32 id, float middle);
	BString			_Id(int32 id);

	const GraphDocument	&document;
	std::set<int32>		visited;
	BString				out;
};


BString FreeMindWriter::_Id(int32 id)
{
	const GraphNode	*node	= document.FindNode(id);
	BString			text((node != NULL) && node->isGroup ? kGroupPrefix : kNodePrefix);
	text << id;
	return text;
}


FreeMindWriter::Branch FreeMindWriter::_Grow(int32 id, int32 connection)
{
	Branch	branch;
	branch.id			= id;
	branch.connection	= connection;
	visited.insert(id);
	const GraphNode	*node	= document.FindNode(id);
	if ((node != NULL) && node->isGroup) {
		// all members first, so a connection between two of them becomes
		// an arrow link instead of taking one out of the group
		std::vector<int32>	members;
		for (size_t i = 0; i < document.nodes.size(); i++) {
			if ((document.nodes[i].parent == id)
				&& (visited.find(document.nodes[i].id) == visited.end())) {
				members.push_back(document.nodes[i].id);
				visited.insert(document.nodes[i].id);
			}
		}
		for (size_t m = 0; m < members.size(); m++)
			branch.children.push_back(_Grow(members[m], -1));
	}
	for (size_t i = 0; i < document.connections.size(); i++) {
		const GraphConnection	&next	= document.connections[i];
		if ((next.from != id) || (document.FindNode(next.to) == NULL))
			continue;
		if (visited.find(next.to) == visited.end())
			branch.children.push_back(_Grow(next.to, (int32)i));
		else
			branch.links.push_back((int32)i);
	}
	return branch;
}


const char *FreeMindWriter::_Side(int32 id, float middle)
{
	const GraphNode	*node	= document.FindNode(id);
	if (node == NULL)
		return "right";
	return ((node->frame.left + node->frame.right) / 2 < middle) ? "left" : "right";
}


BString FreeMindWriter::Write(void)
{
	// a member lies in its group, a target at the end of its connection:
	// what is neither starts a tree
	std::set<int32>	reached;
	for (size_t i = 0; i < document.connections.size(); i++) {
		const GraphConnection	&connection	= document.connections[i];
		if ((connection.from != connection.to)
			&& (document.FindNode(connection.from) != NULL))
			reached.insert(connection.to);
	}
	std::vector<Branch>	roots;
	for (size_t i = 0; i < document.nodes.size(); i++) {
		const GraphNode	&node	= document.nodes[i];
		if ((reached.find(node.id) == reached.end())
			&& (document.FindNode(node.parent) == NULL)
			&& (visited.find(node.id) == visited.end()))
			roots.push_back(_Grow(node.id, -1));
	}
	// circles nothing leads into
	for (size_t i = 0; i < document.nodes.size(); i++) {
		if (visited.find(document.nodes[i].id) == visited.end())
			roots.push_back(_Grow(document.nodes[i].id, -1));
	}

	out << "<map version=\"1.0.1\">\n"
		<< "<!-- written by ProjectConceptor; to open it get FreeMind from "
			"http://freemind.sourceforge.net or Freeplane -->\n";
	BRect	bounds	= document.Bounds();
	float	middle	= bounds.IsValid() ? (bounds.left + bounds.right) / 2 : 0;
	if (roots.size() == 1) {
		const GraphNode	*root	= document.FindNode(roots[0].id);
		if (root != NULL)
			middle	= (root->frame.left + root->frame.right) / 2;
		_WriteBranch(roots[0], "", NULL, true);
	} else if (roots.size() > 1) {
		// a mind map has one root
		out << "<node ID=\"" << kRootId << "\" TEXT=\"\">\n";
		for (size_t i = 0; i < roots.size(); i++)
			_WriteBranch(roots[i], "  ", _Side(roots[i].id, middle), false);
		out << "</node>\n";
	}
	out << "</map>\n";
	return out;
}


void FreeMindWriter::_WriteBranch(const Branch &branch, const BString &indent,
	const char *position, bool isRoot)
{
	const GraphNode	*node	= document.FindNode(branch.id);
	if (node == NULL)
		return;
	out << indent << "<node ID=\"" << _Id(node->id) << "\" TEXT=\""
		<< XmlEscaped(node->name.String()) << "\"";
	if (position != NULL)
		out << " POSITION=\"" << position << "\"";
	// the color the node is marked with: its band, else its card
	if (node->hasBandColor)
		out << " BACKGROUND_COLOR=\"" << HexColor(node->bandColor) << "\"";
	else if (node->hasFillColor)
		out << " BACKGROUND_COLOR=\"" << HexColor(node->fillColor) << "\"";
	if (node->hasTextColor)
		out << " COLOR=\"" << HexColor(node->textColor) << "\"";
	out << ">\n";
	if (branch.connection >= 0) {
		const GraphConnection	&connection	= document.connections[branch.connection];
		if (connection.hasColor)
			out << indent << "<edge COLOR=\"" << HexColor(connection.color) << "\"/>\n";
	}
	bool	noteWritten	= false;
	for (size_t a = 0; a < node->attributes.size(); a++) {
		const GraphAttribute	&attribute	= node->attributes[a];
		if ((attribute.kind == P_C_ATTRIBUTE_NOTE) && !noteWritten) {
			BString	paragraphs(XmlEscaped(attribute.value.String()));
			paragraphs.ReplaceAll("\n", "</p><p>");
			out << indent << "<richcontent TYPE=\"NOTE\"><html><head></head><body><p>"
				<< paragraphs << "</p></body></html></richcontent>\n";
			noteWritten	= true;
			continue;
		}
		out << indent << "<attribute NAME=\"" << XmlEscaped(attribute.name.String())
			<< "\" VALUE=\"" << XmlEscaped(attribute.value.String()) << "\"/>\n";
	}
	for (size_t l = 0; l < branch.links.size(); l++) {
		const GraphConnection	&connection	= document.connections[branch.links[l]];
		out << indent << "<arrowlink ID=\"" << kLinkPrefix << connection.id
			<< "\" DESTINATION=\"" << _Id(connection.to) << "\"";
		if (connection.hasColor)
			out << " COLOR=\"" << HexColor(connection.color) << "\"";
		out << " STARTARROW=\"" << ((connection.arrows & 2) != 0 ? "Default" : "None")
			<< "\" ENDARROW=\"" << ((connection.arrows & 1) != 0 ? "Default" : "None")
			<< "\"/>\n";
	}
	BString	inner(indent);
	inner << "  ";
	float	middle	= (node->frame.left + node->frame.right) / 2;
	for (size_t c = 0; c < branch.children.size(); c++) {
		// FreeMind places only the root's children on a side
		_WriteBranch(branch.children[c], inner,
			isRoot ? _Side(branch.children[c].id, middle) : NULL, false);
	}
	out << indent << "</node>\n";
}


// ---------------------------------------------------------------- import

// "<p>a</p><p>b</p>" -> "a\nb"
static BString PlainText(const TiXmlElement *element)
{
	BString	text;
	for (const TiXmlNode *child = element->FirstChild(); child != NULL;
			child = child->NextSibling()) {
		if (child->ToText() != NULL)
			text << child->Value();
		else if (child->ToElement() != NULL) {
			BString	inner(PlainText(child->ToElement()));
			const char	*name	= child->Value();
			bool		block	= (strcmp(name, "p") == 0) || (strcmp(name, "div") == 0)
				|| (strcmp(name, "li") == 0) || (strcmp(name, "br") == 0);
			if (block && (text.Length() > 0) && !text.EndsWith("\n"))
				text << "\n";
			text << inner;
		}
	}
	return text;
}


static BString Trimmed(BString text)
{
	return text.Trim();
}


// the kind a value looks like; the file doesn't say
static uint32 GuessedKind(const BString &value)
{
	if ((value == "true") || (value == "false"))
		return B_BOOL_TYPE;
	rgb_color	color;
	if ((value.Length() == 7) && ParseColor(value.String(), &color))
		return P_C_ATTRIBUTE_COLOR;
	float	number;
	BString	unit;
	// "2026-11-03" or "10:30" aren't a number with a unit
	if ((ParseNumberWithUnit(value.String(), &number, &unit) == B_OK)
		&& (strpbrk(unit.String(), "0123456789-/:") == NULL))
		return P_C_ATTRIBUTE_NUMBER;
	return B_STRING_TYPE;
}


class FreeMindReader {
public:
	status_t		Read(const char *xml, GraphDocument *document);
private:
	struct Branch {
		/** index into document->nodes, -1 for the export's own root */
		int32				index;
		bool				left;
		std::vector<int32>	children;
		float				height;
	};
	status_t		_ReadNode(const TiXmlElement *element, int32 parentBranch,
						bool left);
	int32			_IdFor(const char *xmlId);
	float			_Measure(int32 branch);
	void			_Place(int32 branch, float x, float top);
	void			_FitGroups(void);

	GraphDocument				*document;
	std::map<BString, int32>	ids;
	std::vector<Branch>			branches;
	/** arrow links, once all nodes have their ids: element, from */
	std::vector<std::pair<const TiXmlElement*, int32> >	links;
	int32						unnamed;
};


int32 FreeMindReader::_IdFor(const char *xmlId)
{
	std::map<BString, int32>::iterator	found	= ids.find(BString(xmlId));
	if (found != ids.end())
		return found->second;
	int32	id	= (int32)ids.size() + 1;
	ids[BString(xmlId)]	= id;
	return id;
}


status_t FreeMindReader::Read(const char *xml, GraphDocument *_document)
{
	document	= _document;
	unnamed		= 0;
	TiXmlDocument	doc;
	// notes keep their line breaks; the setting is shared by everything
	// using the library, so it is put back
	bool			condensed	= TiXmlBase::IsWhiteSpaceCondensed();
	TiXmlBase::SetCondenseWhiteSpace(false);
	// FreeMind writes no declaration and everything past ASCII as &#...;,
	// which TinyXML decodes right only knowing it is UTF-8
	doc.Parse(xml, NULL, TIXML_ENCODING_UTF8);
	TiXmlBase::SetCondenseWhiteSpace(condensed);
	if (doc.Error()) {
		fprintf(stderr, "FreeMindTranslator: %s (line %d)\n", doc.ErrorDesc(),
			doc.ErrorRow());
		return B_BAD_DATA;
	}
	const TiXmlElement	*map	= doc.RootElement();
	if ((map == NULL) || (strcmp(map->Value(), "map") != 0))
		return B_NO_TRANSLATOR;
	const TiXmlElement	*root	= map->FirstChildElement("node");
	if (root == NULL) {
		fprintf(stderr, "FreeMindTranslator: the map has no node\n");
		return B_BAD_DATA;
	}
	status_t	err	= _ReadNode(root, -1, false);
	// connections get ids past all node ids
	for (size_t i = 0; i < document->connections.size(); i++)
		document->connections[i].id	= (int32)(ids.size() + i + 1);
	for (size_t i = 0; (i < links.size()) && (err == B_OK); i++) {
		const TiXmlElement	*link			= links[i].first;
		const char			*destination	= link->Attribute("DESTINATION");
		if ((destination == NULL) || (ids.find(BString(destination)) == ids.end())) {
			fprintf(stderr, "FreeMindTranslator: arrow link to an unknown node %s\n",
				destination != NULL ? destination : "");
			err	= B_BAD_DATA;
			break;
		}
		GraphConnection	connection;
		connection.id	= (int32)(ids.size() + document->connections.size() + 1);
		connection.from	= links[i].second;
		connection.to	= ids[BString(destination)];
		const char	*start	= link->Attribute("STARTARROW");
		const char	*end	= link->Attribute("ENDARROW");
		connection.arrows	= ((end == NULL) || (strcmp(end, "None") != 0) ? 1 : 0)
			| ((start != NULL) && (strcmp(start, "None") != 0) ? 2 : 0);
		connection.hasColor	= ParseColor(link->Attribute("COLOR"), &connection.color);
		document->connections.push_back(connection);
	}
	if (err != B_OK)
		return err;
	_Measure(0);
	_Place(0, 0, 0);
	_FitGroups();
	// the left branches lie left of the root: all of it moved to where the
	// editor's canvas starts
	BRect	bounds	= document->Bounds();
	if (bounds.IsValid()) {
		for (size_t i = 0; i < document->nodes.size(); i++)
			document->nodes[i].frame.OffsetBy(kMargin - bounds.left, kMargin - bounds.top);
	}
	return B_OK;
}


status_t FreeMindReader::_ReadNode(const TiXmlElement *element, int32 parentBranch,
	bool left)
{
	const char	*xmlId	= element->Attribute("ID");
	BString		ownId;
	if (xmlId == NULL) {
		// FreeMind always writes one; a hand-made map may not
		ownId << "PC_UNNAMED_" << ++unnamed;
		xmlId	= ownId.String();
	}
	Branch	branch;
	branch.index	= -1;
	branch.left		= left;
	branch.height	= 0;
	int32	branchIndex	= (int32)branches.size();
	branches.push_back(branch);
	if (parentBranch >= 0)
		branches[parentBranch].children.push_back(branchIndex);

	int32	parentNode	= (parentBranch >= 0) && (branches[parentBranch].index >= 0)
		? document->nodes[branches[parentBranch].index].id : -1;
	bool	exportRoot	= (parentBranch < 0) && (strcmp(xmlId, kRootId) == 0);
	if (!exportRoot) {
		GraphNode	node;
		node.id		= _IdFor(xmlId);
		node.isGroup	= (strncmp(xmlId, kGroupPrefix, strlen(kGroupPrefix)) == 0);
		bool	member	= (parentNode >= 0) && document->nodes[branches[parentBranch].index].isGroup;
		node.parent	= member ? parentNode : -1;
		if (element->Attribute("TEXT") != NULL)
			node.name	= element->Attribute("TEXT");
		node.hasBandColor	= ParseColor(element->Attribute("BACKGROUND_COLOR"), &node.bandColor);
		node.hasTextColor	= ParseColor(element->Attribute("COLOR"), &node.textColor);
		rgb_color	edgeColor;
		bool		hasEdgeColor	= false;
		for (const TiXmlElement *child = element->FirstChildElement(); child != NULL;
				child = child->NextSiblingElement()) {
			const char	*name	= child->Value();
			if (strcmp(name, "attribute") == 0) {
				GraphAttribute	attribute;
				attribute.name	= child->Attribute("NAME") ? child->Attribute("NAME") : "";
				attribute.value	= child->Attribute("VALUE") ? child->Attribute("VALUE") : "";
				attribute.kind	= GuessedKind(attribute.value);
				node.attributes.push_back(attribute);
			} else if (strcmp(name, "richcontent") == 0) {
				const char	*type	= child->Attribute("TYPE");
				BString		text	= Trimmed(PlainText(child));
				if ((type != NULL) && (strcmp(type, "NOTE") == 0)) {
					GraphAttribute	note;
					note.name	= kNoteName;
					note.value	= text;
					note.kind	= P_C_ATTRIBUTE_NOTE;
					node.attributes.push_back(note);
				} else if ((type == NULL) || (strcmp(type, "NODE") == 0))
					node.name	= text;
			} else if (strcmp(name, "edge") == 0)
				hasEdgeColor	= ParseColor(child->Attribute("COLOR"), &edgeColor);
			else if (strcmp(name, "arrowlink") == 0)
				links.push_back(std::make_pair(child, node.id));
		}
		float	rows	= (float)node.attributes.size();
		node.frame.Set(0, 0, kNodeWidth, kNameHeight + rows * kRowHeight);
		branches[branchIndex].index	= (int32)document->nodes.size();
		document->nodes.push_back(node);
		if ((parentNode >= 0) && !member) {
			// numbered once all node ids are known, see Read()
			GraphConnection	connection;
			connection.from		= parentNode;
			connection.to		= node.id;
			connection.arrows	= 1;
			connection.hasColor	= hasEdgeColor;
			if (hasEdgeColor)
				connection.color	= edgeColor;
			document->connections.push_back(connection);
		}
	}
	for (const TiXmlElement *child = element->FirstChildElement("node"); child != NULL;
			child = child->NextSiblingElement("node")) {
		// a side is chosen at the root only, the rest follow their branch
		bool	childLeft	= left;
		if (parentBranch < 0) {
			const char	*position	= child->Attribute("POSITION");
			childLeft	= (position != NULL) && (strcmp(position, "left") == 0);
		}
		status_t	err	= _ReadNode(child, branchIndex, childLeft);
		if (err != B_OK)
			return err;
	}
	return B_OK;
}


float FreeMindReader::_Measure(int32 index)
{
	Branch	&branch	= branches[index];
	float	own		= (branch.index >= 0)
		? document->nodes[branch.index].frame.Height() : 0;
	float	children[2]	= { 0, 0 };
	for (size_t c = 0; c < branch.children.size(); c++) {
		int32	child	= branch.children[c];
		float	height	= _Measure(child);
		int32	side	= branches[child].left ? 1 : 0;
		if (children[side] > 0)
			children[side] += kSiblingGap;
		children[side] += height;
	}
	branch.height	= std::max(own, std::max(children[0], children[1]));
	return branch.height;
}


void FreeMindReader::_Place(int32 index, float x, float top)
{
	Branch	&branch	= branches[index];
	BRect	frame(x, top, x, top);
	if (branch.index >= 0) {
		GraphNode	&node	= document->nodes[branch.index];
		float		width	= node.frame.Width();
		float		height	= node.frame.Height();
		float		left	= branch.left ? x - width : x;
		float		y		= top + (branch.height - height) / 2;
		node.frame.Set(left, y, left + width, y + height);
		frame	= node.frame;
	}
	// each side of the children centered on the node
	for (int32 side = 0; side < 2; side++) {
		float	span	= 0;
		for (size_t c = 0; c < branch.children.size(); c++) {
			const Branch	&child	= branches[branch.children[c]];
			if ((child.left ? 1 : 0) != side)
				continue;
			if (span > 0)
				span += kSiblingGap;
			span += child.height;
		}
		float	y	= top + (branch.height - span) / 2;
		for (size_t c = 0; c < branch.children.size(); c++) {
			int32	child	= branch.children[c];
			if ((branches[child].left ? 1 : 0) != side)
				continue;
			_Place(child, side == 1 ? frame.left - kLevelGap : frame.right + kLevelGap, y);
			y += branches[child].height + kSiblingGap;
		}
	}
}


void FreeMindReader::_FitGroups(void)
{
	// each group around its members, the innermost first
	for (int32 pass = 0; pass < 64; pass++) {
		bool	changed	= false;
		for (size_t g = 0; g < document->nodes.size(); g++) {
			GraphNode	&group	= document->nodes[g];
			if (!group.isGroup)
				continue;
			BRect	frame;
			for (size_t m = 0; m < document->nodes.size(); m++) {
				if (document->nodes[m].parent == group.id)
					frame	= frame.IsValid() ? (frame | document->nodes[m].frame)
						: document->nodes[m].frame;
			}
			if (!frame.IsValid())
				continue;
			frame.InsetBy(-kGroupPadding, -kGroupPadding);
			frame.top	-= kGroupPadding;
			if (frame != group.frame) {
				group.frame	= frame;
				changed		= true;
			}
		}
		if (!changed)
			break;
	}
}


// ---------------------------------------------------------------- hooks

static bool LooksLikeFreeMind(BPositionIO *in)
{
	// <map> is the first element, after an optional <?xml ...?> line and
	// comments - not any page with an image map in it
	char	start[1024];
	off_t	position	= in->Position();
	ssize_t	bytesRead	= in->Read(start, sizeof(start) - 1);
	in->Seek(position, SEEK_SET);
	if (bytesRead <= 0)
		return false;
	start[bytesRead]	= '\0';
	const char	*next	= start;
	// a UTF-8 byte order mark
	if (strncmp(next, "\xef\xbb\xbf", 3) == 0)
		next += 3;
	while (true) {
		next += strspn(next, " \t\r\n");
		const char	*close	= NULL;
		if (strncmp(next, "<?", 2) == 0)
			close	= strstr(next, "?>");
		else if (strncmp(next, "<!--", 4) == 0)
			close	= strstr(next, "-->");
		else
			break;
		if (close == NULL)
			return false;
		next	= strchr(close, '>') + 1;
	}
	return (strncmp(next, "<map", 4) == 0)
		&& ((next[4] == '>') || isspace((unsigned char)next[4]));
}


status_t Identify(BPositionIO *inSource, const translation_format *inFormat,
	BMessage *ioExtension, translator_info *outInfo, uint32 outType)
{
	if (((outType == 0) || (outType == P_C_DOCUMENT_RAW_TYPE))
		&& LooksLikeFreeMind(inSource)) {
		outInfo->type		= P_C_FREEMIND_TYPE;
		outInfo->group		= B_TRANSLATOR_TEXT;
		outInfo->quality	= 0.4;
		outInfo->capability	= 0.8;
		strcpy(outInfo->name, "FreeMind");
		strcpy(outInfo->MIME, kFreeMindMime);
		return B_OK;
	}
	if ((outType == P_C_FREEMIND_TYPE) && LooksLikeDocument(inSource)) {
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
	if (outType == P_C_FREEMIND_TYPE) {
		GraphDocument	document;
		status_t		err	= document.Read(inSource);
		if (err != B_OK)
			return err;
		BString	text	= FreeMindWriter(document).Write();
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
	FreeMindReader	reader;
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
	ioExtension->AddInt32(P_C_FORMAT_TYPE, P_C_FREEMIND_TYPE);
	ioExtension->AddString(P_C_FORMAT_EXTENSION, "mm");
	return B_OK;
}
