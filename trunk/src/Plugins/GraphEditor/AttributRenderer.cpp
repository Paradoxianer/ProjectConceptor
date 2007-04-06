
#include <interface/Font.h>
#include <interface/View.h>
#include <interface/GraphicsDefs.h>
//#include <interface/InterfaceDefs.h>*/
#include <interface/Window.h>

#include <support/String.h>
#include <math.h>

#include "AttributRenderer.h"
#include "ProjectConceptorDefs.h"
#include "PCommandManager.h"
#include "BoolRenderer.h"

AttributRenderer::AttributRenderer(GraphEditor *parentEditor,BMessage *forAttribut,BRect attribRect,BMessage *chgMessage=NULL, BMessage *delMessage=NULL):Renderer(parentEditor,NULL,NULL)
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
	deleter		= NULL;
	kontextMenu	= new BPopUpMenu("deleter");
	BMenuItem	*deleter = new BMenuItem(_T("Delete"),deleteMessage);
	kontextMenu->AddItem(deleter);
	status_t err = kontextMenu->SetTargetForItems(editor->BelongTo());
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
	SetFrame(frame);
}
	
void AttributRenderer::SetFrame(BRect newRect)
{
	TRACE();
	frame			= newRect;
	divider			= frame.Width()/3;
	float maxBottom	= frame.bottom;
	if (name)
		name->SetFrame(BRect(frame.left,frame.top,frame.left+divider-1,frame.bottom));
	if (value)
		value->SetFrame(BRect(frame.left+divider,frame.top,frame.right-DELETER_WIDTH,frame.bottom));
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
	if (deleter)
		deleter->SetFrame(BRect(frame.right-DELETER_WIDTH,frame.top,frame.right,maxBottom));
	frame.bottom	= maxBottom;	
}

void AttributRenderer::MouseDown(BPoint where)
{
	TRACE();
	uint32 modifiers = 0;
	uint32 buttons = 0;
	BMessage *currentMsg = editor->Window()->CurrentMessage();
	currentMsg->FindInt32("buttons", (int32 *)&buttons);
	currentMsg->FindInt32("modifiers", (int32 *)&modifiers);
	if (buttons & B_SECONDARY_MOUSE_BUTTON)
 	{
 		BMenuItem *selected;
 		BMessage *copy;
 		editor->ConvertToScreen(&where);
		selected = kontextMenu->Go(where); 
		selected->Invoke();
	}
	else
	{
		if ( (name) && (name->Caught(where)) )
			name->MouseDown(where);
		else if ((deleter) && (deleter->Caught(where)) )
			deleter->MouseDown(where);
		else
		{
			if (value)
				value->MouseDown(where);
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
		else if ((deleter) && (deleter->Caught(where)) )
			deleter->MouseUp(where);
		else
		{
			if (value)
				value->MouseUp(where);
		}
	}
}


void AttributRenderer::MoveBy(float dx, float dy)
{
	frame.OffsetBy(dx,dy);
	name->MoveBy(dx,dy);
	value->MoveBy(dx,dy);
}

void AttributRenderer::Draw(BView *drawOn, BRect updateRect)
{	
	if (name)
		name->Draw(drawOn,updateRect);
	if (value)
		value->Draw(drawOn,updateRect);
	if (deleter)
		deleter->Draw(drawOn,updateRect);
	/*rgb_color stored = drawOn->HighColor();
	drawOn->SetHighColor(60,50,50,255);
	drawOn->StrokeLine(BPoint(frame.left+divider,frame.top),BPoint(frame.left+divider,frame.bottom));
	drawOn->SetHighColor(stored);*/
}

