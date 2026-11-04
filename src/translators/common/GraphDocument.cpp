#include "GraphDocument.h"

#include <stdio.h>
#include <string.h>

#include "AttributeText.h"
#include "ProjectConceptorDefs.h"

// the editor's standard pattern (GraphEditor::Init): these mean "not set"
static const rgb_color	kStandardFill		= {152, 180, 190, 255};
static const rgb_color	kStandardBorder		= {0, 0, 0, 255};
static const rgb_color	kStandardText		= {0, 0, 0, 255};
// GraphEditor's kDefaultConnectionColor
static const rgb_color	kStandardConnection	= {187, 67, 47, 255};


static bool SameColor(rgb_color a, rgb_color b)
{
	return (a.red == b.red) && (a.green == b.green) && (a.blue == b.blue);
}


// an int32 the editor wrote an rgb_color into
static rgb_color Unpacked(int32 packed)
{
	uint8	bytes[4];
	memcpy(bytes, &packed, sizeof(bytes));
	return make_color(bytes[0], bytes[1], bytes[2], bytes[3]);
}


// a color of the pattern, false if missing or the standard one
static bool FindColor(const BMessage &pattern, const char *field,
	rgb_color standard, rgb_color *color)
{
	int32	packed;
	if (pattern.FindInt32(field, &packed) != B_OK)
		return false;
	*color	= Unpacked(packed);
	return !SameColor(*color, standard);
}


bool LooksLikeDocument(BPositionIO *in)
{
	char	magic[4];
	off_t	position	= in->Position();
	ssize_t	bytesRead	= in->Read(magic, sizeof(magic));
	in->Seek(position, SEEK_SET);
	return (bytesRead == sizeof(magic)) && (memcmp(magic, "HMF1", 4) == 0);
}


status_t GraphDocument::Read(BPositionIO *in)
{
	BMessage	document;
	BMessage	allNodes;
	BMessage	allConnections;
	if ((document.Unflatten(in) != B_OK)
		|| (document.FindMessage(P_C_DOC_ALLNODES, &allNodes) != B_OK))
		return B_BAD_DATA;
	BMessage	entry;
	for (int32 i = 0; allNodes.FindMessage(P_C_DOC_ENTRY, i, &entry) == B_OK; i++) {
		status_t	err	= _ReadNode(entry);
		if (err != B_OK)
			return err;
	}
	// a document without connections has no list for them
	document.FindMessage(P_C_DOC_ALLCONNECTIONS, &allConnections);
	for (int32 i = 0; allConnections.FindMessage(P_C_DOC_ENTRY, i, &entry) == B_OK; i++) {
		status_t	err	= _ReadConnection(entry);
		if (err != B_OK)
			return err;
	}
	return B_OK;
}


status_t GraphDocument::_ReadNode(const BMessage &message)
{
	GraphNode	node;
	if (message.FindInt32(P_C_NODE_ID, &node.id) != B_OK) {
		fprintf(stderr, "GraphDocument: node without id\n");
		return B_BAD_DATA;
	}
	node.isGroup	= (message.what == P_C_GROUP_TYPE);
	node.parent		= message.GetInt32(P_C_NODE_PARENT, -1);
	node.frame		= message.GetRect(P_C_NODE_FRAME, BRect());
	node.attributeColumn	= message.GetFloat(P_C_NODE_ATTRIBUTE_COLUMN, 0);

	BMessage	data;
	message.FindMessage(P_C_NODE_DATA, &data);
	node.name		= data.GetString(P_C_NODE_NAME, "");
	char		*fieldName;
	type_code	type;
	int32		count;
	// every entry of a name is a row of its own, as the editor shows them
	for (int32 i = 0; data.GetInfo(B_MESSAGE_TYPE, i, &fieldName, &type, &count) == B_OK; i++) {
		for (int32 entry = 0; entry < count; entry++) {
			BMessage		block;
			GraphAttribute	attribute;
			if (data.FindMessage(fieldName, entry, &block) != B_OK)
				continue;
			attribute.name	= block.GetString(P_C_ATTRIBUTE_NAME, fieldName);
			attribute.kind	= block.what;
			AttributeValueText(block, &attribute.value);
			node.attributes.push_back(attribute);
		}
	}

	BMessage	pattern;
	message.FindMessage(P_C_NODE_PATTERN, &pattern);
	node.hasBandColor	= false;
	if (!node.isGroup) {
		// before HighlightColor the fill was the band and the card the
		// standard one (ClassRenderer::ValueChanged)
		int32	packed	= 0;
		if (pattern.FindInt32(P_C_PATTERN_HIGHLIGHT_COLOR, &packed) == B_OK) {
			node.bandColor		= Unpacked(packed);
			node.hasBandColor	= true;
			node.hasFillColor	= FindColor(pattern, P_C_PATTERN_FILL_COLOR,
				kStandardFill, &node.fillColor);
		} else {
			node.hasBandColor	= (pattern.FindInt32(P_C_PATTERN_FILL_COLOR, &packed) == B_OK);
			node.bandColor		= Unpacked(packed);
			node.hasFillColor	= false;
		}
	} else
		node.hasFillColor	= FindColor(pattern, P_C_PATTERN_FILL_COLOR,
			kStandardFill, &node.fillColor);
	node.hasBorderColor	= FindColor(pattern, P_C_PATTERN_BORDER_COLOR,
		kStandardBorder, &node.borderColor);
	node.hasTextColor	= FindColor(pattern, P_C_PATTERN_HIGH_COLOR,
		kStandardText, &node.textColor);
	message.FindMessage(P_C_NODE_SHAPE, &node.shape);
	nodes.push_back(node);
	return B_OK;
}


