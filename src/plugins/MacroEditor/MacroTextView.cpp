#include "MacroTextView.h"

#include <interface/InterfaceDefs.h>
#include <interface/Window.h>

#include "MacroEditor.h"
#include "ProjectConceptorDefs.h"

// mirrors MacroText.cpp's own convention (SerializeFieldLines()/ParseMacro()):
// two spaces per nesting level.
static const int32 kSpacesPerDepth = 2;

// the exact rendered DSL header line SerializeFieldLines() emits for the
// "included_node" field (see Insert.cpp's kInsertProperties comment for
// where that field name comes from) - the "~" is MacroText.cpp's own block
// marker, not part of the field name itself, but has to be matched here too
// since it's what actually appears in the rendered text.
static const char *kIncludedNodeHeader = "~included_node";


static BString LineText(const BString &text, int32 start, int32 end)
{
	BString line;
	text.CopyInto(line, start, end - start);
	return line;
}


static int32 LineDepth(const BString &line)
{
	int32	spaces	= 0;
	while ((spaces < line.Length()) && (line[spaces] == ' '))
		spaces++;
	return spaces / kSpacesPerDepth;
}


static BString Trimmed(const BString &line)
{
	BString	trimmed(line);
	trimmed.Trim();
	return trimmed;
}


// the exact leading whitespace of `line`, so a rebuilt placeholder lines up
// with whatever depth the block was actually found at.
static BString LeadingWhitespace(const BString &line)
{
	int32	spaces	= 0;
	while ((spaces < line.Length()) && (line[spaces] == ' '))
		spaces++;
	BString	indent;
	line.CopyInto(indent, 0, spaces);
	return indent;
}


// end offset (exclusive, right after the line's own trailing "\n", or
// TextLength() for the very last line) of the "~included_node" block whose
// header line is [headerStart,headerEnd) - every following line whose depth
// is greater than the header's own depth belongs to it. A blank line doesn't
// end the block (matches ParseMacro()'s own leniency - "blank or comment
// line - skip, no depth/tree effect"), it just can't happen in practice
// since SerializeCommands() never emits one.
static int32 FindBlockEnd(const BString &text, int32 headerStart, int32 headerEnd)
{
	int32	headerDepth	= LineDepth(LineText(text,headerStart,headerEnd));
	int32	lineStart	= headerEnd;
	int32	textLength	= text.Length();
	while (lineStart < textLength) {
		int32	nl	= text.FindFirst("\n",lineStart);
		int32	lineEnd	= (nl >= 0) ? nl+1 : textLength;
		BString	line	= LineText(text,lineStart,lineEnd);
		BString	trimmed	= Trimmed(line);
		if ((trimmed.Length() > 0) && (LineDepth(line) <= headerDepth))
			break;
		lineStart	= lineEnd;
	}
	return lineStart;
}


// "Connection" if the block carries Connection::type (checked first - a
// connection's own embedded Node::Data still carries a filler name, see
// GenerateInsertCommand(), so checking name first would always win and
// "Connection" would never actually show), else "Node::name="..."" if
// present, else a generic placeholder - never blank, so a folded chip
// always gives some hint what's inside without opening it.
static void ExtractLabel(const BString &blockText, BString *outLabel)
{
	BString	connectionNeedle(P_C_NODE_CONNECTION_TYPE);
	connectionNeedle	<< "=";
	if (blockText.FindFirst(connectionNeedle) >= 0) {
		*outLabel	= "Connection";
		return;
	}
	BString	nameNeedle(P_C_NODE_NAME);
	nameNeedle	<< "=\"";
	int32	nameAt	= blockText.FindFirst(nameNeedle);
	if (nameAt >= 0) {
		int32	valueStart	= nameAt+nameNeedle.Length();
		int32	valueEnd	= blockText.FindFirst("\"",valueStart);
		if (valueEnd > valueStart) {
			BString	name;
			blockText.CopyInto(name,valueStart,valueEnd-valueStart);
			*outLabel	= BString("\"") << name << "\"";
			return;
		}
	}
	*outLabel	= "{...}";
}


// true, with *outId set, for a line whose trimmed content is exactly
// "~included_node[<digits>]" possibly followed by " <label>" - the
// placeholder form FoldBlock()/SetMacroText() build. Never matches a real,
// expanded "~included_node" header (no "[" there at all).
static bool IsFoldedPlaceholder(const BString &trimmed, int32 *outId)
{
	BString	prefix(kIncludedNodeHeader);
	prefix	<< "[";
	if (!trimmed.StartsWith(prefix))
		return false;
	int32	closeBracket	= trimmed.FindFirst("]",prefix.Length());
	if (closeBracket < 0)
		return false;
	BString	idText;
	trimmed.CopyInto(idText,prefix.Length(),closeBracket-prefix.Length());
	if (idText.Length() == 0)
		return false;
	for (int32 i=0;i<idText.Length();i++)
		if ((idText[i] < '0') || (idText[i] > '9'))
			return false;
	*outId	= atol(idText.String());
	return true;
}


static bool IsExpandedHeader(const BString &trimmed)
{
	return trimmed == BString(kIncludedNodeHeader);
}


