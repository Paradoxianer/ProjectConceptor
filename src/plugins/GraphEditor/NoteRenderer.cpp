#include "NoteRenderer.h"

#include <Catalog.h>
#include <math.h>
#include <interface/View.h>
#include <support/Url.h>

#include "TextEditorControl.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "NoteRenderer"


NoteRenderer::NoteRenderer(GraphEditor *parentEditor,const char *forText,
		BRect noteRect,BMessage *message)
	: Renderer(parentEditor,NULL),
	fonts(*be_plain_font)
{
	changeMessage	= message;
	text			= (forText != NULL) ? forText : "";
	textColor		= editor->Style().text;
	mutedColor		= editor->Style().mutedText;
	cut				= false;
	BFont	rowFont(be_plain_font);
	rowFont.SetSize(editor->Style().attributeFontSize);
	frame			= noteRect;
	SetFont(rowFont);
}


NoteRenderer::~NoteRenderer()
{
	delete changeMessage;
}


void NoteRenderer::SetFont(const BFont &newFont)
{
	font	= newFont;
	fonts	= NoteFonts(font);
	font_height	height;
	font.GetHeight(&height);
	ascent		= height.ascent;
	lineHeight	= ceilf(height.ascent + height.descent + height.leading) + 1;
	SetFrame(frame);
}


void NoteRenderer::SetColors(rgb_color newText, rgb_color muted)
{
	textColor	= newText;
	mutedColor	= muted;
}


void NoteRenderer::SetFrame(BRect newRect)
{
	frame	= newRect;
	LayoutNote(text,fonts,frame.Width() - 2,kMaxNoteLines,&lines,&cut);
	int32	shown	= lines.empty() ? 1 : (int32)lines.size();
	frame.bottom	= frame.top + shown*lineHeight + 2;
}


void NoteRenderer::Draw(BView *drawOn, BRect updateRect)
{
	drawOn->PushState();
	if (lines.empty()) {
		drawOn->SetFont(&font);
		drawOn->SetHighColor(mutedColor);
		drawOn->DrawString(B_TRANSLATE("Empty note"),
			BPoint(frame.left + 2,frame.top + ascent + 1));
	}
	rgb_color	linkColor	= editor->Style().accent;
	for (uint32 i = 0; i < lines.size(); i++) {
		float	baseline	= frame.top + i*lineHeight + ascent + 1;
		for (uint32 r = 0; r < lines[i].size(); r++) {
			const NoteRun	&run	= lines[i][r];
			bool	link	= (run.style & kNoteLink) != 0;
			drawOn->SetFont(&fonts.For(run.style));
			drawOn->SetHighColor(link ? linkColor : textColor);
			BPoint	at(frame.left + 2 + run.x,baseline);
			drawOn->DrawString(run.text.String(),at);
			if (link)
				drawOn->StrokeLine(at + BPoint(0,2),at + BPoint(run.width,2));
		}
	}
	drawOn->PopState();
}


BString NoteRenderer::LinkAt(BPoint where)
{
	int32	line	= (int32)((where.y - frame.top - 1) / lineHeight);
	if ((line < 0) || (line >= (int32)lines.size()))
		return BString();
	float	x	= where.x - frame.left - 2;
	for (uint32 r = 0; r < lines[line].size(); r++) {
		const NoteRun	&run	= lines[line][r];
		if (((run.style & kNoteLink) != 0) && (x >= run.x) && (x <= run.x + run.width))
			return run.url;
	}
	return BString();
}


BCursorID NoteRenderer::CursorAt(BPoint where)
{
	return (LinkAt(where).Length() > 0) ? B_CURSOR_ID_FOLLOW_LINK : B_CURSOR_ID_I_BEAM;
}


void NoteRenderer::MouseDown(BPoint where, int32 buttons, int32 clicks,
	int32 modifiers)
{
	BString	address	= LinkAt(where);
	if (address.Length() == 0) {
		StartEditing();
		return;
	}
	BUrl	url(address.String(),true);
	url.OpenWithPreferredApplication();
}


void NoteRenderer::StartEditing(void)
{
	float	scale	= editor->Scale();
	// room for a few more lines than the note has
	BRect	box		= frame;
	float	minimum	= 5*lineHeight;
	if (box.Height() < minimum)
		box.bottom	= box.top + minimum;
	BRect	textRect(box.left*scale,box.top*scale,box.right*scale,box.bottom*scale);
	// the control is not scaled itself, see StringRenderer::MouseDown()
	TextEditorControl	*editer	= new TextEditorControl(textRect,"NoteEditor",
		new BMessage(*changeMessage),B_FOLLOW_LEFT|B_FOLLOW_TOP);
	editer->SetSingleLine(false);
	editer->SetWordWrap(true);
	editer->SetTarget(editor->BelongTo());
	BFont	zoomed	= font;
	zoomed.SetSize(font.Size()*scale);
	editer->SetFontAndColor(&zoomed);
	editor->AddChild(editer);
	editer->SetText(text.String());
	editer->MakeFocus(true);
	editer->Select(text.Length(),text.Length());
	editor->SetActiveTextEditor(editer);
}
