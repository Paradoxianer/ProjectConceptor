#include "ShadowCache.h"

#include <Bitmap.h>
#include <View.h>

#include <math.h>
#include <string.h>

#include <vector>

#include "NodeShape.h"

// sizes change a lot while resizing; beyond this, start over
static const size_t	kMaxShadows	= 256;


ShadowCache::ShadowCache(void)
{
}


ShadowCache::~ShadowCache(void)
{
	Clear();
}


void ShadowCache::Clear(void)
{
	std::map<BString, BBitmap*>::iterator	iter;
	for (iter = fShadows.begin(); iter != fShadows.end(); iter++)
		delete iter->second;
	fShadows.clear();
}


void ShadowCache::BoxBlur(uint8 *mask, int32 width, int32 height,
	int32 radius)
{
	if ((radius < 1) || (width < 1) || (height < 1))
		return;
	std::vector<uint8>	line(width > height ? width : height);
	const int32			span	= 2 * radius + 1;
	// three box passes come close to a gaussian
	for (int32 pass = 0; pass < 3; pass++) {
		for (int32 y = 0; y < height; y++) {
			uint8	*row	= mask + y * width;
			int32	sum		= 0;
			for (int32 x = -radius; x <= radius; x++)
				sum += (x >= 0 && x < width) ? row[x] : 0;
			for (int32 x = 0; x < width; x++) {
				line[x]	= (uint8)(sum / span);
				int32	out	= x - radius;
				int32	in	= x + radius + 1;
				sum	-= (out >= 0) ? row[out] : 0;
				sum	+= (in < width) ? row[in] : 0;
			}
			memcpy(row, &line[0], width);
		}
		for (int32 x = 0; x < width; x++) {
			int32	sum	= 0;
			for (int32 y = -radius; y <= radius; y++)
				sum += (y >= 0 && y < height) ? mask[y * width + x] : 0;
			for (int32 y = 0; y < height; y++) {
				line[y]	= (uint8)(sum / span);
				int32	out	= y - radius;
				int32	in	= y + radius + 1;
				sum	-= (out >= 0) ? mask[out * width + x] : 0;
				sum	+= (in < height) ? mask[in * width + x] : 0;
			}
			for (int32 y = 0; y < height; y++)
				mask[y * width + x]	= line[y];
		}
	}
}


BBitmap* ShadowCache::Build(const NodeShape &shape, BRect frame,
	float cornerRadius, rgb_color color, int32 margin)
{
	int32	width	= (int32)ceilf(frame.Width()) + 1 + 2 * margin;
	int32	height	= (int32)ceilf(frame.Height()) + 1 + 2 * margin;
	BRect	bounds(0, 0, width - 1, height - 1);
	BRect	silhouette(margin, margin, margin + frame.Width(), margin + frame.Height());

	// the silhouette goes black on white: read back from the color, it
	// doesn't depend on how the app_server writes alpha into a bitmap
	BBitmap	canvas(bounds, B_RGBA32, true);
	if (canvas.InitCheck() != B_OK)
		return NULL;
	BView	*view	= new BView(bounds, "shadow", B_FOLLOW_NONE, B_WILL_DRAW);
	canvas.AddChild(view);
	canvas.Lock();
	memset(canvas.Bits(), 0xff, canvas.BitsLength());
	view->SetHighColor(0, 0, 0, 255);
	if (shape.HasPath()) {
		NodeShape	placed(shape);
		placed.Layout(silhouette);
		placed.Fill(view);
	} else
		view->FillRoundRect(silhouette, cornerRadius, cornerRadius);
	view->Sync();
	canvas.Unlock();

	std::vector<uint8>	mask(width * height);
	const uint8	*bits	= (const uint8*)canvas.Bits();
	int32		bpr		= canvas.BytesPerRow();
	for (int32 y = 0; y < height; y++) {
		for (int32 x = 0; x < width; x++)
			mask[y * width + x]	= 255 - bits[y * bpr + x * 4];
	}
	BoxBlur(&mask[0], width, height, margin / 2);

	BBitmap	*shadow	= new BBitmap(bounds, B_RGBA32);
	uint8	*out	= (uint8*)shadow->Bits();
	int32	outBpr	= shadow->BytesPerRow();
	for (int32 y = 0; y < height; y++) {
		for (int32 x = 0; x < width; x++) {
			uint8	*pixel	= out + y * outBpr + x * 4;
			pixel[0]	= color.blue;
			pixel[1]	= color.green;
			pixel[2]	= color.red;
			pixel[3]	= (uint8)(mask[y * width + x] * color.alpha / 255);
		}
	}
	return shadow;
}


void ShadowCache::Draw(BView *view, const NodeShape &shape,
	const char *shapeName, BRect frame, float cornerRadius, rgb_color color,
	float blur, float offsetY)
{
	int32	margin	= (int32)ceilf(blur);
	BString	key;
	key.SetToFormat("%s %d %d %d %d %d", shapeName,
		(int)ceilf(frame.Width()), (int)ceilf(frame.Height()),
		(int)ceilf(cornerRadius), (int)margin, (int)color.alpha);
	std::map<BString, BBitmap*>::iterator	found	= fShadows.find(key);
	BBitmap	*shadow	= NULL;
	if (found != fShadows.end())
		shadow	= found->second;
	else {
		if (fShadows.size() >= kMaxShadows)
			Clear();
		shadow	= Build(shape, frame, cornerRadius, color, margin);
		if (shadow == NULL)
			return;
		fShadows[key]	= shadow;
	}
	BRect	target	= shadow->Bounds();
	target.OffsetTo(frame.left - margin, frame.top - margin + offsetY);
	view->PushState();
	view->SetDrawingMode(B_OP_ALPHA);
	view->SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
	view->DrawBitmap(shadow, shadow->Bounds(), target);
	view->PopState();
}