status_t GraphDocument::_ReadConnection(const BMessage &message)
{
	GraphConnection	connection;
	if ((message.FindInt32(P_C_NODE_ID, &connection.id) != B_OK)
		|| (message.FindInt32(P_C_NODE_CONNECTION_FROM, &connection.from) != B_OK)
		|| (message.FindInt32(P_C_NODE_CONNECTION_TO, &connection.to) != B_OK)) {
		fprintf(stderr, "GraphDocument: connection without id or ends\n");
		return B_BAD_DATA;
	}
	// its name isn't shown anywhere, so exports leave it out too
	// missing on connections older than arrows: target only, as drawn
	connection.arrows	= message.GetInt8(P_C_NODE_CONNECTION_ARROWS, 1);
	BMessage	pattern;
	message.FindMessage(P_C_NODE_PATTERN, &pattern);
	connection.hasColor	= FindColor(pattern, P_C_PATTERN_FILL_COLOR,
		kStandardConnection, &connection.color);
	connections.push_back(connection);
	return B_OK;
}


// the int32 the editor keeps an rgb_color in
static int32 Packed(rgb_color color)
{
	uint8	bytes[4]	= { color.red, color.green, color.blue, color.alpha };
	int32	packed;
	memcpy(&packed, bytes, sizeof(packed));
	return packed;
}


// an attribute block {Name, Value[, Unit]} of its kind
static BMessage AttributeBlock(const GraphAttribute &attribute)
{
	BMessage	block(attribute.kind);
	block.AddString(P_C_ATTRIBUTE_NAME, attribute.name);
	switch (attribute.kind) {
		case B_BOOL_TYPE:
			block.AddBool(P_C_ATTRIBUTE_VALUE, attribute.value == "true");
			break;
		case P_C_ATTRIBUTE_NUMBER: {
			float	number	= 0;
			BString	unit;
			if (ParseNumberWithUnit(attribute.value.String(), &number, &unit) == B_OK) {
				block.AddFloat(P_C_ATTRIBUTE_VALUE, number);
				block.AddString(P_C_ATTRIBUTE_UNIT, unit);
			} else {
				// not a number after all: kept as text
				block.what	= B_STRING_TYPE;
				block.AddString(P_C_ATTRIBUTE_VALUE, attribute.value);
			}
			break;
		}
		case P_C_ATTRIBUTE_COLOR: {
			unsigned int	red, green, blue;
			if (sscanf(attribute.value.String(), "#%2x%2x%2x", &red, &green, &blue) == 3) {
				rgb_color	color	= make_color(red, green, blue);
				block.AddData(P_C_ATTRIBUTE_VALUE, B_RGB_COLOR_TYPE, &color, sizeof(color));
			} else {
				block.what	= B_STRING_TYPE;
				block.AddString(P_C_ATTRIBUTE_VALUE, attribute.value);
			}
			break;
		}
		default:
			block.AddString(P_C_ATTRIBUTE_VALUE, attribute.value);
	}
	return block;
}


