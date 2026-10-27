#include <stdio.h>
#include <interface/Point.h>
#include <interface/Screen.h>
#include <interface/View.h>
#include <interface/ControlLook.h>
#include "ColorToolItem.h"

#include "ColorSwatchView.h"
#include "ColorPickerWindow.h"
#include "ToolBar.h"

#include <app/Message.h>
#include <storage/Directory.h>
#include <storage/File.h>
#include <storage/FindDirectory.h>
#include <storage/Path.h>
#include <support/Autolock.h>
#include <support/Locker.h>

enum {
	// from the picker window - apply this color
	CTI_SWATCH_CLICKED	= 'ctSC',
	// from the arrow strip - open the full picker
	CTI_OPEN_PICKER		= 'ctOP',
};

static const float kArrowWidth	= 10.0;

// the custom colors used last, shared by every color item and kept
// across sessions
static BLocker		sHistoryLock("ColorToolItem history");
static rgb_color	sHistory[CTI_COLOR_HISTORY_SIZE];
static int32		sHistoryCount	= 0;
static bool			sHistoryLoaded	= false;
static const char	*kHistoryField	= "color";


static status_t
HistoryPath(BPath *path)
{
	status_t	status	= find_directory(B_USER_SETTINGS_DIRECTORY,path);
	if (status == B_OK)
		status	= path->Append("ProjectConceptor/ColorHistory");
	return status;
}


// sHistoryLock must be held
static void
LoadHistory(void)
{
	if (sHistoryLoaded)
		return;
	sHistoryLoaded	= true;
	BPath	path;
	if (HistoryPath(&path) != B_OK)
		return;
	BFile	file(path.Path(),B_READ_ONLY);
	BMessage	archive;
	if ((file.InitCheck() != B_OK) || (archive.Unflatten(&file) != B_OK))
		return;
	int32	packed;
	for (int32 i = 0; (sHistoryCount < CTI_COLOR_HISTORY_SIZE)
			&& (archive.FindInt32(kHistoryField,i,&packed) == B_OK); i++)
		sHistory[sHistoryCount++]	= *(rgb_color *)&packed;
}


