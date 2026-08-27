#include <interface/Point.h>
#include <interface/Screen.h>
#include <interface/View.h>
#include "ColorToolItem.h"

#include "ColorSwatchView.h"
#include "ColorPickerWindow.h"
#include "ToolBar.h"

enum {
	// from the picker window - apply this color
	CTI_SWATCH_CLICKED	= 'ctSC',
	// from the arrow strip - open the full picker
	CTI_OPEN_PICKER		= 'ctOP',
};

static const float kArrowWidth	= 10.0;


// Narrow strip on the right edge of ColorToolItem that opens the full
// picker - the "arrow" half of the split button. A real BButton (not
// just a plain BView with a triangle drawn on it) so it actually looks
// and behaves like a toolbar button - raised bevel, pressed-state
// feedback - all for free from BButton::Draw(), just with a small
// triangle layered on top instead of a text label. A separate child
// view rather than hit-testing inside ColorToolItem::MouseDown() itself,
// same idiom FloatToolItem already uses for its embedded text control:
// Haiku's normal view-hierarchy dispatch routes a click landing on this
// child straight to it, leaving the rest of the outer button's own
// bounds (and its native BButton click-Invoke() behavior) untouched.
class ColorPickerArrowButton : public BButton {
public:
	ColorPickerArrowButton(BRect frame, BMessage *message)
		: BButton(frame, "ColorToolItem::arrow", "", message) {}

	virtual void Draw(BRect updateRect) {
		BButton::Draw(updateRect);
		BRect	b		= Bounds();
		BPoint	center(b.left + b.Width()/2, b.top + b.Height()/2);
		SetDrawingMode(B_OP_OVER);
		SetHighColor(0,0,0,255);
		FillTriangle(
			BPoint(center.x - 3, center.y - 2),
			BPoint(center.x + 3, center.y - 2),
			BPoint(center.x, center.y + 3));
	}
};


ColorToolItem::ColorToolItem(const char *name, rgb_color newValue,BMessage *msg):BaseItem(name),BButton(BRect(0,0,ITEM_WIDTH+kArrowWidth,ITEM_HEIGHT),name,"",msg)
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
	BRect	arrowFrame(ITEM_WIDTH, 0, ITEM_WIDTH+kArrowWidth, ITEM_HEIGHT);
	ColorPickerArrowButton	*arrowButton	=
		new ColorPickerArrowButton(arrowFrame, new BMessage(CTI_OPEN_PICKER));
	arrowButton->SetTarget(this);
	AddChild(arrowButton);
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
	SetDrawingMode(B_OP_OVER);
	BRect	swatchFrame=BRect(0,0,17,17);
	if (Value() != B_CONTROL_ON)
	{
		swatchFrame.OffsetTo(4,4);
	}
	else
	{
		swatchFrame.OffsetTo(5,5);
		swatchFrame.bottom -=2;
		swatchFrame.right -=2;
	}
	SetHighColor(value);
	FillRoundRect(swatchFrame,4,4);
	SetHighColor(tint_color(value,0));
	StrokeLine(BPoint(swatchFrame.left,swatchFrame.top+1),BPoint(swatchFrame.right,swatchFrame.top+1));
	SetHighColor(tint_color(value,0.2));
	StrokeLine(BPoint(swatchFrame.left,swatchFrame.top+2),BPoint(swatchFrame.right,swatchFrame.top+2));
	SetHighColor(tint_color(value,0.4));
	StrokeLine(BPoint(swatchFrame.left,swatchFrame.top+3),BPoint(swatchFrame.right,swatchFrame.top+3));
	SetHighColor(tint_color(value,0.6));
	StrokeLine(BPoint(swatchFrame.left,swatchFrame.top+4),BPoint(swatchFrame.right,swatchFrame.top+4));
	SetHighColor(tint_color(value,0.8));
	StrokeLine(BPoint(swatchFrame.left,swatchFrame.top+5),BPoint(swatchFrame.right,swatchFrame.top+5));

	SetHighColor(ui_color(B_KEYBOARD_NAVIGATION_COLOR));
	StrokeRoundRect(swatchFrame,4,4);
}

void ColorToolItem::SetColor(rgb_color newColor)
{
	// pickerWindow is never pushed this value back - it's always the
	// *source* of a color change (palette swatch or the color/alpha
	// controls inside it), never the target of one, so there's nothing
	// to sync into it here. (An earlier version of this class did call
	// pickerWindow->SetColor() from here, across threads without
	// locking pickerWindow first - a confirmed "Looper must be locked"
	// crash inside BColorControl::SetValue(). Removing the call outright
	// turned out to be the right fix, not adding the missing Lock().)
	value = newColor;
	Invalidate();
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
