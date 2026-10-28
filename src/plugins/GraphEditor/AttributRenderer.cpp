
#include <interface/Font.h>
#include <interface/View.h>
#include <interface/GraphicsDefs.h>
#include <interface/Window.h>
#include <storage/Resources.h>
#include <support/DataIO.h>
#include <translation/TranslationUtils.h>
#include <translation/TranslatorFormats.h>

#include <support/String.h>
#include <math.h>
#include <Catalog.h>

#include "AttributRenderer.h"
#include "ProjectConceptorDefs.h"
#include "PCommandManager.h"
#include "BoolRenderer.h"
#include "StringRenderer.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "AttributeRenderer"

AttributRenderer::AttributRenderer(GraphEditor *parentEditor,
		BMessage *forAttribut,
		BRect attribRect,
		BMessage *chgMessage,
		BMessage *delMessage)
	:Renderer(parentEditor, NULL)
{
	TRACE();
	changeMessage	= chgMessage;
	deleteMessage	= delMessage;
	editor			= parentEditor;
	frame			= attribRect;
	
	Init();
	SetAttribute(forAttribut);
	
}
void AttributRenderer::Init()
{
	TRACE();
	name		= NULL;
	value		= NULL;
	showDelete	= false;
	columnWidth	= (frame.Width()-DELETER_WIDTH)*0.4f;
	kontextMenu	= new BPopUpMenu("deleter");
	BMenuItem	*delMenu = new BMenuItem(B_TRANSLATE("Delete"),deleteMessage);
	kontextMenu->AddItem(delMenu);
	kontextMenu->SetTargetForItems(editor->BelongTo());
}

void AttributRenderer::SetAttribute(BMessage *newAttribut)
{
	TRACE();
	char		*attribName	= NULL;
	attribut = newAttribut;
	if ( attribut->FindString("Name",(const char **)&attribName) == B_OK )
	{	
		if (name)
			delete name;
		BMessage	*nameChange		= new BMessage(*changeMessage);
		BMessage	*valueContainer	= new BMessage();
		nameChange->FindMessage("valueContainer",valueContainer);
		valueContainer->AddString("name","Name");
		valueContainer->AddInt32("type",B_STRING_TYPE);
		nameChange->ReplaceMessage("valueContainer",valueContainer);
		name = new StringRenderer(editor, attribName, frame,nameChange);
	}
	switch(attribut->what) 
	{
		case B_STRING_TYPE:
		{
			char	*attribValue	= NULL;
			attribut->FindString("Value",(const char **)&attribValue);
			BMessage	*valueChange	= new BMessage(*changeMessage);
			BMessage	*valueContainer	= new BMessage();
			valueChange->FindMessage("valueContainer",valueContainer);
			valueContainer->AddString("name","Value");
			valueContainer->AddInt32("type",B_STRING_TYPE);
			valueChange->ReplaceMessage("valueContainer",valueContainer);
			value	= new StringRenderer(editor,attribValue,frame,valueChange);
			break;
		}
		case B_BOOL_TYPE:
		{
			bool	attribValue	= false;
			attribut->FindBool("Value",&attribValue);
			BMessage	*valueChange	= new BMessage(*changeMessage);
			BMessage	*valueContainer	= new BMessage();
			valueChange->FindMessage("valueContainer",valueContainer);
			valueContainer->AddString("name","Value");
			valueContainer->AddInt32("type",B_BOOL_TYPE);
			valueChange->ReplaceMessage("valueContainer",valueContainer);
			value	= new BoolRenderer(editor,attribValue,frame,valueChange);	
			break;
		}

	}
	const GraphStyle	&style	= editor->Style();
	BFont	rowFont(be_plain_font);
	rowFont.SetSize(style.attributeFontSize);
	StringRenderer	*label	= dynamic_cast<StringRenderer*>(name);
	if (label != NULL) {
		label->SetFont(rowFont);
		label->SetColor(style.mutedText);
	}
	StringRenderer	*text	= dynamic_cast<StringRenderer*>(value);
	if (text != NULL) {
		text->SetFont(rowFont);
		text->SetColor(style.text);
	}
	SetFrame(frame);
}
	
void AttributRenderer::SetFrame(BRect newRect)
{
	TRACE();
	frame			= newRect;
	// the node sets one column for all its rows (#162)
	float	usable	= frame.Width()-DELETER_WIDTH;
	divider			= (columnWidth < usable) ? columnWidth : usable;
	float maxBottom	= frame.bottom;
	if (name)
		name->SetFrame(BRect(frame.left,frame.top,frame.left+divider-1,frame.bottom));
	if (value)
		value->SetFrame(BRect(frame.left+divider+1,frame.top,frame.right-DELETER_WIDTH,frame.bottom));
	if ( (name) && (value) )
	{
		if (name->Frame().bottom>value->Frame().bottom)
			maxBottom = name->Frame().bottom;
		else
			maxBottom = value->Frame().bottom;

	}
	else
	{
		if (name)
			maxBottom	= name->Frame().bottom;
		else if (value)
			maxBottom	= value->Frame().bottom;
	}
	delRect.Set(frame.right-DELETER_WIDTH,frame.top,frame.right,maxBottom);
	delRect.InsetBy(2,2);
	frame.bottom	= maxBottom;
}

