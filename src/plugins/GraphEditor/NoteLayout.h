#ifndef NOTE_LAYOUT_H
#define NOTE_LAYOUT_H

#include <interface/Font.h>
#include <support/String.h>

#include <vector>

/** Text of a note attribute (#157) laid out in lines: a small part of
 * Markdown is shown, the stored text stays plain.
 *
 * Block: "# " heading (bold), "- " / "* " bullet, "1. " numbered item;
 * the item's further lines hang under its text.
 * Inline: **bold**, *italic*, [text](url), bare http:// and https:// links,
 * a backslash keeps the next character as it is. */

enum {
	kNoteBold		= 1,
	kNoteItalic		= 2,
	kNoteLink		= 4
};

/** a piece of text in one style, before it is put into lines */
struct NoteSpan {
	BString		text;
	uint32		style;
	BString		url;
};

/** a piece of one line, x from the left of the note */
struct NoteRun {
	BString		text;
	uint32		style;
	BString		url;
	float		x;
	float		width;
};

typedef std::vector<NoteRun>	NoteLine;

/** the fonts a note draws with, one per bold/italic combination */
class NoteFonts {
public:
						NoteFonts(const BFont &plain);
	const	BFont&		For(uint32 style) const;
private:
			BFont		fonts[4];
};

/** inline markup of one line of text into spans */
void	ParseNoteInline(const BString &text, uint32 style,
			std::vector<NoteSpan> *spans);

/** the whole text in lines of at most width; *cut tells whether there
 * was more than maxLines - then the last line ends in an ellipsis */
void	LayoutNote(const BString &text, const NoteFonts &fonts, float width,
			int32 maxLines, std::vector<NoteLine> *lines, bool *cut);

#endif
