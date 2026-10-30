#ifndef GRAPH_DOCUMENT_H
#define GRAPH_DOCUMENT_H

#include <app/Message.h>
#include <interface/GraphicsDefs.h>
#include <interface/Rect.h>
#include <support/DataIO.h>
#include <support/String.h>

#include <vector>

/** A flattened ProjectConceptor document read into plain structs for the
 * translators that write other formats: nodes and groups in document
 * order, connections by node id. Colors the editor treats as "not set"
 * (its standard pattern) are flagged, so an export can use its own
 * defaults for them, e.g. light and dark variants.
 */

struct GraphAttribute {
	BString		name;
	/** as AttributeValueText() writes it */
	BString		value;
	/** the attribute kind (P_C_*_TYPE of the attribute block) */
	uint32		kind;
};

struct GraphNode {
	int32		id;
	bool		isGroup;
	/** id of the group the node lies in, -1 for none */
	int32		parent;
	BString		name;
	BRect		frame;
	std::vector<GraphAttribute>	attributes;
	/** width of the attribute name column, 0 = fit to the names */
	float		attributeColumn;
	/** the card, its colored band, its border and its text; each only
	 * meaningful where the matching has*Color is set */
	rgb_color	fillColor;
	bool		hasFillColor;
	rgb_color	bandColor;
	bool		hasBandColor;
	rgb_color	borderColor;
	bool		hasBorderColor;
	rgb_color	textColor;
	bool		hasTextColor;
	/** the Node::Shape archive (empty for none) */
	BMessage	shape;
};

struct GraphConnection {
	int32		id;
	int32		from;
	int32		to;
	/** 1 = arrow at the target, 2 = at the source, 3 = both, 0 = none */
	int8		arrows;
	rgb_color	color;
	bool		hasColor;
};

class GraphDocument {
public:
	/** B_BAD_DATA if in holds no ProjectConceptor document */
			status_t		Read(BPositionIO *in);

	/** NULL for an unknown id */
			const GraphNode	*FindNode(int32 id) const;
	/** the frames of all nodes and groups together */
			BRect			Bounds(void) const;

			std::vector<GraphNode>			nodes;
			std::vector<GraphConnection>	connections;

private:
			status_t		_ReadNode(const BMessage &message);
			status_t		_ReadConnection(const BMessage &message);
};

/** whether in starts with a flattened ProjectConceptor document; reads
 * only the first bytes and seeks back */
bool		LooksLikeDocument(BPositionIO *in);

/** text for XML content and attribute values: & < > " escaped */
BString		XmlEscaped(const char *text);

/** #rrggbb */
BString		HexColor(rgb_color color);

#endif
