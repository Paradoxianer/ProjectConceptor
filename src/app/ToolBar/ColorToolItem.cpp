#include <math.h>

#include <interface/Point.h>
#include <interface/Screen.h>
#include "ColorToolItem.h"

#include "ColorSwatchView.h"
#include "ColorPickerWindow.h"
#include "ToolBar.h"

enum {
	// from a palette swatch or the picker window - apply this color
	CTI_SWATCH_CLICKED	= 'ctSC',
	// from the current-color swatch - open the full picker
	CTI_OPEN_PICKER		= 'ctOP',
};

static const float kCurrentSwatchSize	= 24.0;
static const float kPaletteSwatchWidth	= 14.0;
static const float kMargin				= 2.0;
static const float kGap				= 4.0;


// Standard HSV->RGB conversion (hueDegrees in [0,360), saturation/value in
// [0,1]) - written fresh rather than reusing any third-party
// implementation, since this is what generates the algorithmic palette
// (see class comment in the header for why that matters).
static rgb_color
HueToColor(float hueDegrees, float saturation, float value)
{
	float	c			= value * saturation;
	float	hPrime		= hueDegrees / 60.0;
	float	x			= c * (1.0 - fabs(fmod(hPrime, 2.0) - 1.0));
	float	r1 = 0, g1 = 0, b1 = 0;

	if (hPrime < 1)			{ r1 = c; g1 = x; b1 = 0; }
	else if (hPrime < 2)	{ r1 = x; g1 = c; b1 = 0; }
	else if (hPrime < 3)	{ r1 = 0; g1 = c; b1 = x; }
	else if (hPrime < 4)	{ r1 = 0; g1 = x; b1 = c; }
	else if (hPrime < 5)	{ r1 = x; g1 = 0; b1 = c; }
	else					{ r1 = c; g1 = 0; b1 = x; }

	float	m = value - c;
	rgb_color	color;
	color.red	= (uint8)((r1 + m) * 255.0 + 0.5);
	color.green	= (uint8)((g1 + m) * 255.0 + 0.5);
	color.blue	= (uint8)((b1 + m) * 255.0 + 0.5);
	color.alpha	= 255;
	return color;
}


ColorToolItem::ColorToolItem(const char *name, rgb_color newValue,BMessage *msg):BaseItem(name),BButton(BRect(0,0,ITEM_WIDTH,ITEM_HEIGHT),name,"",msg)
{
	Init();
	value	= newValue;
	tName	= name;
	BuildChildren(newValue);
}

ColorToolItem::ColorToolItem(BMessage *archive):BaseItem(""),BButton(archive)
{
	status_t	err;
	ssize_t		size;
	Init();
	err = archive->FindString("ColorToolItem::tName", &tName);
	//**check if the tName ist good??
	void *pointer=&value;
	err = archive->FindData("ColorToolItem::value",B_PATTERN_TYPE,(const void **)&pointer, &size);
	//**Wenn die vorher geladenen variablen nicht da waren, ist das nicht so schlimm
	//err=B_OK;
	err = archive->FindString("ColorToolItem::description",description);
	err = archive->FindString("ColorToolItem::toolTip",toolTip);
	err = archive->FindInt32("ColorToolItem::toolTip",(int32 *)&behavior);
	err = archive->FindInt32("ColorToolItem::toolTip",(int32 *)&state);
	BMessenger tmpMessenger;
	err = archive->FindMessenger("ColorToolItem::Messenger()",&tmpMessenger);	//**nachtragen shadow_offset_by..
	if (err == B_OK)
		SetTarget(tmpMessenger);
	BuildChildren(value);
}
void ColorToolItem::Init(void)
{
	description			= NULL;
	toolTip				= NULL;
	state				= P_M_ITEM_UP;
	pickerWindow		= NULL;
}

void ColorToolItem::BuildChildren(rgb_color initialColor)
{
	float	x	= kMargin;

	currentColorSwatch = new ColorSwatchView("ColorToolItem::currentColor",
		new BMessage(CTI_OPEN_PICKER), this, initialColor,
		kCurrentSwatchSize, kCurrentSwatchSize);
	currentColorSwatch->MoveTo(x, (ITEM_HEIGHT - kCurrentSwatchSize) / 2);
	AddChild(currentColorSwatch);
	x += kCurrentSwatchSize + kGap;

	for (int32 i = 0; i < CTI_PALETTE_SIZE; i++) {
		rgb_color	swatchColor	= HueToColor(
			i * (360.0 / CTI_PALETTE_SIZE), 0.85, 0.95);
		paletteSwatch[i] = new ColorSwatchView("ColorToolItem::palette",
			new BMessage(CTI_SWATCH_CLICKED), this, swatchColor,
			kPaletteSwatchWidth, ITEM_HEIGHT - 4);
		paletteSwatch[i]->MoveTo(x, 2);
		AddChild(paletteSwatch[i]);
		x += kPaletteSwatchWidth;
	}

	ResizeTo(x + kMargin, ITEM_HEIGHT);
}

