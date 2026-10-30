#include "SvgWriter.h"

#include <interface/Shape.h>
#include <support/TypeConstants.h>

#include <algorithm>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "ProjectConceptorDefs.h"

// GraphStyle::Card() at the default 12 pt
static const float	kCornerRadius		= 8;
static const float	kStripeHeight		= 4;
static const float	kPaddingX			= 12;
static const float	kPaddingY			= 8;
static const float	kNameSize			= 13;
static const float	kAttributeSize		= 12;
static const float	kRowSpacing			= 4;
static const float	kLineFactor			= 1.3;
static const float	kColumnGap			= 8;
static const float	kGroupRadius		= 14;
static const float	kGroupPadding		= 12;
static const float	kGroupLabelSpace	= 26;
static const float	kMargin				= 24;
// an average glyph of the sans-serif fonts, per point of size
static const float	kCharWidth			= 0.56;


static BString Number(float value)
{
	BString	text;
	text.SetToFormat("%.1f", value);
	if (text.EndsWith(".0"))
		text.Truncate(text.Length() - 2);
	return text;
}


static float TextWidth(const BString &text, float size)
{
	return text.CountChars() * size * kCharWidth;
}


// text that fits width, shortened with "…" at the end or in the middle
static BString Fitted(const BString &text, float size, float width,
	bool middle)
{
	// a column exactly as wide as its text must not lose a char to rounding
	int32	fits	= (int32)(width / (size * kCharWidth) + 0.01f);
	int32	chars	= text.CountChars();
	if (chars <= fits)
		return text;
	if (fits < 2)
		return BString("…");
	BString	fitted;
	if (middle) {
		int32	head	= (fits - 1) / 2;
		int32	tail	= fits - 1 - head;
		BString	end;
		text.CopyCharsInto(fitted, 0, head);
		text.CopyCharsInto(end, chars - tail, tail);
		fitted << "…" << end;
	} else {
		text.CopyCharsInto(fitted, 0, fits - 1);
		fitted << "…";
	}
	return fitted;
}


// a value as one row of a card
static BString RowText(const GraphAttribute &attribute)
{
	BString	text(ShownValue(attribute));
	if ((attribute.kind == P_C_ATTRIBUTE_FILE) || (attribute.kind == P_C_ATTRIBUTE_IMAGE)) {
		int32	slash	= text.FindLast('/');
		if (slash >= 0)
			text.Remove(0, slash + 1);
	}
	int32	newline	= text.FindFirst('\n');
	if (newline >= 0) {
		text.Truncate(newline);
		text << " …";
	}
	return text;
}


// only addresses a browser opens as a page or a mail
static bool IsWebAddress(const BString &text)
{
	return text.StartsWith("http://") || text.StartsWith("https://")
		|| text.StartsWith("mailto:") || text.StartsWith("ftp://");
}


static BString JsonString(const char *text)
{
	BString	json("\"");
	for (const char *c = text; *c != '\0'; c++) {
		switch (*c) {
			case '"':	json << "\\\"";	break;
			case '\\':	json << "\\\\";	break;
			case '\n':	json << "\\n";	break;
			case '\r':	json << "\\r";	break;
			case '\t':	json << "\\t";	break;
			// "</script>" would end the page's script block
			case '/':	json << ((c != text) && (c[-1] == '<') ? "\\/" : "/");	break;
			default:
				if ((uint8)*c < 0x20) {
					char	escaped[8];
					sprintf(escaped, "\\u%04x", *c);
					json << escaped;
				} else
					json << *c;
		}
	}
	json << "\"";
	return json;
}


/** a node's outline as SVG path data, its 0..1 points scaled to frame */
class PathBuilder : public BShapeIterator {
public:
	PathBuilder(BRect frame) : fFrame(frame) {}

	virtual status_t IterateMoveTo(BPoint *point)
	{
		fPath << "M" << _Point(*point);
		return B_OK;
	}

	virtual status_t IterateLineTo(int32 lineCount, BPoint *linePoints)
	{
		for (int32 i = 0; i < lineCount; i++)
			fPath << "L" << _Point(linePoints[i]);
		return B_OK;
	}

	virtual status_t IterateBezierTo(int32 bezierCount, BPoint *bezierPoints)
	{
		for (int32 i = 0; i < bezierCount; i++) {
			fPath << "C" << _Point(bezierPoints[i * 3]) << " "
				<< _Point(bezierPoints[i * 3 + 1]) << " "
				<< _Point(bezierPoints[i * 3 + 2]);
		}
		return B_OK;
	}

	virtual status_t IterateClose(void)
	{
		fPath << "Z";
		return B_OK;
	}

