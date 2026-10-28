#include "ImageRenderer.h"

#include <Catalog.h>
#include <app/Roster.h>
#include <interface/View.h>
#include <storage/Entry.h>
#include <support/Beep.h>
#include <translation/TranslationUtils.h>

#include <map>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ImageRenderer"

/** Rows are made anew on every change of their node, so the pictures are
 * kept here by path: loaded once, freed with the add-on. A picture that
 * can't be read is remembered as NULL, not tried again on every move. */
typedef std::map<BString,BBitmap*>	BitmapCache;

static BitmapCache	sBitmaps;

static struct CacheCleaner {
	~CacheCleaner() {
		for (BitmapCache::iterator it = sBitmaps.begin(); it != sBitmaps.end(); it++)
			delete it->second;
	}
} sCleaner;


static BBitmap* CachedBitmap(const BString &path)
{
	BitmapCache::iterator	found	= sBitmaps.find(path);
	if (found != sBitmaps.end())
		return found->second;
	BBitmap	*bitmap	= BTranslationUtils::GetBitmap(path.String());
	sBitmaps[path]	= bitmap;
	return bitmap;
}


ImageRenderer::ImageRenderer(GraphEditor *parentEditor, const char *forPath,
		BRect imageRect, BMessage *changeMessage)
	: Renderer(parentEditor,NULL),
	change(changeMessage),
	path(forPath),
	bitmap(NULL)
{
	mutedColor	= editor->Style().mutedText;
	font		= *be_plain_font;
	font.SetSize(editor->Style().attributeFontSize);
	font_height	height;
	font.GetHeight(&height);
	ascent		= height.ascent;
	if (path.Length() > 0)
		bitmap	= CachedBitmap(path);
	SetFrame(imageRect);
}


ImageRenderer::~ImageRenderer()
{
	delete change;
}


void ImageRenderer::SetColors(rgb_color text, rgb_color muted)
{
	mutedColor	= muted;
}


void ImageRenderer::SetFrame(BRect newRect)
{
	frame	= newRect;
	if (bitmap == NULL) {
		frame.bottom	= frame.top + ascent + 6;
		return;
	}
	BRect	bounds	= bitmap->Bounds();
	float	height	= frame.Width() * (bounds.Height() + 1) / (bounds.Width() + 1);
	if (height > kMaxImageHeight)
		height	= kMaxImageHeight;
	frame.bottom	= frame.top + height + 4;
}


void ImageRenderer::Draw(BView *drawOn, BRect updateRect)
{
	drawOn->PushState();
	if (bitmap != NULL) {
		BRect	bounds	= bitmap->Bounds();
		BRect	target(frame.left,frame.top + 2,frame.right,frame.bottom - 2);
		// keep the proportions: as wide as the row, unless that's too high
		float	width	= target.Height() * (bounds.Width() + 1) / (bounds.Height() + 1);
		if (width < target.Width()) {
			target.left		+= (target.Width() - width) / 2;
			target.right	= target.left + width;
		}
		drawOn->SetDrawingMode(B_OP_ALPHA);
		drawOn->DrawBitmap(bitmap,bounds,target,B_FILTER_BITMAP_BILINEAR);
	} else {
		drawOn->SetFont(&font);
		drawOn->SetHighColor(mutedColor);
		BString	text(path.Length() > 0 ? B_TRANSLATE("Image not found")
			: B_TRANSLATE("Choose image…"));
		font.TruncateString(&text,B_TRUNCATE_MIDDLE,frame.Width() - 4);
		drawOn->DrawString(text.String(),BPoint(frame.left + 2,frame.top + ascent + 2));
	}
	drawOn->PopState();
}


void ImageRenderer::MouseDown(BPoint where, int32 buttons, int32 clicks,
	int32 modifiers)
{
	if (bitmap == NULL) {
		Edit();
		return;
	}
	entry_ref	ref;
	if ((get_ref_for_path(path.String(),&ref) != B_OK)
			|| (be_roster->Launch(&ref) != B_OK))
		beep();
}


void ImageRenderer::Edit(void)
{
	editor->ChooseAttributeFile(*change,true);
}
