#include "BoolRenderer.h"
#include "ProjectConceptorDefs.h"
#include "PCommandManager.h"

#include <interface/Font.h>
#include <interface/View.h>
#include <interface/GraphicsDefs.h>
#include <interface/Window.h>
#include <storage/Resources.h>
#include <support/DataIO.h>
#include <support/String.h>
#include <translation/TranslationUtils.h>
#include <translation/TranslatorFormats.h>

//#include <interface/InterfaceDefs.h>*/

BoolRenderer::BoolRenderer(GraphEditor *parentEditor,
			bool forValue,
			BRect valueRect,
			BMessage *message)
	:Renderer(parentEditor, NULL)
{
	TRACE();
	changeMessage	= message;
	editor			= parentEditor;
	Init();
	SetBool(forValue);
	SetFrame(valueRect);
}

BoolRenderer::~BoolRenderer()
{
}

void BoolRenderer::Init()
{
	TRACE();
	value		= false;
}

void BoolRenderer::SetBool(bool newValue)
{
	TRACE();
	value = newValue;
}
	
void BoolRenderer::SetFrame(BRect newRect)
{
	TRACE();
	frame			= newRect;
	float	size	= editor->Style().attributeFontSize;
	frame.right		= frame.left + size;
	frame.bottom	= frame.top + size + 2;
}

void BoolRenderer::MouseDown(BPoint where, int32 buttons,
	                              int32 clicks,int32 modifiers)
{
	TRACE();

}

void BoolRenderer::MouseUp(BPoint where)
{
	TRACE();	
	value = !value;
	BMessage	*valueContainer	= new BMessage();
	BMessage	sendMessage	= BMessage(*changeMessage);
	sendMessage.FindMessage("valueContainer",valueContainer);
	valueContainer->AddBool("newValue",value);
	sendMessage.ReplaceMessage("valueContainer",valueContainer);
	BMessenger *sender	= new BMessenger(editor->BelongTo());
	sender->SendMessage(&sendMessage);
}

void BoolRenderer::Draw(BView *drawOn, BRect updateRect)
{
	const GraphStyle	&style	= editor->Style();
	BRect	box(frame.left, frame.top + 2, frame.right, frame.bottom);
	drawOn->PushState();
	drawOn->SetPenSize(1.0);
	drawOn->SetHighColor(GraphColors::Mix(style.mutedText, style.cardFill, 0.4f));
	drawOn->StrokeRoundRect(box, 2, 2);
	if (value) {
		// a check mark, drawn like everything else on the card
		drawOn->SetPenSize(1.8);
		drawOn->SetLineMode(B_ROUND_CAP, B_ROUND_JOIN);
		drawOn->SetHighColor(style.accent);
		float	w	= box.Width();
		float	h	= box.Height();
		drawOn->StrokeLine(BPoint(box.left + w * 0.2f, box.top + h * 0.55f),
			BPoint(box.left + w * 0.42f, box.top + h * 0.78f));
		drawOn->StrokeLine(BPoint(box.left + w * 0.42f, box.top + h * 0.78f),
			BPoint(box.left + w * 0.82f, box.top + h * 0.25f));
	}
	drawOn->PopState();
}