MacroTextView::MacroTextView(BRect frame, const char *name, BRect textRect,
	uint32 resizingMode, uint32 flags)
	:BTextView(frame,name,textRect,resizingMode,flags)
{
	fEditor	= NULL;
}


void MacroTextView::KeyDown(const char *bytes, int32 numBytes)
{
	BTextView::KeyDown(bytes,numBytes);
	// B_RETURN and B_ENTER are the same byte (0x0a) in Haiku - covers both
	// the main Return key and the numpad Enter key uniformly.
	if ((fEditor != NULL) && (numBytes == 1) && (bytes[0] == B_ENTER))
		fEditor->ApplyEdits();
}


void MacroTextView::MakeFocus(bool focused)
{
	bool	wasFocused	= IsFocus();
	BTextView::MakeFocus(focused);
	// losing focus (switching macros, tabs, or clicking elsewhere) is the
	// other natural pause point - without this, a last line typed but
	// never terminated with Enter would silently be lost the moment
	// ShowSelectedMacro() overwrites this view's text with the last
	// applied version.
	if (wasFocused && (!focused) && (fEditor != NULL))
		fEditor->ApplyEdits();
}


void MacroTextView::MouseDown(BPoint where)
{
	BTextView::MouseDown(where);
	int32	clicks	= 1;
	if (Window() != NULL && Window()->CurrentMessage() != NULL)
		Window()->CurrentMessage()->FindInt32("clicks",&clicks);
	if (clicks != 2)
		return;
	int32	lineIndex	= LineAt(where);
	int32	lineStart	= OffsetAt(lineIndex);
	int32	lineEnd		= (lineIndex+1 < CountLines())
		? OffsetAt(lineIndex+1) : TextLength();
	ToggleFoldAtLine(lineStart,lineEnd);
}


void MacroTextView::ToggleFoldAtLine(int32 lineStart, int32 lineEnd)
{
	BString	fullText(Text());
	BString	line	= LineText(fullText,lineStart,lineEnd);
	BString	trimmed	= Trimmed(line);
	int32	foldedId	= -1;

	if (IsFoldedPlaceholder(trimmed,&foldedId)) {
		if ((foldedId < 0) || (foldedId >= (int32)fFoldedBlockText.size()))
			return;
		const BString	&blockText	= fFoldedBlockText[foldedId];
		Delete(lineStart,lineEnd);
		Insert(lineStart,blockText.String(),blockText.Length());
		return;
	}

	if (IsExpandedHeader(trimmed)) {
		int32	blockEnd	= FindBlockEnd(fullText,lineStart,lineEnd);
		BString	blockText	= LineText(fullText,lineStart,blockEnd);
		BString	label;
		ExtractLabel(blockText,&label);
		int32	id	= (int32)fFoldedBlockText.size();
		fFoldedBlockText.push_back(blockText);
		BString	placeholder	= LeadingWhitespace(line);
		placeholder	<< kIncludedNodeHeader << "[" << id << "] " << label << "\n";
		Delete(lineStart,blockEnd);
		Insert(lineStart,placeholder.String(),placeholder.Length());
		return;
	}
}


void MacroTextView::SetMacroText(const BString &canonicalText)
{
	fFoldedBlockText.clear();
	BString	folded;
	int32	lineStart	= 0;
	int32	textLength	= canonicalText.Length();
	while (lineStart < textLength) {
		int32	nl	= canonicalText.FindFirst("\n",lineStart);
		int32	lineEnd	= (nl >= 0) ? nl+1 : textLength;
		BString	line	= LineText(canonicalText,lineStart,lineEnd);
		BString	trimmed	= Trimmed(line);
		if (IsExpandedHeader(trimmed)) {
			int32	blockEnd	= FindBlockEnd(canonicalText,lineStart,lineEnd);
			BString	blockText	= LineText(canonicalText,lineStart,blockEnd);
			BString	label;
			ExtractLabel(blockText,&label);
			int32	id	= (int32)fFoldedBlockText.size();
			fFoldedBlockText.push_back(blockText);
			folded	<< LeadingWhitespace(line) << kIncludedNodeHeader
				<< "[" << id << "] " << label << "\n";
			lineStart	= blockEnd;
		} else {
			folded	<< line;
			lineStart	= lineEnd;
		}
	}
	SetText(folded.String());
}


void MacroTextView::ExpandedText(BString *out)
{
	out->SetTo("");
	BString	fullText(Text());
	int32	lineStart	= 0;
	int32	textLength	= fullText.Length();
	while (lineStart < textLength) {
		int32	nl	= fullText.FindFirst("\n",lineStart);
		int32	lineEnd	= (nl >= 0) ? nl+1 : textLength;
		BString	line	= LineText(fullText,lineStart,lineEnd);
		BString	trimmed	= Trimmed(line);
		int32	foldedId	= -1;
		if (IsFoldedPlaceholder(trimmed,&foldedId)
				&& (foldedId >= 0) && (foldedId < (int32)fFoldedBlockText.size())) {
			*out	<< fFoldedBlockText[foldedId];
		} else {
			*out	<< line;
		}
		lineStart	= lineEnd;
	}
}
