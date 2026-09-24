#include "MacroTextView.h"

#include <interface/Font.h>
#include <interface/InterfaceDefs.h>
#include <interface/Window.h>

#include <string.h>
#include <set>
#include <utility>
#include <vector>

#include "MacroEditor.h"
#include "MacroText.h"
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

// prepended to every folded placeholder line, purely cosmetic (stripped
// before matching in IsFoldedPlaceholder()) - a plain "~included_node[0]
// ..." line looked exactly like any other line of DSL, easy to select and
// delete without noticing it was standing in for a whole node/connection's
// data, not just itself.
// plain ASCII, not the more decorative U+25B8 "▸" disclosure triangle first
// tried here - that rendered as a fallback dash in the (italic) chip font,
// unreadable as a triangle. ">>" reads unambiguously as "expand me" without
// depending on font glyph coverage.
static const char *kFoldGlyph = ">> ";

static const rgb_color kFoldColor = {80,100,150,255};


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


// the block's own reference id, from its "this=N" field - Indexer::
// IndexNode()/IndexConnection() always add this (raw literal there too,
// there's no ProjectConceptorDefs.h constant for it), so it's present on
// every legitimately machine-generated "~included_node" block. -1 if
// missing (hand-edited away, or malformed input).
static int32 ExtractThisId(const BString &blockText)
{
	int32	lineStart	= 0;
	int32	textLength	= blockText.Length();
	while (lineStart < textLength) {
		int32	nl	= blockText.FindFirst("\n",lineStart);
		int32	lineEnd	= (nl >= 0) ? nl+1 : textLength;
		BString	trimmed	= Trimmed(LineText(blockText,lineStart,lineEnd));
		if (trimmed.StartsWith("this=")) {
			BString	idText;
			trimmed.CopyInto(idText,5,trimmed.Length()-5);
			bool	allDigits	= idText.Length() > 0;
			for (int32 i=0;i<idText.Length();i++)
				if ((idText[i] < '0') || (idText[i] > '9'))
					allDigits	= false;
			if (allDigits)
				return atol(idText.String());
		}
		lineStart	= lineEnd;
	}
	return -1;
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


// true, with *outKey set to the fFoldedBlockText map key, for a line whose
// trimmed content is the fold glyph (if present) followed by
// "~included_node[@<digits>]" (a block with a real "this=N" reference id -
// see ExtractThisId()) or "~included_node[#<digits>]" (the synthetic
// fallback for a block missing one) and optionally " <label>" - the
// placeholder form SetMacroText()/ToggleFoldAtLine() build. Never matches a
// real, expanded "~included_node" header (no "[" there at all). "@N" maps
// to key N itself; "#N" maps to key -N, matching how ToggleFoldAtLine()/
// SetMacroText() insert into fFoldedBlockText.
static bool IsFoldedPlaceholder(const BString &trimmed, int32 *outKey)
{
	BString	rest(trimmed);
	if (rest.StartsWith(kFoldGlyph))
		rest.Remove(0,strlen(kFoldGlyph));
	BString	prefix(kIncludedNodeHeader);
	prefix	<< "[";
	if (!rest.StartsWith(prefix))
		return false;
	int32	closeBracket	= rest.FindFirst("]",prefix.Length());
	if (closeBracket < 0)
		return false;
	BString	idText;
	rest.CopyInto(idText,prefix.Length(),closeBracket-prefix.Length());
	if (idText.Length() < 2)
		return false;
	char	marker	= idText[0];
	if ((marker != '@') && (marker != '#'))
		return false;
	BString	digits;
	idText.CopyInto(digits,1,idText.Length()-1);
	for (int32 i=0;i<digits.Length();i++)
		if ((digits[i] < '0') || (digits[i] > '9'))
			return false;
	int32	value	= atol(digits.String());
	*outKey	= (marker == '@') ? value : -value;
	return true;
}


static bool IsExpandedHeader(const BString &trimmed)
{
	return trimmed == BString(kIncludedNodeHeader);
}


// true for a bare command name - "Select", "Group", the macro's own
// top-level command, a subPCommand nested under another one - as opposed to
// a "fieldName=value" data line or a "~fieldName" block header. All three
// can sit at the very same indentation depth (a subPCommand and its
// parent's own data fields nest one level under the parent alike), which is
// exactly why indentation alone doesn't tell them apart - this does, purely
// from the line's own shape: a command name never contains "=" and never
// starts with "~" (or the fold glyph that stands in for one).
static bool IsCommandLine(const BString &trimmed)
{
	if (trimmed.Length() == 0)
		return false;
	if (trimmed.StartsWith(kFoldGlyph))
		return false;
	if (trimmed[0] == '~')
		return false;
	if (trimmed.FindFirst('=') >= 0)
		return false;
	return true;
}


MacroTextView::MacroTextView(BRect frame, const char *name, BRect textRect,
	uint32 resizingMode, uint32 flags)
	:BTextView(frame,name,textRect,resizingMode,flags)
{
	fEditor	= NULL;
	fOriginalBlockCount	= 0;
	fNextSyntheticId	= 1;
	GetFontAndColor(0,&fDefaultFont,&fDefaultColor);
}


// the placeholder text for a block keyed at `key` in fFoldedBlockText - the
// same "@N"/"#N" formatting IsFoldedPlaceholder() parses back.
static BString FoldKeyText(int32 key)
{
	BString	text;
	if (key >= 0)
		text << "@" << key;
	else
		text << "#" << -key;
	return text;
}


// italic + a muted blue-gray - visibly distinct from ordinary DSL text, so
// a folded chip reads as "there's more here, this isn't just a line" rather
// than looking like any other line that's just as easy to select and
// delete without noticing what it was standing in for.
void MacroTextView::StyleAsFoldedChip(int32 start, int32 end)
{
	BFont	font(fDefaultFont);
	font.SetFace(B_ITALIC_FACE);
	SetFontAndColor(start,end,&font,B_FONT_ALL,&kFoldColor);
}


void MacroTextView::ClearFoldStyle(int32 start, int32 end)
{
	SetFontAndColor(start,end,&fDefaultFont,B_FONT_ALL,&fDefaultColor);
}


void MacroTextView::KeyDown(const char *bytes, int32 numBytes)
{
	// Tab/Shift+Tab indent instead of moving focus - the DSL is indentation
	// (2 spaces per level) and a literal tab character would not parse.
	if ((numBytes == 1) && (bytes[0] == B_TAB)) {
		int32	modifiers	= 0;
		if ((Window() != NULL) && (Window()->CurrentMessage() != NULL))
			Window()->CurrentMessage()->FindInt32("modifiers",&modifiers);
		ShiftSelectedLines((modifiers & B_SHIFT_KEY) ? -1 : 1);
		return;
	}
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
		fEditor->ApplyEdits(false);
}


void MacroTextView::Select(int32 startOffset, int32 endOffset)
{
	BTextView::Select(startOffset,endOffset);
	if (fEditor == NULL)
		return;
	int32		line	= 1;
	int32		column	= 1;
	const char	*text	= Text();
	for (int32 i = 0; (i < startOffset) && (text[i] != '\0'); i++) {
		if (text[i] == '\n') {
			line++;
			column	= 1;
		} else
			column++;
	}
	fEditor->UpdateCursorPosition(line,column);
}


bool MacroTextView::AcceptsDrop(const BMessage *message)
{
	if (message->HasBool(kCommandSnippetDragMarker))
		return false;
	return BTextView::AcceptsDrop(message);
}


void MacroTextView::MessageReceived(BMessage *message)
{
	if (message->WasDropped() && message->HasBool(kCommandSnippetDragMarker)) {
		// BTextView first: it does the drop's own cleanup (drag caret,
		// auto-scroll runner) before checking AcceptsDrop() - which says no
		// for this message, so it inserts nothing itself.
		BTextView::MessageReceived(message);
		const void	*data	= NULL;
		ssize_t		length	= 0;
		if ((message->FindData("text/plain",B_MIME_TYPE,&data,&length) == B_OK) && (length > 0)) {
			BPoint	dropOffset;
			BPoint	dropPoint	= message->DropPoint(&dropOffset);
			ConvertFromScreen(&dropPoint);
			DropSnippet(dropPoint,(const char*)data,(int32)length);
		}
		return;
	}
	BTextView::MessageReceived(message);
}


void MacroTextView::ShiftSelectedLines(int32 levels)
{
	int32	selStart	= 0;
	int32	selEnd		= 0;
	GetSelection(&selStart,&selEnd);
	int32	firstLine	= LineAt(selStart);
	int32	lastLine	= LineAt(selEnd);
	// a selection ending exactly at a line's start doesn't include that line
	if ((selEnd > selStart) && (OffsetAt(lastLine) == selEnd) && (lastLine > firstLine))
		lastLine--;
	BString	all(Text());
	int32	delta		= 0;
	// bottom to top, so earlier offsets stay valid while editing
	for (int32 line = lastLine; line >= firstLine; line--) {
		int32	start	= OffsetAt(line);
		int32	end		= start;
		while ((end < all.Length()) && (all[end] != '\n'))
			end++;
		BString	lineText(LineText(all,start,end));
		if (Trimmed(lineText).Length() == 0)
			continue;
		int32	change	= IndentChange(lineText,levels);
		if (change > 0) {
			BString	spaces;
			for (int32 i = 0; i < change; i++)
				spaces << " ";
			Insert(start,spaces.String(),spaces.Length());
			all.Insert(spaces,start);
		} else if (change < 0) {
			Delete(start,start-change);
			all.Remove(start,-change);
		}
		if (line == firstLine)
			delta	= change;
	}
	int32		newStart	= OffsetAt(firstLine);
	if (selEnd > selStart) {
		const char	*text	= Text();
		int32		newEnd	= OffsetAt(lastLine);
		while ((text[newEnd] != '\0') && (text[newEnd] != '\n'))
			newEnd++;
		Select(newStart,newEnd);
	} else {
		int32	caret	= selStart+delta;
		Select((caret > newStart) ? caret : newStart,(caret > newStart) ? caret : newStart);
	}
}


void MacroTextView::DropSnippet(BPoint where, const char *text, int32 length)
{
	// an Insert prototype is always node 1 - a second one would clash with
	// the first, so it takes the next free id
	BString	snippet(text,length);
	BString	existing;
	ExpandedText(&existing);
	int32	nextId	= HighestReferencedId(existing)+1;
	RenumberInsertPrototype(&snippet,(nextId > 1) ? nextId : 1);
	BString	all(Text());
	int32	line		= (all.Length() > 0) ? LineAt(where) : 0;
	bool	lowerHalf	= (all.Length() > 0)
		&& (where.y > PointAt(OffsetAt(line)).y + LineHeight(line)/2);
	int32	insertOffset	= 0;
	BString	insertText;

	// Lines are selected and the drop lands on them: wrap them - the new
	// command goes in front, the selected commands become its subcommands
	// (indented one level deeper), in place. Dropping anywhere else with
	// something selected is just a normal drop.
	int32	selStart	= 0;
	int32	selEnd		= 0;
	GetSelection(&selStart,&selEnd);
	if ((all.Length() > 0) && (selEnd > selStart)) {
		int32	firstLine	= LineAt(selStart);
		int32	lastLine	= LineAt(selEnd);
		if ((OffsetAt(lastLine) == selEnd) && (lastLine > firstLine))
			lastLine--;
		if ((line >= firstLine) && (line <= lastLine)) {
			SnippetInsertion(all,firstLine,false,snippet,&insertOffset,&insertText);
			ShiftSelectedLines(1);
			Insert(insertOffset,insertText.String(),insertText.Length());
			ClearFoldStyle(insertOffset,insertOffset+insertText.Length());
			StyleCommandLines();
			Select(insertOffset,insertOffset+insertText.Length());
			ScrollToSelection();
			return;
		}
	}

	SnippetInsertion(all,line,lowerHalf,snippet,&insertOffset,&insertText);
	Insert(insertOffset,insertText.String(),insertText.Length());
	// the text takes the style of whatever it landed next to - a chip's
	// italics would otherwise spread over the dropped commands
	ClearFoldStyle(insertOffset,insertOffset+insertText.Length());
	StyleCommandLines();
	Select(insertOffset,insertOffset+insertText.Length());
	ScrollToSelection();
}


// blockText was captured at the depth its chip stood at then; Tab/Shift+Tab
// and wrapping in a command move the chip line but not the folded text, so
// its lines are shifted to where the chip stands now (a block at the wrong
// depth parses as a different command tree)
static BString ReindentBlock(const BString &blockText, const BString &targetIndent)
{
	int32	baseIndent	= LeadingWhitespace(blockText).Length();
	int32	target		= targetIndent.Length();
	if (baseIndent == target)
		return blockText;
	BString	result;
	int32	lineStart	= 0;
	int32	textLength	= blockText.Length();
	while (lineStart < textLength) {
		int32	nl		= blockText.FindFirst("\n",lineStart);
		int32	lineEnd	= (nl >= 0) ? nl+1 : textLength;
		BString	line	= LineText(blockText,lineStart,lineEnd);
		if (Trimmed(line).Length() == 0) {
			result	<< line;
		} else if (target > baseIndent) {
			result.Append(' ',target-baseIndent);
			result	<< line;
		} else {
			int32	remove	= 0;
			while ((remove < baseIndent-target) && (remove < line.Length()) && (line[remove] == ' '))
				remove++;
			line.Remove(0,remove);
			result	<< line;
		}
		lineStart	= lineEnd;
	}
	return result;
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
	AdoptEditedChipIds();
	BString	fullText(Text());
	BString	line	= LineText(fullText,lineStart,lineEnd);
	BString	trimmed	= Trimmed(line);
	int32	foldedKey	= 0;

	if (IsFoldedPlaceholder(trimmed,&foldedKey)) {
		std::map<int32,BString>::iterator	found	= fFoldedBlockText.find(foldedKey);
		if (found == fFoldedBlockText.end())
			return;
		ApplyChipLabelEdit(&found->second,trimmed);
		BString	blockText	= ReindentBlock(found->second,LeadingWhitespace(line));
		// the block's text is the view's own from here on - a stale entry
		// would make the key look taken when the block is folded again
		fFoldedBlockText.erase(found);
		Delete(lineStart,lineEnd);
		Insert(lineStart,blockText.String(),blockText.Length());
		ClearFoldStyle(lineStart,lineStart+blockText.Length());
		return;
	}

	if (IsExpandedHeader(trimmed)) {
		int32	blockEnd	= FindBlockEnd(fullText,lineStart,lineEnd);
		BString	blockText	= LineText(fullText,lineStart,blockEnd);
		BString	label;
		ExtractLabel(blockText,&label);
		int32	key		= AllocateFoldKey(ExtractThisId(blockText));
		fFoldedBlockText[key]	= blockText;
		BString	placeholder	= LeadingWhitespace(line);
		placeholder	<< kFoldGlyph << kIncludedNodeHeader << "[" << FoldKeyText(key)
			<< "] " << label << "\n";
		Delete(lineStart,blockEnd);
		Insert(lineStart,placeholder.String(),placeholder.Length());
		StyleAsFoldedChip(lineStart,lineStart+placeholder.Length());
		return;
	}
}


static void SetThisId(BString *blockText, int32 newId)
{
	int32	lineStart	= 0;
	int32	textLength	= blockText->Length();
	while (lineStart < textLength) {
		int32	nl		= blockText->FindFirst("\n",lineStart);
		int32	lineEnd	= (nl >= 0) ? nl+1 : textLength;
		BString	line	= LineText(*blockText,lineStart,lineEnd);
		if (Trimmed(line).StartsWith("this=")) {
			BString	replacement	= LeadingWhitespace(line);
			replacement	<< "this=" << newId << "\n";
			blockText->Remove(lineStart,lineEnd-lineStart);
			blockText->Insert(replacement,lineStart);
			return;
		}
		lineStart	= lineEnd;
	}
}


void MacroTextView::AdoptEditedChipIds(void)
{
	// the "[@N]" in a chip is text like any other and gets edited (to match
	// a hand-changed "node=@N", say) - its folded block is then no longer
	// found under that id. With exactly one such chip and one block no chip
	// points at anymore, the edit is the rename of that block.
	BString	fullText(Text());
	std::set<int32>	shownKeys;
	std::vector<int32>	unresolved;
	int32	lineStart	= 0;
	int32	textLength	= fullText.Length();
	while (lineStart < textLength) {
		int32	nl		= fullText.FindFirst("\n",lineStart);
		int32	lineEnd	= (nl >= 0) ? nl+1 : textLength;
		int32	key		= 0;
		if (IsFoldedPlaceholder(Trimmed(LineText(fullText,lineStart,lineEnd)),&key)) {
			shownKeys.insert(key);
			if (fFoldedBlockText.find(key) == fFoldedBlockText.end())
				unresolved.push_back(key);
		}
		lineStart	= lineEnd;
	}
	if (unresolved.size() != 1)
		return;
	std::vector<int32>	orphans;
	for (std::map<int32,BString>::iterator entry = fFoldedBlockText.begin();
			entry != fFoldedBlockText.end(); ++entry)
		if (shownKeys.find(entry->first) == shownKeys.end())
			orphans.push_back(entry->first);
	if (orphans.size() != 1)
		return;
	int32	newKey	= unresolved[0];
	BString	block	= fFoldedBlockText[orphans[0]];
	if (newKey >= 0)
		SetThisId(&block,newKey);
	fFoldedBlockText.erase(orphans[0]);
	fFoldedBlockText[newKey]	= block;
}


int32 MacroTextView::AllocateFoldKey(int32 thisId)
{
	// two blocks can carry the same this=N (a dropped Insert prototype is
	// always this=1): the second one must not overwrite the first's entry
	if ((thisId >= 0) && (fFoldedBlockText.find(thisId) == fFoldedBlockText.end()))
		return thisId;
	return -(fNextSyntheticId++);
}


void MacroTextView::ApplyChipLabelEdit(BString *blockText, const BString &placeholderTrimmed)
{
	// a chip's label is shown as "Name" - typing into it renames the node
	// in its folded block; anything but a plain quoted label is left alone
	int32	closeBracket	= placeholderTrimmed.FindFirst("] ");
	if (closeBracket < 0)
		return;
	BString	label;
	placeholderTrimmed.CopyInto(label,closeBracket+2,placeholderTrimmed.Length()-closeBracket-2);
	label.Trim();
	if ((label.Length() < 2) || (label[0] != '"') || (label[label.Length()-1] != '"'))
		return;
	BString	newName;
	label.CopyInto(newName,1,label.Length()-2);
	if ((newName.FindFirst("\"") >= 0) || (newName.FindFirst("\\") >= 0))
		return;

	BString	currentLabel;
	ExtractLabel(*blockText,&currentLabel);
	if (currentLabel == label)
		return;
	BString	nameNeedle(P_C_NODE_NAME);
	nameNeedle	<< "=\"";
	int32	nameAt	= blockText->FindFirst(nameNeedle);
	if (nameAt < 0)
		return;
	int32	valueStart	= nameAt+nameNeedle.Length();
	int32	valueEnd	= blockText->FindFirst("\"",valueStart);
	if (valueEnd < valueStart)
		return;
	blockText->Remove(valueStart,valueEnd-valueStart);
	blockText->Insert(newName,valueStart);
}


void MacroTextView::SetMacroText(const BString &canonicalText)
{
	fFoldedBlockText.clear();
	fNextSyntheticId	= 1;
	BString	folded;
	std::vector<std::pair<int32,int32> >	chipRanges;
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
			int32	key		= AllocateFoldKey(ExtractThisId(blockText));
			fFoldedBlockText[key]	= blockText;
			int32	chipStart	= folded.Length();
			folded	<< LeadingWhitespace(line) << kFoldGlyph << kIncludedNodeHeader
				<< "[" << FoldKeyText(key) << "] " << label << "\n";
			chipRanges.push_back(std::make_pair(chipStart,folded.Length()));
			lineStart	= blockEnd;
		} else {
			folded	<< line;
			lineStart	= lineEnd;
		}
	}
	SetText(folded.String());
	for (size_t i=0;i<chipRanges.size();i++)
		StyleAsFoldedChip(chipRanges[i].first,chipRanges[i].second);
	StyleCommandLines();
	fOriginalBlockCount	= (int32)fFoldedBlockText.size();
	// BTextView::SetText() doesn't go through the public Select() this
	// view overrides - without this, the line/column status would keep
	// showing wherever the cursor happened to be in whatever macro was
	// open before, not "Line 1, Col 1" for the one actually now on screen.
	Select(0,0);
}