	BString Path(void) const { return fPath; }

private:
	BString _Point(BPoint point)
	{
		BString	text;
		text << Number(fFrame.left + point.x * fFrame.Width()) << " "
			<< Number(fFrame.top + point.y * fFrame.Height());
		return text;
	}

	BRect	fFrame;
	BString	fPath;
};


// the outline's path and the part of it text goes in (0..1); an empty
// path means the editor's rounded rectangle
static BString OutlinePath(const GraphNode &node, BRect frame, BRect *textRect)
{
	textRect->Set(0, 0, 1, 1);
	if (node.shape.IsEmpty() || (strcmp(node.shape.GetString(P_C_SHAPE_NAME, "rounded"),
			"rounded") == 0))
		return BString();
	BMessage	copy(node.shape);
	BShape		shape(&copy);
	PathBuilder	builder(frame);
	builder.Iterate(&shape);
	BRect		stored	= node.shape.GetRect(P_C_SHAPE_TEXT_RECT, BRect(0, 0, 1, 1));
	if (stored.IsValid())
		*textRect	= stored;
	return builder.Path();
}


static float NameLineHeight(void)
{
	return kNameSize * kLineFactor;
}


static float RowHeight(void)
{
	return kAttributeSize * kLineFactor + kRowSpacing;
}


SvgWriter::SvgWriter(const GraphDocument &document)
	:
	document(document)
{
	_Layout();
}


void SvgWriter::_Layout(void)
{
	for (size_t i = 0; i < document.nodes.size(); i++) {
		const GraphNode	&node	= document.nodes[i];
		BRect			frame	= node.frame;
		BRect			textRect;
		if (!node.isGroup && OutlinePath(node, frame, &textRect).Length() == 0) {
			// a card grows with its rows; the stored height is its minimum
			float	content	= kStripeHeight + kPaddingY + NameLineHeight()
				+ node.attributes.size() * RowHeight() + kPaddingY;
			if (frame.Height() < content)
				frame.bottom	= frame.top + content;
		}
		frames[node.id]	= frame;
	}
	// groups around their members, the innermost first
	std::vector<std::pair<int32, int32> >	groups;
	for (size_t i = 0; i < document.nodes.size(); i++) {
		if (!document.nodes[i].isGroup)
			continue;
		int32				depth	= 0;
		const GraphNode		*parent	= document.FindNode(document.nodes[i].parent);
		for (; (parent != NULL) && (depth < 64); depth++)
			parent	= document.FindNode(parent->parent);
		groups.push_back(std::make_pair(-depth, document.nodes[i].id));
	}
	std::sort(groups.begin(), groups.end());
	for (size_t g = 0; g < groups.size(); g++) {
		int32	id		= groups[g].second;
		BRect	frame	= frames[id];
		for (size_t i = 0; i < document.nodes.size(); i++) {
			if (document.nodes[i].parent != id)
				continue;
			BRect	member	= frames[document.nodes[i].id];
			member.left		-= kGroupPadding;
			member.right	+= kGroupPadding;
			member.top		-= kGroupLabelSpace;
			member.bottom	+= kGroupPadding;
			frame	= frame.IsValid() ? (frame | member) : member;
		}
		frames[id]	= frame;
	}
}


BString SvgWriter::Svg(bool standalone, const char *title)
{
	out		= "";
	defs	= "";
	markers.clear();
	BRect	bounds;
	for (std::map<int32, BRect>::iterator i = frames.begin(); i != frames.end(); i++)
		bounds	= bounds.IsValid() ? (bounds | i->second) : i->second;
	if (!bounds.IsValid())
		bounds.Set(0, 0, 200, 100);
	bounds.InsetBy(-kMargin, -kMargin);

	BString	body;
	// groups behind everything, outer ones first
	for (int32 depth = 0; depth < 64; depth++) {
		bool	any	= false;
		for (size_t i = 0; i < document.nodes.size(); i++) {
			const GraphNode	&node	= document.nodes[i];
			if (!node.isGroup)
				continue;
			int32			level	= 0;
			const GraphNode	*parent	= document.FindNode(node.parent);
			for (; (parent != NULL) && (level < 64); level++)
				parent	= document.FindNode(parent->parent);
			if (level == depth) {
				_WriteGroup(node);
				any	= true;
			}
		}
		if (!any)
			break;
	}
	for (size_t i = 0; i < document.connections.size(); i++)
		_WriteConnection(document.connections[i]);
	for (size_t i = 0; i < document.nodes.size(); i++) {
		if (!document.nodes[i].isGroup)
			_WriteNode(document.nodes[i]);
	}
	body	= out;

	BString	svg;
	if (standalone)
		svg << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" class=\"pc-graph\" "
		<< "viewBox=\"" << Number(bounds.left) << " " << Number(bounds.top) << " "
		<< Number(bounds.Width()) << " " << Number(bounds.Height()) << "\" "
		<< "width=\"" << Number(bounds.Width()) << "\" height=\""
		<< Number(bounds.Height()) << "\">\n";
	// in a page the title would be the tooltip of the whole picture
	if (standalone) {
		svg << "<title>" << XmlEscaped(title) << "</title>\n"
			<< "<style>\n" << Style() << "</style>\n";
	}
	svg << "<defs>\n"
		<< "<filter id=\"pc-shadow\" x=\"-20%\" y=\"-20%\" width=\"140%\" height=\"150%\">"
		<< "<feDropShadow dx=\"0\" dy=\"3\" stdDeviation=\"4\" flood-opacity=\"0.18\"/>"
		<< "</filter>\n"
		<< defs << "</defs>\n";
	svg << "<rect class=\"pc-canvas\" x=\"" << Number(bounds.left) << "\" y=\""
		<< Number(bounds.top) << "\" width=\"" << Number(bounds.Width())
		<< "\" height=\"" << Number(bounds.Height()) << "\"/>\n"
		<< body << "</svg>\n";
	return svg;
}