void AttributRenderer::SetColumnWidth(float width)
{
	columnWidth	= width;
	SetFrame(frame);
}

float AttributRenderer::LabelWidth(void)
{
	if (name == NULL)
		return 0;
	BFont	rowFont(be_plain_font);
	rowFont.SetSize(editor->Style().attributeFontSize);
	return rowFont.StringWidth(name->GetString()) + 10;
}

void AttributRenderer::MouseDown(BPoint where,
								 int32 buttons, int32 clicks,int32 modifiers)
{
	TRACE();
	if (buttons & B_SECONDARY_MOUSE_BUTTON)
 	{
 		BMenuItem *selected;
 		editor->ConvertToScreen(&where);
		selected = kontextMenu->Go(where); 
		// ### We're not allowed to call it as it is protected.
		// Needs some review.
		// selected->Invoke();
	}
	else
	{
		if ( (name) && (name->Caught(where)) )
			name->MouseDown(where);
		else if ((value) && (value->Caught(where)) )
				value->MouseDown(where);
		else
		{
			//it must be klicked in the deleter
		}
	}
}

void AttributRenderer::MouseUp(BPoint where)
{
	TRACE();
	uint32 modifiers = 0;
	uint32 buttons = 0;
	BMessage *currentMsg = editor->Window()->CurrentMessage();
	currentMsg->FindInt32("buttons", (int32 *)&buttons);
	currentMsg->FindInt32("modifiers", (int32 *)&modifiers);
	if (!(buttons & B_SECONDARY_MOUSE_BUTTON))
	{
		if ( (name) && (name->Caught(where)) )
			name->MouseUp(where);
		else if ((value) && (value->Caught(where)) )
			value->MouseUp(where);
		else 
		{
			if (showDelete && delRect.Contains(where)){
				//it was the deleter
				BMessenger *sender	= new BMessenger(editor->BelongTo());
				sender->SendMessage(deleteMessage); 
			}
		}
	}
}


void AttributRenderer::MoveBy(float dx, float dy)
{
	frame.OffsetBy(dx,dy);
	name->MoveBy(dx,dy);
	value->MoveBy(dx,dy);
	delRect.OffsetBy(dx,dy);
}

void AttributRenderer::Draw(BView *drawOn, BRect updateRect)
{	
	if (name)
		name->Draw(drawOn,updateRect);
	if (value)
		value->Draw(drawOn,updateRect);
	/*if (deleter)
		deleter->Draw(drawOn,updateRect);*/
	if (showDelete) {
		// a quiet cross, not an alarm sign
		const GraphStyle	&style	= editor->Style();
		BPoint	center((delRect.left+delRect.right)/2,(delRect.top+delRect.bottom)/2);
		float	arm		= 3.0f;
		drawOn->PushState();
		drawOn->SetPenSize(1.4);
		drawOn->SetLineMode(B_ROUND_CAP,B_ROUND_JOIN);
		drawOn->SetHighColor(style.mutedText);
		drawOn->StrokeLine(center+BPoint(-arm,-arm),center+BPoint(arm,arm));
		drawOn->StrokeLine(center+BPoint(-arm,arm),center+BPoint(arm,-arm));
		drawOn->PopState();
	}
}



void AttributRenderer::SetTextColors(rgb_color textColor, rgb_color muted)
{
	StringRenderer	*label	= dynamic_cast<StringRenderer*>(name);
	if (label != NULL)
		label->SetColor(muted);
	StringRenderer	*text	= dynamic_cast<StringRenderer*>(value);
	if (text != NULL)
		text->SetColor(textColor);
}


void AttributRenderer::StartEditingName(void)
{
	if (name != NULL)
		name->MouseDown(BPoint(0,0));
}

BCursorID AttributRenderer::CursorAt(BPoint where)
{
	if (showDelete && delRect.Contains(where))
		return B_CURSOR_ID_FOLLOW_LINK;
	if ((name != NULL) && name->Caught(where))
		return B_CURSOR_ID_I_BEAM;
	if ((value != NULL) && value->Caught(where)) {
		return (dynamic_cast<BoolRenderer*>(value) != NULL)
			? B_CURSOR_ID_FOLLOW_LINK : B_CURSOR_ID_I_BEAM;
	}
	return B_CURSOR_ID_SYSTEM_DEFAULT;
}