// sHistoryLock must be held
static void
SaveHistory(void)
{
	BPath	path;
	if (HistoryPath(&path) != B_OK)
		return;
	BPath	parent;
	if (path.GetParent(&parent) == B_OK)
		create_directory(parent.Path(),0755);
	BMessage	archive;
	for (int32 i = 0; i < sHistoryCount; i++)
		archive.AddInt32(kHistoryField,*(int32 *)&sHistory[i]);
	BFile	file(path.Path(),B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	if ((file.InitCheck() != B_OK) || (archive.Flatten(&file) != B_OK))
		fprintf(stderr,"ColorToolItem: could not save %s\n",path.Path());
}


// Narrow strip on the right edge of ColorToolItem that opens the full
// picker - the "arrow" half of the split button. A plain BView, not a
// BButton: making it a real BButton looked right but broke clicking
// entirely, because ColorToolItem itself is *also* a BButton, and
// ToolBar's own MouseDown/MouseUp dispatch calls straight into the
// outer BButton directly (see ToolBar.cpp) regardless of where exactly
// in its Frame() the click landed - that outer BButton's own native
// mouse-capture then wins the tracking over the inner BButton's, so the
// inner one's Invoke() never fires. A plain BView with its own small
// MouseDown/MouseUp (no BControl-style capture) sidesteps that
// entirely, and drawing it via BControlLook still gets a proper native
// button bevel/pressed-state look - just without being a real BControl.
class ColorPickerArrowView : public BView {
public:
	ColorPickerArrowView(BRect frame, BMessage *message, BHandler *target)
		: BView(frame, "ColorToolItem::arrow", B_FOLLOW_NONE, B_WILL_DRAW),
		fMessage(message), fTarget(target), fTrackingStart(-1.0, -1.0),
		fPressed(false)
	{
		SetViewColor(B_TRANSPARENT_32_BIT);
	}

	virtual ~ColorPickerArrowView() { delete fMessage; }

	virtual void Draw(BRect updateRect) {
		BRect	frame	= Bounds();
		rgb_color	base	= ui_color(B_PANEL_BACKGROUND_COLOR);
		uint32	flags	= fPressed ? BControlLook::B_ACTIVATED : 0;
		be_control_look->DrawButtonFrame(this, frame, updateRect, base, base, flags);
		be_control_look->DrawButtonBackground(this, frame, updateRect, base, flags);

		BPoint	center(Bounds().left + Bounds().Width()/2,
			Bounds().top + Bounds().Height()/2);
		SetHighColor(0,0,0,255);
		FillTriangle(
			BPoint(center.x - 3, center.y - 2),
			BPoint(center.x + 3, center.y - 2),
			BPoint(center.x, center.y + 3));
	}

	virtual void MouseDown(BPoint where) {
		if (Bounds().Contains(where)) {
			fTrackingStart	= where;
			fPressed		= true;
			Invalidate();
		}
	}

	virtual void MouseUp(BPoint where) {
		bool	invoke	= Bounds().Contains(where) && Bounds().Contains(fTrackingStart);
		fPressed = false;
		Invalidate();
		if (invoke && (fMessage != NULL) && (fTarget != NULL)) {
			BLooper *looper = fTarget->Looper();
			if (looper != NULL)
				looper->PostMessage(fMessage, fTarget);
		}
		fTrackingStart.x = -1.0;
		fTrackingStart.y = -1.0;
	}

private:
	BMessage	*fMessage;
	BHandler	*fTarget;
	BPoint		fTrackingStart;
	bool		fPressed;
};


ColorToolItem::ColorToolItem(const char *name, rgb_color newValue,BMessage *msg,BMessage *previewMsg):BaseItem(name),BButton(BRect(0,0,ITEM_WIDTH+kArrowWidth,ITEM_HEIGHT),name,"",msg)
{
	Init();
	value			= newValue;
	tName			= name;
	previewMessage	= previewMsg;
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
	targetCount		= 0;
	currentTarget	= 0;
	description			= NULL;
	toolTip				= NULL;
	state				= P_M_ITEM_UP;
	pickerWindow		= NULL;
	previewMessage		= NULL;
	hasPreview			= false;
}

void ColorToolItem::BuildChildren(rgb_color initialColor)
{
	BRect	arrowFrame(ITEM_WIDTH, 0, ITEM_WIDTH+kArrowWidth, ITEM_HEIGHT);
	AddChild(new ColorPickerArrowView(arrowFrame, new BMessage(CTI_OPEN_PICKER), this));
}

ColorToolItem::~ColorToolItem(void)
{
	if (description!=NULL) delete description;
	if (toolTip!=NULL) delete toolTip;
	if (previewMessage!=NULL) delete previewMessage;
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
	rgb_color	drawColor	= hasPreview ? previewValue : value;
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
	// flat: the color and a rim in its own darker shade
	SetDrawingMode(B_OP_ALPHA);
	SetHighColor(drawColor);
	FillRoundRect(swatchFrame,4,4);
	rgb_color	rim	= tint_color(drawColor,B_DARKEN_2_TINT);
	rim.alpha	= 255;
	SetHighColor(rim);
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
	value		= newColor;
	hasPreview	= false;
	if (targetCount > 0)
		targets[currentTarget].color	= newColor;
	RecordColorInHistory(newColor);
	Invalidate();
	Invoke();
}

void ColorToolItem::ClearTargets(void)
{
	keepTarget		= TargetField();
	targetCount		= 0;
	currentTarget	= 0;
}

void ColorToolItem::AddTarget(const char *label, const char *field,
	rgb_color color, rgb_color standard, bool light)
{
	if (targetCount >= PW_MAX_TARGETS)
		return;
	targets[targetCount].label		= label;
	targets[targetCount].color		= color;
	targets[targetCount].standard	= standard;
	targets[targetCount].light		= light;
	targetFields[targetCount]		= field;
	if (keepTarget == field)
		currentTarget	= targetCount;
	targetCount++;
	ShowColor(targets[currentTarget].color);
}

const char* ColorToolItem::TargetField(void) const
{
	if (targetCount == 0)
		return "FillColor";
	return targetFields[currentTarget].String();
}

rgb_color ColorToolItem::ColorFor(const char *field) const
{
	for (int32 i = 0; i < targetCount; i++) {
		if (targetFields[i] == field)
			return targets[i].color;
	}
	return value;
}

bool ColorToolItem::FindColor(const char *field, rgb_color *color) const
{
	for (int32 i = 0; i < targetCount; i++) {
		if (targetFields[i] == field) {
			*color	= targets[i].color;
			return true;
		}
	}
	return false;
}

void ColorToolItem::ShowColor(rgb_color newColor)
{
	value		= newColor;
	hasPreview	= false;
	Invalidate();
}

void ColorToolItem::PreviewColor(rgb_color newColor)
{
	// Live preview only - value/GetColor() (the committed color
	// GraphEditor's G_E_COLOR_CHANGED handler reads) is untouched here.
	// Mirrors ClassRenderer::MouseMoved() previewing a drag purely at
	// the renderer level - see docs/notes.md.
	hasPreview		= true;
	previewValue	= newColor;
	Invalidate();
	if (previewMessage != NULL) {
		BMessage	preview(*previewMessage);
		AddColorToMessage(&preview,newColor);
		preview.AddString("field",TargetField());
		Invoke(&preview);
	}
}

void ColorToolItem::CancelPreview(void)
{
	// Escape discards whatever was being previewed - no commit. The
	// renderers GraphEditor updated directly while previewing (see
	// PreviewColor() above) have no other way to learn this session
	// ended without a real commit, so they're told explicitly to fall
	// back to their real color again.
	hasPreview	= false;
	Invalidate();
	if (previewMessage != NULL) {
		BMessage	cancel(*previewMessage);
		cancel.AddBool("cancel",true);
		Invoke(&cancel);
	}
}

void ColorToolItem::RecordColorInHistory(rgb_color color)
{
	// palette and standard colors are one click away already
	if (IsPaletteColor(color)
			|| ((targetCount > 0) && (targets[currentTarget].standard == color)))
		return;
	BAutolock	lock(sHistoryLock);
	LoadHistory();
	// MRU: re-using a color moves it to the front instead of adding it twice
	int32	existing	= -1;
	for (int32 i = 0; i < sHistoryCount; i++) {
		if (sHistory[i] == color) {
			existing	= i;
			break;
		}
	}
	int32	last	= (existing >= 0) ? existing
		: ((sHistoryCount < CTI_COLOR_HISTORY_SIZE) ? sHistoryCount : CTI_COLOR_HISTORY_SIZE - 1);
	for (int32 i = last; i > 0; i--)
		sHistory[i]	= sHistory[i-1];
	sHistory[0]	= color;
	if ((existing < 0) && (sHistoryCount < CTI_COLOR_HISTORY_SIZE))
		sHistoryCount++;
	SaveHistory();
}

void ColorToolItem::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case CTI_SWATCH_CLICKED: {
			// despite the name (kept from before the redesign), this is
			// now only ever a live-preview report from inside the open
			// picker (palette swatch, BColorControl, AlphaSlider) - the
			// real commit happens once, in PW_CLOSED below
			rgb_color	newColor;
			if (ColorFromMessage(message,newColor))
				PreviewColor(newColor);
			break;
		}
		case CTI_OPEN_PICKER: {
			if (pickerWindow == NULL) {
				BPoint	startPoint	= Frame().LeftBottom();
				startPoint			= parentToolBar->ConvertToScreen(startPoint);
				startPoint.y++;

				BRect	frame(startPoint.x,startPoint.y,startPoint.x+1,startPoint.y+1);
				ColorPickerTarget	only[1];
				only[0].color		= value;
				only[0].standard	= value;
				only[0].light		= false;
				rgb_color	history[CTI_COLOR_HISTORY_SIZE];
				int32		historyCount;
				{
					BAutolock	lock(sHistoryLock);
					LoadHistory();
					historyCount	= sHistoryCount;
					for (int32 i = 0; i < historyCount; i++)
						history[i]	= sHistory[i];
				}
				pickerWindow = new ColorPickerWindow(frame,
					new BMessage(CTI_SWATCH_CLICKED),this,
					targetCount > 0 ? targets : only,
					targetCount > 0 ? targetCount : 1,currentTarget,
					history,historyCount);

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
		case PW_TARGET_CHANGED: {
			// what was picked for the old target goes to it first
			if (hasPreview)
				SetColor(previewValue);
			int32	index;
			if ((message->FindInt32("target",&index) == B_OK)
					&& (index >= 0) && (index < targetCount)) {
				currentTarget	= index;
				ShowColor(targets[index].color);
			}
			break;
		}
		case PW_CLOSED: {
			pickerWindow = NULL;
			// the one real, undo-worthy commit - whatever was last
			// previewed while the picker was open. If nothing was ever
			// previewed (opened and closed without touching anything),
			// hasPreview is still false and this is correctly a no-op.
			bool	cancel	= false;
			message->FindBool("cancel",&cancel);
			if (hasPreview) {
				if (cancel)
					CancelPreview();
				else
					SetColor(previewValue);
			}
			break;
		}
		default:
			BButton::MessageReceived(message);
			break;
	}
}
