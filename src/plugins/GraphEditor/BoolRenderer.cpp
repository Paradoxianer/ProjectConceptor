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
void BoolRenderer::Init()
{
	TRACE();
	image_info	*info 	= new image_info;
	size_t		size;
	// look up the plugininfos
	get_image_info(editor->PluginID(),info);
	// init the ressource for the plugin files
	BResources *res=new BResources(new BFile((const char*)info->name,B_READ_ONLY));
	// load the addBool icon
	const void *data=res->LoadResource((type_code)'PNG ',"addBool",&size);
	if (data)
		//translate the icon because it was png but we ne a bmp 
		selected	= BTranslationUtils::GetBitmap(new BMemoryIO(data,size));
	data=res->LoadResource((type_code)'PNG ',"unselectedBool",&size);
	if (data)
		unselected	= BTranslationUtils::GetBitmap(new BMemoryIO(data,size));
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
	if (selected)
	{
		frame.bottom	= frame.top + selected->Bounds().Height();
		frame.right		= frame.left + selected->Bounds().Width();
	}
	else
	{
		frame.bottom	= frame.top+10;
		frame.right		= frame.left+10;
	}
}

void BoolRenderer::MouseDown(BPoint where, int32 buttons,
	                              int32 clicks,int32 modifiers)
{
	TRACE();

}

void BoolRenderer::MouseUp(BPoint where)
{
	TRACE();	
	value=!value;
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
	if (value)	
		drawOn->DrawBitmapAsync(selected,frame);
	else
		drawOn->DrawBitmapAsync(unselected,frame);
}