void MacroTextView::StyleCommandLines(void)
{
	BString	fullText(Text());
	int32	lineStart	= 0;
	int32	textLength	= fullText.Length();
	BFont	boldFont(fDefaultFont);
	boldFont.SetFace(B_BOLD_FACE);
	while (lineStart < textLength) {
		int32	nl	= fullText.FindFirst("\n",lineStart);
		int32	lineEnd	= (nl >= 0) ? nl+1 : textLength;
		BString	trimmed	= Trimmed(LineText(fullText,lineStart,lineEnd));
		if (IsCommandLine(trimmed)) {
			int32	nameStart	= lineStart+LeadingWhitespace(
				LineText(fullText,lineStart,lineEnd)).Length();
			int32	nameEnd		= (nl >= 0) ? nl : lineEnd;
			SetFontAndColor(nameStart,nameEnd,&boldFont,B_FONT_ALL,&fDefaultColor);
		}
		lineStart	= lineEnd;
	}
}


int32 MacroTextView::CurrentBlockCount(void)
{
	BString	fullText(Text());
	int32	count		= 0;
	int32	lineStart	= 0;
	int32	textLength	= fullText.Length();
	while (lineStart < textLength) {
		int32	nl	= fullText.FindFirst("\n",lineStart);
		int32	lineEnd	= (nl >= 0) ? nl+1 : textLength;
		BString	trimmed	= Trimmed(LineText(fullText,lineStart,lineEnd));
		int32	foldedId	= -1;
		if (IsExpandedHeader(trimmed) || IsFoldedPlaceholder(trimmed,&foldedId))
			count++;
		lineStart	= lineEnd;
	}
	return count;
}