void SvgWriter::_WriteGroup(const GraphNode &group)
{
	BRect	frame	= frames[group.id];
	out << "<g class=\"pc-group\" id=\"n" << group.id << "\" data-id=\""
		<< group.id << "\">\n"
		<< "<rect class=\"pc-group-box\" x=\"" << Number(frame.left) << "\" y=\""
		<< Number(frame.top) << "\" width=\"" << Number(frame.Width())
		<< "\" height=\"" << Number(frame.Height()) << "\" rx=\""
		<< Number(kGroupRadius) << "\"";
	if (group.hasFillColor)
		out << " style=\"fill:" << HexColor(group.fillColor) << "\"";
	out << "/>\n"
		<< "<text class=\"pc-group-label\" x=\"" << Number(frame.left + kGroupPadding)
		<< "\" y=\"" << Number(frame.top + kGroupLabelSpace - 9) << "\">"
		<< XmlEscaped(Fitted(group.name, kAttributeSize,
			frame.Width() - 2 * kGroupPadding, true).String())
		<< "</text>\n</g>\n";
}


BString SvgWriter::_Marker(const GraphConnection &connection)
{
	BString	key(connection.hasColor ? HexColor(connection.color) : BString());
	std::map<BString, BString>::iterator	found	= markers.find(key);
	if (found != markers.end())
		return found->second;
	BString	id("pc-arrow");
	if (key.Length() > 0)
		id << "-" << (key.String() + 1);
	defs << "<marker id=\"" << id << "\" viewBox=\"0 0 10 10\" refX=\"9\" refY=\"5\" "
		<< "markerWidth=\"7\" markerHeight=\"7\" orient=\"auto-start-reverse\">"
		<< "<path d=\"M0 0L10 5L0 10Z\" class=\"pc-arrow\"";
	if (key.Length() > 0)
		defs << " style=\"fill:" << key << "\"";
	defs << "/></marker>\n";
	markers[key]	= id;
	return id;
}


void SvgWriter::_WriteConnection(const GraphConnection &connection)
{
	std::map<int32, BRect>::iterator	from	= frames.find(connection.from);
	std::map<int32, BRect>::iterator	to		= frames.find(connection.to);
	if ((from == frames.end()) || (to == frames.end())) {
		fprintf(stderr, "HTMLTranslator: connection %ld to a missing node\n",
			(long)connection.id);
		return;
	}
	// the nearest of the four side midpoints, like the editor's connectors
	BRect	a	= from->second;
	BRect	b	= to->second;
	BPoint	starts[4]	= { BPoint(a.left, (a.top + a.bottom) / 2),
		BPoint((a.left + a.right) / 2, a.top), BPoint(a.right, (a.top + a.bottom) / 2),
		BPoint((a.left + a.right) / 2, a.bottom) };
	BPoint	ends[4]		= { BPoint(b.left, (b.top + b.bottom) / 2),
		BPoint((b.left + b.right) / 2, b.top), BPoint(b.right, (b.top + b.bottom) / 2),
		BPoint((b.left + b.right) / 2, b.bottom) };
	BPoint	start		= starts[0];
	BPoint	end			= ends[0];
	float	shortest	= -1;
	for (int32 i = 0; i < 4; i++) {
		for (int32 j = 0; j < 4; j++) {
			float	dx			= ends[j].x - starts[i].x;
			float	dy			= ends[j].y - starts[i].y;
			float	distance	= dx * dx + dy * dy;
			if ((shortest < 0) || (distance < shortest)) {
				shortest	= distance;
				start		= starts[i];
				end			= ends[j];
			}
		}
	}
	BString	marker	= _Marker(connection);
	out << "<line class=\"pc-edge\" data-from=\"" << connection.from
		<< "\" data-to=\"" << connection.to << "\" x1=\"" << Number(start.x)
		<< "\" y1=\"" << Number(start.y) << "\" x2=\"" << Number(end.x)
		<< "\" y2=\"" << Number(end.y) << "\"";
	if ((connection.arrows & 1) != 0)
		out << " marker-end=\"url(#" << marker << ")\"";
	if ((connection.arrows & 2) != 0)
		out << " marker-start=\"url(#" << marker << ")\"";
	if (connection.hasColor)
		out << " style=\"stroke:" << HexColor(connection.color) << "\"";
	out << "/>\n";
}


