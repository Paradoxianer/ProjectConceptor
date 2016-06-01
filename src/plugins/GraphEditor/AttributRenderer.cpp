
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
	image_info	*info 	= new image_info;
	size_t		size;
	delBitmap	= NULL;
	kontextMenu	= new BPopUpMenu("deleter");
	BMenuItem	*delMenu = new BMenuItem(B_TRANSLATE("Delete"),deleteMessage);
	kontextMenu->AddItem(delMenu);
	kontextMenu->SetTargetForItems(editor->BelongTo());

	// look up the plugininfos
	get_image_info(editor->PluginID(),info);
	// init the ressource for the plugin files
	BResources *res=new BResources(new BFile((const char*)info->name,B_READ_ONLY));
	// load the addBool icon
	const void *data=res->LoadResource((type_code)'PNG ',"delete",&size);
	if (data)
		//translate the icon because it was png but we ne a bmp 
		delBitmap	= BTranslationUtils::GetBitmap(new BMemoryIO(data,size));
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
	divider			= (frame.Width()-DELETER_WIDTH)/2;
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
			if (delRect.Contains(where)){
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
	if (delBitmap)
		drawOn->DrawBitmapAsync(delBitmap,delRect);
	rgb_color stored = drawOn->HighColor();
	drawOn->SetHighColor(60,50,50,255);
	drawOn->StrokeRect(frame);
	drawOn->StrokeLine(BPoint(frame.left+divider,frame.top),BPoint(frame.left+divider,frame.bottom));
	drawOn->SetHighColor(stored);
}

