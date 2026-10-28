#include "NoteLayout.h"

#include <ctype.h>
#include <string.h>


NoteFonts::NoteFonts(const BFont &plain)
{
	for (int32 i = 0; i < 4; i++) {
		fonts[i]	= plain;
		uint16	face	= 0;
		if ((i & kNoteBold) != 0)
			face	|= B_BOLD_FACE;
		if ((i & kNoteItalic) != 0)
			face	|= B_ITALIC_FACE;
		if (face != 0)
			fonts[i].SetFace(face);
	}
}


const BFont&
NoteFonts::For(uint32 style) const
{
	return fonts[style & (kNoteBold | kNoteItalic)];
}


static bool StartsWith(const BString &text, int32 at, const char *start)
{
	return strncmp(text.String() + at, start, strlen(start)) == 0;
}


static void AddSpan(std::vector<NoteSpan> *spans, BString *text, uint32 style,
	const BString &url)
{
	if (text->Length() == 0)
		return;
	NoteSpan	span;
	span.text	= *text;
	span.style	= style;
	span.url	= url;
	spans->push_back(span);
	text->SetTo("");
}


static void ParseInline(const BString &text, uint32 style, const BString &url,
	std::vector<NoteSpan> *spans)
{
	BString	plain;
	int32	length	= text.Length();
	int32	i		= 0;
	while (i < length) {
		char	c	= text.ByteAt(i);
		if ((c == '\\') && (i + 1 < length)
				&& (strchr("\\*[]()#-_`", text.ByteAt(i + 1)) != NULL)) {
			plain << text.ByteAt(i + 1);
			i	+= 2;
			continue;
		}
		if (StartsWith(text,i,"**")) {
			int32	end	= text.FindFirst("**",i + 2);
			if (end > i + 2) {
				AddSpan(spans,&plain,style,url);
				ParseInline(BString(text.String() + i + 2,end - i - 2),
					style | kNoteBold,url,spans);
				i	= end + 2;
				continue;
			}
		}
		if ((c == '*') && (i + 1 < length) && (text.ByteAt(i + 1) != ' ')) {
			int32	end	= text.FindFirst('*',i + 1);
			if ((end > i + 1) && (text.ByteAt(end - 1) != ' ')) {
				AddSpan(spans,&plain,style,url);
				ParseInline(BString(text.String() + i + 1,end - i - 1),
					style | kNoteItalic,url,spans);
				i	= end + 1;
				continue;
			}
		}
		if ((c == '[') && (url.Length() == 0)) {
			int32	middle	= text.FindFirst("](",i + 1);
			int32	end		= (middle > i) ? text.FindFirst(')',middle + 2) : -1;
			if ((middle > i + 1) && (end > middle + 2)) {
				AddSpan(spans,&plain,style,url);
				BString	target(text.String() + middle + 2,end - middle - 2);
				ParseInline(BString(text.String() + i + 1,middle - i - 1),
					style | kNoteLink,target,spans);
				i	= end + 1;
				continue;
			}
		}
		if ((url.Length() == 0)
				&& (StartsWith(text,i,"http://") || StartsWith(text,i,"https://"))
				&& ((i == 0) || isspace(text.ByteAt(i - 1)) || (text.ByteAt(i - 1) == '('))) {
			int32	end	= i;
			while ((end < length) && !isspace(text.ByteAt(end)))
				end++;
			// a sentence may end right after the address
			while ((end > i) && (strchr(".,;:!?)",text.ByteAt(end - 1)) != NULL))
				end--;
			AddSpan(spans,&plain,style,url);
			BString	address(text.String() + i,end - i);
			BString	same(address);
			AddSpan(spans,&same,style | kNoteLink,address);
			i	= end;
			continue;
		}
		plain << c;
		i++;
	}
	AddSpan(spans,&plain,style,url);
}


void ParseNoteInline(const BString &text, uint32 style,
	std::vector<NoteSpan> *spans)
{
	ParseInline(text,style,BString(),spans);
}


/** a word with the style it is drawn in */
struct NoteWord {
	BString		text;
	uint32		style;
	BString		url;
	bool		spaceBefore;
};


static void SplitWords(const std::vector<NoteSpan> &spans,
	std::vector<NoteWord> *words)
{
	bool	space	= false;
	for (uint32 s = 0; s < spans.size(); s++) {
		const BString	&text	= spans[s].text;
		BString			word;
		for (int32 i = 0; i <= text.Length(); i++) {
			if ((i == text.Length()) || (text.ByteAt(i) == ' ')) {
				if (word.Length() > 0) {
					NoteWord	w;
					w.text			= word;
					w.style			= spans[s].style;
					w.url			= spans[s].url;
					w.spaceBefore	= space;
					words->push_back(w);
					word.SetTo("");
					space	= false;
				}
				if (i < text.Length())
					space	= true;
			} else
				word << text.ByteAt(i);
		}
	}
}


/** bytes of the longest start of text that fits width, at least one
 * whole UTF-8 character */
static int32 FittingBytes(const BFont &font, const BString &text, float width)
{
	int32	fits	= 0;
	int32	next	= 0;
	while (next < text.Length()) {
		next++;
		while ((next < text.Length()) && ((text.ByteAt(next) & 0xC0) == 0x80))
			next++;
		BString	start(text.String(),next);
		if ((fits > 0) && (font.StringWidth(start.String()) > width))
			break;
		fits	= next;
	}
	return fits;
}


