#include <TranslatorAddOn.h>
#include <TranslationKit.h>

#include <stdio.h>
#include <string.h>

#include "ConfigView.h"
#include "GraphDocument.h"
#include "ProjectConceptorDefs.h"

/** Writes a document as a Mermaid flowchart (#123), bare or inside a
 * Markdown file, which GitHub, GitLab and many editors show as a diagram:
 * groups become subgraphs, attributes lines below the name. Export only. */

char	translatorName[]	= "MermaidTranslator";
char	translatorInfo[]	= "Write ProjectConceptor documents as Mermaid flowcharts";
int32	translatorVersion	= 100;

translation_format inputFormats[] = {
	{ P_C_DOCUMENT_RAW_TYPE, B_TRANSLATOR_NONE, 0.4, 0.8, P_C_DOCUMENT_MIMETYPE, "ProjectConceptor RAW Document" },
	{ 0, 0, 0, 0, "\0", "\0" }
};

translation_format outputFormats[] = {
	{ P_C_MERMAID_TYPE, B_TRANSLATOR_TEXT, 0.4, 0.8, "text/x-mermaid", "Mermaid flowchart" },
	{ P_C_MARKDOWN_TYPE, B_TRANSLATOR_TEXT, 0.4, 0.8, "text/markdown", "Markdown with Mermaid" },
	{ 0, 0, 0, 0, "\0", "\0" }
};


// text inside a Mermaid "..." label: its own entity codes, line breaks
static BString Label(const char *text)
{
	BString	label(text);
	label.ReplaceAll("#", "#35;");
	label.ReplaceAll("\"", "#quot;");
	label.ReplaceAll("<", "#lt;");
	label.ReplaceAll(">", "#gt;");
	label.ReplaceAll("\n", "<br/>");
	return label;
}


class MermaidWriter {
public:
					MermaidWriter(const GraphDocument &document)
						: document(document) {}
	BString			Write(void);
private:
	void			_WriteMembers(int32 parent, const BString &indent);
	BString			_Name(int32 id);

	const GraphDocument	&document;
	BString			out;
	BString			styles;
};


BString MermaidWriter::Write(void)
{
	BRect	bounds	= document.Bounds();
	out << "flowchart " << (bounds.Width() >= bounds.Height() ? "LR" : "TD") << "\n";
	_WriteMembers(-1, "  ");
	// linkStyle counts the links written
	int32	links	= 0;
	for (size_t i = 0; i < document.connections.size(); i++) {
		const GraphConnection	&connection	= document.connections[i];
		if ((document.FindNode(connection.from) == NULL)
			|| (document.FindNode(connection.to) == NULL)) {
			fprintf(stderr, "MermaidTranslator: connection %ld to a missing node\n",
				(long)connection.id);
			continue;
		}
		static const char	*arrows[]	= { "---", "-->", "-->", "<-->" };
		int32	kind	= connection.arrows & 3;
		// an arrow only at the source points back: written the other way
		if (kind == 2)
			out << "  " << _Name(connection.to) << " " << arrows[kind] << " "
				<< _Name(connection.from) << "\n";
		else
			out << "  " << _Name(connection.from) << " " << arrows[kind] << " "
				<< _Name(connection.to) << "\n";
		if (connection.hasColor)
			styles << "  linkStyle " << links << " stroke:"
				<< HexColor(connection.color) << "\n";
		links++;
	}
	out << styles;
	return out;
}


void MermaidWriter::_WriteMembers(int32 parent, const BString &indent)
{
	for (size_t i = 0; i < document.nodes.size(); i++) {
		const GraphNode	&node	= document.nodes[i];
		// a parent that isn't in the document leaves the node on top
		int32			inside	= (document.FindNode(node.parent) != NULL)
			? node.parent : -1;
		if (inside != parent)
			continue;
		if (node.isGroup) {
			BString	inner(indent);
			inner << "  ";
			out << indent << "subgraph " << _Name(node.id) << " [\""
				<< Label(node.name.String()) << "\"]\n";
			_WriteMembers(node.id, inner);
			out << indent << "end\n";
			if (node.hasFillColor)
				styles << "  style " << _Name(node.id) << " fill:"
					<< HexColor(node.fillColor) << "\n";
			continue;
		}
		out << indent << _Name(node.id) << "[\"<b>" << Label(node.name.String()) << "</b>";
		for (size_t a = 0; a < node.attributes.size(); a++) {
			out << "<br/>" << Label(node.attributes[a].name.String()) << ": "
				<< Label(ShownValue(node.attributes[a]).String());
		}
		out << "\"]\n";
		BString	style;
		if (node.hasFillColor)
			style << "fill:" << HexColor(node.fillColor);
		// the band's color has no place of its own: the border takes it
		if (node.hasBandColor || node.hasBorderColor) {
			style << (style.Length() > 0 ? "," : "") << "stroke:"
				<< HexColor(node.hasBorderColor ? node.borderColor : node.bandColor);
		}
		if (node.hasTextColor)
			style << (style.Length() > 0 ? "," : "") << "color:" << HexColor(node.textColor);
		if (style.Length() > 0)
			styles << "  style " << _Name(node.id) << " " << style << "\n";
	}
}


BString MermaidWriter::_Name(int32 id)
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
	// but a document asked for as Mermaid is ours
	if (((outType != P_C_MERMAID_TYPE) && (outType != P_C_MARKDOWN_TYPE))
		|| !LooksLikeDocument(inSource))
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
	if ((outType != P_C_MERMAID_TYPE) && (outType != P_C_MARKDOWN_TYPE))
		return B_NO_TRANSLATOR;
	inSource->Seek(0, SEEK_SET);
	GraphDocument	document;
	status_t		err	= document.Read(inSource);
	if (err != B_OK)
		return err;
	BString	text	= MermaidWriter(document).Write();
	if (outType == P_C_MARKDOWN_TYPE) {
		const char	*title	= (ioExtension != NULL)
			? ioExtension->GetString(P_C_FORMAT_TITLE, "ProjectConceptor")
			: "ProjectConceptor";
		BString	markdown;
		markdown << "# " << title << "\n\n```mermaid\n" << text << "```\n";
		text	= markdown;
	}
	ssize_t	written	= outDestination->Write(text.String(), text.Length());
	if (written < 0)
		return (status_t)written;
	return (written == text.Length()) ? B_OK : B_IO_ERROR;
}


status_t MakeConfig(BMessage *ioExtension, BView **outView, BRect *outExtent)
{
	*outView	= new ConfigView(translatorName);
	return B_OK;
}


status_t GetConfigMessage(BMessage *ioExtension)
{
	ioExtension->AddInt32(P_C_FORMAT_TYPE, P_C_MERMAID_TYPE);
	ioExtension->AddString(P_C_FORMAT_EXTENSION, "mmd");
	ioExtension->AddInt32(P_C_FORMAT_TYPE, P_C_MARKDOWN_TYPE);
	ioExtension->AddString(P_C_FORMAT_EXTENSION, "md");
	return B_OK;
}