bool MacroTextView::LostFoldedBlocks(BString *outWarning)
{
	int32	current	= CurrentBlockCount();
	if (current >= fOriginalBlockCount)
		return false;
	outWarning->SetToFormat(
		"Warning: %ld of %ld node/connection block(s) are gone - "
		"applied anyway. If that wasn't intentional, undo (Ctrl+Z) before "
		"editing further.",
		(long)(fOriginalBlockCount-current),(long)fOriginalBlockCount);
	return true;
}


void MacroTextView::ExpandedText(BString *out)
{
	AdoptEditedChipIds();
	out->SetTo("");
	BString	fullText(Text());
	int32	lineStart	= 0;
	int32	textLength	= fullText.Length();
	while (lineStart < textLength) {
		int32	nl	= fullText.FindFirst("\n",lineStart);
		int32	lineEnd	= (nl >= 0) ? nl+1 : textLength;
		BString	line	= LineText(fullText,lineStart,lineEnd);
		BString	trimmed	= Trimmed(line);
		int32	foldedKey	= 0;
		std::map<int32,BString>::iterator	found	= fFoldedBlockText.end();
		if (IsFoldedPlaceholder(trimmed,&foldedKey))
			found	= fFoldedBlockText.find(foldedKey);
		if (found != fFoldedBlockText.end()) {
			ApplyChipLabelEdit(&found->second,trimmed);
			*out	<< ReindentBlock(found->second,LeadingWhitespace(line));
		} else {
			*out	<< line;
		}
		lineStart	= lineEnd;
	}
}