status_t GraphDocument::Write(BPositionIO *out) const
{
	BMessage	allNodes;
	BMessage	allConnections;
	for (size_t i = 0; i < nodes.size(); i++) {
		const GraphNode	&node	= nodes[i];
		BMessage	message(node.isGroup ? P_C_GROUP_TYPE : P_C_CLASS_TYPE);
		message.AddInt32(P_C_NODE_ID, node.id);
		message.AddRect(P_C_NODE_FRAME, node.frame);
		if (FindNode(node.parent) != NULL)
			message.AddInt32(P_C_NODE_PARENT, node.parent);
		for (size_t c = 0; c < nodes.size(); c++) {
			if (node.isGroup && (nodes[c].parent == node.id))
				message.AddInt32(P_C_NODE_CHILD_IDS, nodes[c].id);
		}
		BMessage	data;
		data.AddString(P_C_NODE_NAME, node.name);
		for (size_t a = 0; a < node.attributes.size(); a++) {
			BMessage	block	= AttributeBlock(node.attributes[a]);
			data.AddMessage(node.attributes[a].name.String(), &block);
		}
		message.AddMessage(P_C_NODE_DATA, &data);
		if (node.hasFillColor || node.hasBandColor || node.hasBorderColor
			|| node.hasTextColor) {
			// a pattern without FillColor or PenSize would zero them
			BMessage	pattern;
			pattern.AddInt32(P_C_PATTERN_FILL_COLOR,
				Packed(node.hasFillColor ? node.fillColor : kStandardFill));
			if (node.hasBandColor)
				pattern.AddInt32(P_C_PATTERN_HIGHLIGHT_COLOR, Packed(node.bandColor));
			if (node.hasBorderColor)
				pattern.AddInt32(P_C_PATTERN_BORDER_COLOR, Packed(node.borderColor));
			if (node.hasTextColor)
				pattern.AddInt32(P_C_PATTERN_HIGH_COLOR, Packed(node.textColor));
			pattern.AddFloat(P_C_PATTERN_PEN_SIZE, 1.0);
			message.AddMessage(P_C_NODE_PATTERN, &pattern);
		}
		allNodes.AddMessage(P_C_DOC_ENTRY, &message);
	}
	for (size_t i = 0; i < connections.size(); i++) {
		const GraphConnection	&connection	= connections[i];
		BMessage	message(P_C_CONNECTION_TYPE);
		message.AddInt32(P_C_NODE_ID, connection.id);
		message.AddInt32(P_C_NODE_CONNECTION_FROM, connection.from);
		message.AddInt32(P_C_NODE_CONNECTION_TO, connection.to);
		message.AddInt8(P_C_NODE_CONNECTION_ARROWS, connection.arrows);
		BMessage	data;
		data.AddString(P_C_NODE_NAME, "");
		message.AddMessage(P_C_NODE_DATA, &data);
		if (connection.hasColor) {
			BMessage	pattern;
			pattern.AddInt32(P_C_PATTERN_FILL_COLOR, Packed(connection.color));
			pattern.AddFloat(P_C_PATTERN_PEN_SIZE, 1.0);
			message.AddMessage(P_C_NODE_PATTERN, &pattern);
		}
		allConnections.AddMessage(P_C_DOC_ENTRY, &message);
	}
	BMessage	document;
	// PDocument::Load() refuses documents without a format version
	document.AddInt32(P_C_DOC_FORMAT_VERSION_FIELD, P_C_DOC_FORMAT_VERSION);
	document.AddMessage(P_C_DOC_ALLNODES, &allNodes);
	document.AddMessage(P_C_DOC_ALLCONNECTIONS, &allConnections);
	return document.Flatten(out);
}


const GraphNode *GraphDocument::FindNode(int32 id) const
{
	for (size_t i = 0; i < nodes.size(); i++) {
		if (nodes[i].id == id)
			return &nodes[i];
	}
	return NULL;
}


BRect GraphDocument::Bounds(void) const
{
	BRect	bounds;
	for (size_t i = 0; i < nodes.size(); i++)
		bounds	= bounds.IsValid() ? (bounds | nodes[i].frame) : nodes[i].frame;
	return bounds;
}


BString XmlEscaped(const char *text)
{
	BString	escaped(text);
	escaped.ReplaceAll("&", "&amp;");
	escaped.ReplaceAll("<", "&lt;");
	escaped.ReplaceAll(">", "&gt;");
	escaped.ReplaceAll("\"", "&quot;");
	return escaped;
}


BString ShownValue(const GraphAttribute &attribute)
{
	if (attribute.kind == B_BOOL_TYPE)
		return BString(attribute.value == "true" ? "☑" : "☐");
	return attribute.value;
}


BString HexColor(rgb_color color)
{
	BString	text;
	text.SetToFormat("#%02x%02x%02x", color.red, color.green, color.blue);
	return text;
}
