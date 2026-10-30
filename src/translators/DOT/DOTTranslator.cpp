#include <TranslatorAddOn.h>
#include <TranslationKit.h>

#include <stdio.h>
#include <string.h>

#include "ConfigView.h"
#include "GraphDocument.h"
#include "ProjectConceptorDefs.h"

/** Writes a document as a Graphviz DOT graph (#121): groups become
 * clusters, nodes HTML-like labels with their attributes, and each node keeps its
 * place as pos="x,y!" for `neato -n`. Export only. */

char	translatorName[]	= "DOTTranslator";
char	translatorInfo[]	= "Write ProjectConceptor documents as Graphviz DOT graphs";
int32	translatorVersion	= 100;

translation_format inputFormats[] = {
	{ P_C_DOCUMENT_RAW_TYPE, B_TRANSLATOR_NONE, 0.4, 0.8, P_C_DOCUMENT_MIMETYPE, "ProjectConceptor RAW Document" },
	{ 0, 0, 0, 0, "\0", "\0" }
};

translation_format outputFormats[] = {
	{ P_C_DOT_TYPE, B_TRANSLATOR_TEXT, 0.4, 0.8, "text/vnd.graphviz", "Graphviz DOT" },
	{ 0, 0, 0, 0, "\0", "\0" }
};


// a DOT string between quotes
static BString Quoted(const char *text)
{
	BString	quoted(text);
	quoted.ReplaceAll("\\", "\\\\");
	quoted.ReplaceAll("\"", "\\\"");
	quoted.ReplaceAll("\n", "\\n");
	quoted.Prepend("\"");
	quoted.Append("\"");
	return quoted;
}


// a card like the editor's: a band in the node's color, the name in
// bold, below it one row per attribute
static BString NodeLabel(const GraphNode &node)
{
	BString	label("<<TABLE BORDER=\"1\" STYLE=\"rounded\" CELLBORDER=\"0\" "
		"CELLSPACING=\"0\" CELLPADDING=\"3\"");
	label << " COLOR=\"" << (node.hasBorderColor ? HexColor(node.borderColor)
			: BString("#c4ccd0")) << "\""
		<< " BGCOLOR=\"" << (node.hasFillColor ? HexColor(node.fillColor)
			: BString("#ffffff")) << "\">";
	if (node.hasBandColor)
		label << "<TR><TD COLSPAN=\"2\" HEIGHT=\"4\" BGCOLOR=\""
			<< HexColor(node.bandColor) << "\"></TD></TR>";
	BString	font;
	if (node.hasTextColor)
		font << "<FONT COLOR=\"" << HexColor(node.textColor) << "\">";
	BString	fontEnd(node.hasTextColor ? "</FONT>" : "");
	label << "<TR><TD COLSPAN=\"2\">" << font << "<B>"
		<< XmlEscaped(node.name.String()) << "</B>" << fontEnd << "</TD></TR>";
	for (size_t i = 0; i < node.attributes.size(); i++) {
		BString	value(XmlEscaped(ShownValue(node.attributes[i]).String()));
		value.ReplaceAll("\n", "<BR ALIGN=\"LEFT\"/>");
		label << "<TR><TD ALIGN=\"LEFT\">" << font << "<FONT COLOR=\"#6b7378\">"
			<< XmlEscaped(node.attributes[i].name.String()) << "</FONT>" << fontEnd
			<< "</TD><TD ALIGN=\"LEFT\">" << font << value << fontEnd << "</TD></TR>";
	}
	label << "</TABLE>>";
	return label;
}


class DotWriter {
public:
					DotWriter(const GraphDocument &document)
						: document(document), bounds(document.Bounds()) {}
	BString			Write(void);
private:
	void			_WriteMembers(int32 parent, const BString &indent);
	void			_WriteNode(const GraphNode &node, const BString &indent);
	// the DOT node a connection ends at: a group's hidden anchor
	BString			_EndName(int32 id);
	// the frame's center in points, y up; `neato -n` keeps it
	BString			Position(BRect frame);

	const GraphDocument	&document;
	BRect			bounds;
	BString			out;
};