/** merge: the text may join the last run when it has the same style */
static void AddRun(NoteLine *line, const NoteFonts &fonts, const NoteWord &word,
	const BString &text, float x, bool space, bool merge)
{
	if (merge && !line->empty()) {
		NoteRun	&last	= line->back();
		if ((last.style == word.style) && (last.url == word.url)) {
			if (space)
				last.text << " ";
			last.text << text;
			last.width	= fonts.For(last.style).StringWidth(last.text.String());
			return;
		}
	}
	NoteRun	run;
	run.text	= text;
	run.style	= word.style;
	run.url		= word.url;
	run.x		= x;
	run.width	= fonts.For(word.style).StringWidth(text.String());
	line->push_back(run);
}


/** "- ", "* " or "1. " at the start of a line: the marker drawn and how many
 * bytes it takes */
static bool ListMarker(const BString &line, BString *marker, int32 *skip)
{
	if (StartsWith(line,0,"- ") || StartsWith(line,0,"* ")) {
		marker->SetTo("•");
		*skip	= 2;
		return true;
	}
	int32	digits	= 0;
	while ((digits < line.Length()) && isdigit(line.ByteAt(digits)))
		digits++;
	if ((digits > 0) && StartsWith(line,digits,". ")) {
		marker->SetTo(line.String(),digits + 1);
		*skip	= digits + 2;
		return true;
	}
	return false;
}


static void LayoutParagraph(const BString &paragraph, const NoteFonts &fonts,
	float width, std::vector<NoteLine> *lines)
{
	BString	text(paragraph);
	uint32	base	= 0;
	BString	marker;
	int32	skip	= 0;
	NoteLine	line;
	float	indent	= 0;
	if (StartsWith(text,0,"#")) {
		int32	level	= 0;
		while ((level < text.Length()) && (text.ByteAt(level) == '#'))
			level++;
		if (StartsWith(text,level," ")) {
			text.Remove(0,level + 1);
			base	= kNoteBold;
		}
	} else if (ListMarker(text,&marker,&skip)) {
		text.Remove(0,skip);
		NoteWord	bullet;
		bullet.style		= base;
		bullet.spaceBefore	= false;
		AddRun(&line,fonts,bullet,marker,0,false,false);
		indent	= line.back().width + fonts.For(base).StringWidth(" ");
	}
	// the item's text never joins the marker's run
	uint32	keep	= line.size();

	std::vector<NoteSpan>	spans;
	ParseNoteInline(text,base,&spans);
	std::vector<NoteWord>	words;
	SplitWords(spans,&words);

	float	x	= indent;
	for (uint32 w = 0; w < words.size(); w++) {
		const NoteWord	&word	= words[w];
		const BFont		&font	= fonts.For(word.style);
		float	wordWidth	= font.StringWidth(word.text.String());
		bool	space		= word.spaceBefore && (x > indent);
		float	gap			= space ? font.StringWidth(" ") : 0;
		if (x + gap + wordWidth <= width) {
			AddRun(&line,fonts,word,word.text,x + gap,space,line.size() > keep);
			x	+= gap + wordWidth;
			continue;
		}
		if (x > indent) {
			lines->push_back(line);
			line.clear();
			keep	= 0;
			x	= indent;
		}
		// a word longer than the line is broken where it has to
		BString	rest(word.text);
		while (font.StringWidth(rest.String()) > width - indent) {
			int32	fits	= FittingBytes(font,rest,width - indent);
			AddRun(&line,fonts,word,BString(rest.String(),fits),x,false,false);
			lines->push_back(line);
			line.clear();
			keep	= 0;
			rest.Remove(0,fits);
		}
		AddRun(&line,fonts,word,rest,x,false,false);
		x	+= font.StringWidth(rest.String());
	}
	lines->push_back(line);
}


void LayoutNote(const BString &text, const NoteFonts &fonts, float width,
	int32 maxLines, std::vector<NoteLine> *lines, bool *cut)
{
	lines->clear();
	*cut	= false;
	if ((width <= 0) || (text.Length() == 0))
		return;
	int32	start	= 0;
	while ((start <= text.Length()) && ((int32)lines->size() <= maxLines)) {
		int32	end	= text.FindFirst('\n',start);
		if (end < 0)
			end	= text.Length();
		LayoutParagraph(BString(text.String() + start,end - start),fonts,
			width,lines);
		start	= end + 1;
	}
	if ((int32)lines->size() <= maxLines)
		return;
	lines->resize(maxLines);
	*cut	= true;
	NoteLine		&last	= lines->back();
	const BFont		&plain	= fonts.For(0);
	const char		*ellipsis	= "…";
	float			right	= last.empty() ? 0 : last.back().x + last.back().width;
	float			gap		= plain.StringWidth(" ");
	if (right + gap + plain.StringWidth(ellipsis) <= width) {
		NoteRun	run;
		run.text	= ellipsis;
		run.style	= 0;
		run.x		= last.empty() ? 0 : right + gap;
		run.width	= plain.StringWidth(ellipsis);
		last.push_back(run);
	} else {
		// TruncateString() ends the text in an ellipsis itself
		NoteRun	&run	= last.back();
		BString	shorter(run.text);
		shorter << " …";
		fonts.For(run.style).TruncateString(&shorter,B_TRUNCATE_END,width - run.x);
		run.text	= shorter;
		run.width	= fonts.For(run.style).StringWidth(shorter.String());
	}
}