void SvgWriter::_WriteNode(const GraphNode &node)
{
	BRect	frame	= frames[node.id];
	BRect	textRect;
	BString	path	= OutlinePath(node, frame, &textRect);
	BString	outline;
	if (path.Length() > 0)
		outline << "<path d=\"" << path << "\"";
	else {
		outline << "<rect x=\"" << Number(frame.left) << "\" y=\"" << Number(frame.top)
			<< "\" width=\"" << Number(frame.Width()) << "\" height=\""
			<< Number(frame.Height()) << "\" rx=\"" << Number(kCornerRadius) << "\"";
	}
	defs << "<clipPath id=\"pc-clip-" << node.id << "\">" << outline << "/></clipPath>\n";

	BString	textStyle;
	if (node.hasTextColor)
		textStyle << " style=\"fill:" << HexColor(node.textColor) << "\"";
	out << "<g class=\"pc-node\" id=\"n" << node.id << "\" data-id=\"" << node.id << "\">\n"
		<< "<title>" << XmlEscaped(node.name.String()) << "</title>\n"
		<< outline << " class=\"pc-card\"";
	if (node.hasFillColor || node.hasBorderColor) {
		out << " style=\"";
		if (node.hasFillColor)
			out << "fill:" << HexColor(node.fillColor) << ";";
		if (node.hasBorderColor)
			out << "stroke:" << HexColor(node.borderColor) << ";";
		out << "\"";
	}
	out << "/>\n<g clip-path=\"url(#pc-clip-" << node.id << ")\">\n";
	if (node.hasBandColor) {
		out << "<rect x=\"" << Number(frame.left) << "\" y=\"" << Number(frame.top)
			<< "\" width=\"" << Number(frame.Width()) << "\" height=\""
			<< Number(kStripeHeight) << "\" style=\"fill:"
			<< HexColor(node.bandColor) << "\"/>\n";
	}

	BRect	inner(frame.left + textRect.left * frame.Width(),
		frame.top + textRect.top * frame.Height(),
		frame.left + textRect.right * frame.Width(),
		frame.top + textRect.bottom * frame.Height());
	float	left	= inner.left + kPaddingX;
	float	width	= inner.Width() - 2 * kPaddingX;
	float	y		= inner.top + kStripeHeight + kPaddingY + kNameSize;
	out << "<text class=\"pc-name\" x=\"" << Number(left) << "\" y=\"" << Number(y)
		<< "\"" << textStyle << ">"
		<< XmlEscaped(Fitted(node.name, kNameSize, width, true).String()) << "</text>\n";

	float	column	= node.attributeColumn;
	if (column <= 0) {
		// the names get what the values leave, but at least 45 %
		float	values	= 0;
		for (size_t i = 0; i < node.attributes.size(); i++) {
			column	= std::max(column, TextWidth(node.attributes[i].name, kAttributeSize));
			values	= std::max(values, TextWidth(RowText(node.attributes[i]), kAttributeSize)
				+ (node.attributes[i].kind == P_C_ATTRIBUTE_COLOR ? 14 : 0));
		}
		column	= std::min(column, std::max(width * 0.45f, width - kColumnGap - values));
	}
	y	+= NameLineHeight() - kNameSize + kRowSpacing;
	for (size_t i = 0; i < node.attributes.size(); i++) {
		const GraphAttribute	&attribute	= node.attributes[i];
		y	+= RowHeight();
		float	valueLeft	= left + column + kColumnGap;
		float	valueWidth	= left + width - valueLeft;
		out << "<text class=\"pc-key\" x=\"" << Number(left) << "\" y=\"" << Number(y)
			<< "\">" << XmlEscaped(Fitted(attribute.name, kAttributeSize, column,
				true).String()) << "</text>\n";
		if (attribute.kind == P_C_ATTRIBUTE_COLOR) {
			out << "<rect class=\"pc-swatch\" x=\"" << Number(valueLeft) << "\" y=\""
				<< Number(y - 9) << "\" width=\"10\" height=\"10\" rx=\"2\" style=\"fill:"
				<< XmlEscaped(attribute.value.String()) << "\"/>\n";
			valueLeft	+= 14;
			valueWidth	-= 14;
		}
		BString	value	= XmlEscaped(Fitted(RowText(attribute), kAttributeSize,
			valueWidth, false).String());
		bool	link	= (attribute.kind == P_C_ATTRIBUTE_LINK)
			&& IsWebAddress(attribute.value);
		if (link)
			out << "<a href=\"" << XmlEscaped(attribute.value.String()) << "\">";
		out << "<text class=\"pc-value" << (link ? " pc-link" : "") << "\" x=\""
			<< Number(valueLeft) << "\" y=\"" << Number(y) << "\""
			<< (link ? BString() : textStyle) << ">" << value << "</text>";
		if (link)
			out << "</a>";
		out << "\n";
	}
	out << "</g>\n</g>\n";
}