// number of lines `blockText` spans - every block SetMacroText()/
// ToggleFoldAtLine() ever captures ends in "\n" (see LineText() - it always
// includes the line's own trailing newline), so a plain '\n' count is exact.
static int32 LineCountOf(const BString &blockText)
{
	int32	count	= 0;
	int32	at		= 0;
	while ((at = blockText.FindFirst("\n",at)) >= 0) {
		count++;
		at++;
	}
	return count;
}


bool MacroTextView::RevealCanonicalLine(int32 canonicalLineNo)
{
	if (canonicalLineNo < 1)
		return false;
	// bounded by the number of folded blocks - at most that many can ever
	// need expanding, one per pass (expanding drops the map entry, hence
	// captured up front), before the target line is in plain text.
	const size_t	maxPasses	= fFoldedBlockText.size();
	for (size_t guard = 0; guard <= maxPasses; guard++) {
		BString	fullText(Text());
		int32	canonicalCounter	= 1;
		int32	lineStart			= 0;
		int32	textLength			= fullText.Length();
		bool	restart				= false;
		while (lineStart < textLength) {
			int32	nl		= fullText.FindFirst("\n",lineStart);
			int32	lineEnd	= (nl >= 0) ? nl+1 : textLength;
			BString	trimmed	= Trimmed(LineText(fullText,lineStart,lineEnd));
			int32	foldedKey	= 0;
			std::map<int32,BString>::const_iterator	found	= fFoldedBlockText.end();
			if (IsFoldedPlaceholder(trimmed,&foldedKey))
				found	= fFoldedBlockText.find(foldedKey);
			if (found != fFoldedBlockText.end()) {
				int32	blockLines	= LineCountOf(found->second);
				if (canonicalLineNo < canonicalCounter+blockLines) {
					// the target line is inside this still-folded block - expand
					// it and restart the walk over the now-changed text, rather
					// than try to patch up offsets in place.
					ToggleFoldAtLine(lineStart,lineEnd);
					restart	= true;
					break;
				}
				canonicalCounter	+= blockLines;
			} else {
				if (canonicalLineNo == canonicalCounter) {
					int32	selEnd	= (nl >= 0) ? nl : lineEnd;
					Select(lineStart,selEnd);
					ScrollToSelection();
					return true;
				}
				canonicalCounter++;
			}
			lineStart	= lineEnd;
		}
		if (!restart)
			return false;	// out of range - no block expansion will change that
	}
	return false;
}