BString DotWriter::Write(void)
{
	out << "digraph ProjectConceptor {\n"
		<< "\tcompound=true;\n"
		<< "\tnode [shape=none, margin=0, fontname=\"sans-serif\"];\n"
		<< "\tedge [color=\"#7a7a7a\", fontname=\"sans-serif\"];\n";
	_WriteMembers(-1, "\t");
	for (size_t i = 0; i < document.connections.size(); i++) {
		const GraphConnection	&connection	= document.connections[i];
		const GraphNode			*from		= document.FindNode(connection.from);
		const GraphNode			*to			= document.FindNode(connection.to);
		if ((from == NULL) || (to == NULL)) {
			fprintf(stderr, "DOTTranslator: connection %ld to a missing node\n",
				(long)connection.id);
			continue;
		}
		static const char	*dirs[]	= { "none", "forward", "back", "both" };
		out << "\t" << _EndName(from->id) << " -> " << _EndName(to->id)
			<< " [dir=" << dirs[connection.arrows & 3];
		if (from->isGroup)
			out << ", ltail=cluster_" << from->id;
		if (to->isGroup)
			out << ", lhead=cluster_" << to->id;
		if (connection.hasColor)
			out << ", color=\"" << HexColor(connection.color) << "\"";
		out << "];\n";
	}
	out << "}\n";
	return out;
}


void DotWriter::_WriteMembers(int32 parent, const BString &indent)
{
	for (size_t i = 0; i < document.nodes.size(); i++) {
		const GraphNode	&node	= document.nodes[i];
		// a parent that isn't in the document leaves the node on top
		int32			inside	= (document.FindNode(node.parent) != NULL)
			? node.parent : -1;
		if (inside != parent)
			continue;
		if (!node.isGroup) {
			_WriteNode(node, indent);
			continue;
		}
		BString	inner(indent);
		inner << "\t";
		out << indent << "subgraph cluster_" << node.id << " {\n"
			<< inner << "label=" << Quoted(node.name.String()) << ";\n"
			<< inner << "style=\"rounded,filled\";\n"
			<< inner << "fillcolor=\""
				<< (node.hasFillColor ? HexColor(node.fillColor) : BString("#eef3f5"))
				<< "\";\n"
			<< inner << "color=\"#c8d8de\";\n"
			// edges to the group itself end here, clipped by lhead/ltail
			<< inner << "g" << node.id << " [shape=point, style=invis, pos="
				<< Position(node.frame) << "];\n";
		_WriteMembers(node.id, inner);
		out << indent << "}\n";
	}
}


void DotWriter::_WriteNode(const GraphNode &node, const BString &indent)
{
	out << indent << "n" << node.id << " [label=" << NodeLabel(node)
		<< ", pos=" << Position(node.frame) << "];\n";
}


BString DotWriter::Position(BRect frame)
{
	BString	position;
	position << "\"" << (int32)(frame.left + frame.Width() / 2 - bounds.left)
		<< "," << (int32)(bounds.bottom - frame.top - frame.Height() / 2) << "!\"";
	return position;
}


BString DotWriter::_EndName(int32 id)
{
	const GraphNode	*node	= document.FindNode(id);
	BString			name;
	name << ((node != NULL) && node->isGroup ? "g" : "n") << id;
	return name;
}


status_t Identify(BPositionIO *inSource, const translation_format *inFormat,
	BMessage *ioExtension, translator_info *outInfo, uint32 outType)
{
	// runs for every file any app identifies: export only, so nothing
	// but a document asked for as DOT is ours
	if ((outType != P_C_DOT_TYPE) || !LooksLikeDocument(inSource))
		return B_NO_TRANSLATOR;
	outInfo->type		= P_C_DOCUMENT_RAW_TYPE;
	outInfo->group		= B_TRANSLATOR_NONE;
	outInfo->quality	= 0.4;
	outInfo->capability	= 0.8;
	strcpy(outInfo->name, "ProjectConceptor RAW Document");
	strcpy(outInfo->MIME, P_C_DOCUMENT_MIMETYPE);
	return B_OK;
}


status_t Translate(BPositionIO *inSource, const translator_info *tInfo,
	BMessage *ioExtension, uint32 outType, BPositionIO *outDestination)
{
	if (outType != P_C_DOT_TYPE)
		return B_NO_TRANSLATOR;
	inSource->Seek(0, SEEK_SET);
	GraphDocument	document;
	status_t		err	= document.Read(inSource);
	if (err != B_OK)
		return err;
	BString	dot	= DotWriter(document).Write();
	ssize_t	written	= outDestination->Write(dot.String(), dot.Length());
	if (written < 0)
		return (status_t)written;
	return (written == dot.Length()) ? B_OK : B_IO_ERROR;
}


status_t MakeConfig(BMessage *ioExtension, BView **outView, BRect *outExtent)
{
	*outView	= new ConfigView(translatorName);
	return B_OK;
}


status_t GetConfigMessage(BMessage *ioExtension)
{
	ioExtension->AddInt32(P_C_FORMAT_TYPE, P_C_DOT_TYPE);
	ioExtension->AddString(P_C_FORMAT_EXTENSION, "dot");
	return B_OK;
}