BString SvgWriter::NodesJson(void)
{
	BString	json("[");
	for (size_t i = 0; i < document.nodes.size(); i++) {
		const GraphNode	&node	= document.nodes[i];
		if (i > 0)
			json << ",";
		json << "\n{\"id\":" << node.id << ",\"group\":"
			<< (node.isGroup ? "true" : "false") << ",\"name\":"
			<< JsonString(node.name.String()) << ",\"attributes\":[";
		for (size_t a = 0; a < node.attributes.size(); a++) {
			const char	*kind	= "text";
			switch (node.attributes[a].kind) {
				case B_BOOL_TYPE:			kind	= "bool";	break;
				case P_C_ATTRIBUTE_NOTE:	kind	= "note";	break;
				case P_C_ATTRIBUTE_LINK:	kind	= "link";	break;
				case P_C_ATTRIBUTE_COLOR:	kind	= "color";	break;
				case P_C_ATTRIBUTE_FILE:	kind	= "file";	break;
				case P_C_ATTRIBUTE_IMAGE:	kind	= "image";	break;
				case P_C_ATTRIBUTE_DATE:	kind	= "date";	break;
			}
			if (a > 0)
				json << ",";
			json << "[" << JsonString(node.attributes[a].name.String()) << ","
				<< JsonString(node.attributes[a].value.String()) << ",\""
				<< kind << "\"]";
		}
		json << "]}";
	}
	json << "\n]";
	return json;
}


BString SvgWriter::Style(void)
{
	return BString(
		".pc-graph{--canvas:#f6f6f6;--card:#ffffff;--card-border:#dedede;"
		"--text:#1d1d1d;--muted:#6b6b6b;--edge:#8c8c8c;--accent:#4a7ab5;"
		"--group-fill:rgba(74,122,181,.07);--group-border:rgba(74,122,181,.2);"
		"--group-label:#304a6b;"
		"font-family:system-ui,-apple-system,'Noto Sans','Segoe UI',sans-serif}\n"
		"@media (prefers-color-scheme:dark){.pc-graph{--canvas:#1e1f22;"
		"--card:#2a2b2e;--card-border:#45474b;--text:#e6e6e6;--muted:#a0a3a8;"
		"--edge:#8a8d92;--accent:#6c9bd2;--group-fill:rgba(108,155,210,.11);"
		"--group-border:rgba(108,155,210,.28);--group-label:#a9c3e2}}\n"
		".pc-canvas{fill:var(--canvas)}\n"
		".pc-card{fill:var(--card);stroke:var(--card-border);stroke-width:1;"
		"filter:url(#pc-shadow)}\n"
		".pc-name{fill:var(--text);font-size:13px;font-weight:600}\n"
		".pc-key{fill:var(--muted);font-size:12px}\n"
		".pc-value{fill:var(--text);font-size:12px}\n"
		".pc-link{fill:var(--accent);text-decoration:underline}\n"
		".pc-swatch{stroke:var(--card-border)}\n"
		".pc-edge{stroke:var(--edge);stroke-width:1.5;fill:none}\n"
		".pc-arrow{fill:var(--edge)}\n"
		".pc-group-box{fill:var(--group-fill);stroke:var(--group-border);stroke-width:1}\n"
		".pc-group-label{fill:var(--group-label);font-size:12px;font-weight:600}\n");
}