ColorToolItem::~ColorToolItem(void)
{
	if (description!=NULL) delete description;
	if (toolTip!=NULL) delete toolTip;
}

void ColorToolItem::AttachedToToolBar(ToolBar *tb)
{
	//**check if parentToolBar==NULL or any other error
	BaseItem::AttachedToToolBar(tb);
	parentToolBar->AddChild(this);
}

void ColorToolItem::DetachedFromToolBar(ToolBar *tb)
{
	tb->RemoveChild(this);
	BaseItem::DetachedFromToolBar(tb);
}

status_t ColorToolItem::Archive(BMessage *archive, bool deep) const
{
	status_t err;
	err = BaseItem::Archive(archive,deep);
	err = archive->AddString("class", "ColorToolItem");
	err = archive->AddString("ColorToolItem::tName",tName);
	//**is the NULL - pointer test OK?
	err = archive->AddData("ColorToolItem::value",B_PATTERN_TYPE,&value,sizeof(value));
	if (description!=NULL)
		archive->AddString("ColorToolItem::description",*description);
	if (toolTip!=NULL)
		archive->AddString("ColorToolItem::toolTip",*toolTip);
	err = archive->AddInt32("ColorToolItem::behavior",(int32)behavior);
	err = archive->AddInt32("ColorToolItem::state",(int32)state);

	//**shoud we test  if Message or Messenger==NULL???
	err = archive->AddMessage("ColorToolItem::Message()",Message());
	err = archive->AddMessenger("ColorToolItem::Messenger()",Messenger());
	return err;
}

BArchivable* ColorToolItem::Instantiate(BMessage *archive)
{
	if ( !validate_instantiation(archive, "ColorToolItem") )
		return NULL;
	return new ColorToolItem(archive);
}

void ColorToolItem::Draw(BRect updateRect)
{
	BButton::Draw(updateRect);
}

void ColorToolItem::MouseDown(BPoint point)
{
	BButton::MouseDown(point);
}

void ColorToolItem::MouseUp(BPoint point)
{
	BButton::MouseUp(point);
}

void ColorToolItem::SetColor(rgb_color newColor)
{
	value = newColor;
	currentColorSwatch->SetColor(value);
	if (pickerWindow != NULL)
		pickerWindow->SetColor(value);
	Invoke();
}

void ColorToolItem::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case CTI_SWATCH_CLICKED: {
			rgb_color	newColor;
			if (ColorFromMessage(message,newColor))
				SetColor(newColor);
			break;
		}
		case CTI_OPEN_PICKER: {
			if (pickerWindow == NULL) {
				BPoint	startPoint	= Frame().LeftBottom();
				startPoint			= parentToolBar->ConvertToScreen(startPoint);
				startPoint.y++;

				BRect	frame(startPoint.x,startPoint.y,startPoint.x+1,startPoint.y+1);
				pickerWindow = new ColorPickerWindow(frame,value,
					new BMessage(CTI_SWATCH_CLICKED),this);

				// keep the popup on-screen near the swatch that opened it,
				// same edge-avoidance idea the old inline popup used -
				// Lock() since pickerWindow is a separately-threaded
				// BWindow from here on out
				pickerWindow->Lock();
				BScreen	screen(B_MAIN_SCREEN_ID);
				BRect	screenFrame	= screen.Frame();
				BRect	pickerFrame	= pickerWindow->Frame();
				if (screenFrame.right < pickerFrame.right)
					startPoint.x -= pickerFrame.Width();
				if (screenFrame.bottom < pickerFrame.bottom) {
					startPoint		= Frame().LeftTop();
					startPoint		= parentToolBar->ConvertToScreen(startPoint);
					startPoint.y	-= pickerFrame.Height();
				}
				pickerWindow->MoveTo(startPoint);
				pickerWindow->Unlock();
			}
			pickerWindow->Lock();
			pickerWindow->Show();
			pickerWindow->Activate();
			pickerWindow->Unlock();
			break;
		}
		case PW_CLOSED: {
			pickerWindow = NULL;
			break;
		}
		default:
			BButton::MessageReceived(message);
			break;
	}
}
