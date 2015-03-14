#include "StringRenderer.h"
#include "ProjectConceptorDefs.h"
#include "PCommandManager.h"

#include <interface/Font.h>
#include <interface/View.h>
#include <interface/GraphicsDefs.h>
//#include <interface/InterfaceDefs.h>*/
#include <interface/Window.h>

#include <support/String.h>




StringRenderer::StringRenderer(GraphEditor *parentEditor,char *forString,BRect stringRect,BMessage *message)
	: Renderer(parentEditor,message)
{
	TRACE();
	changeMessage	=message;
	editor		= parentEditor;
	shortString	= NULL;
	editorFont	= new BFont();
	Init();
	SetString(forString);
	SetFrame(stringRect);
}
void StringRenderer::Init()
{
	TRACE();
	font_height fHeight;
	editor->GetFont(editorFont);
	editorFont->GetHeight(&fHeight);
	fontHeight = fHeight.ascent+2;
}

void StringRenderer::SetString(char *newString)
{
	TRACE();
	myString = newString;
	if (shortString)	
		delete shortString;
	shortString	= new BString(myString);
	editorFont->TruncateString(shortString,B_TRUNCATE_MIDDLE,frame.Width()-2);
}
	
void StringRenderer::SetFrame(BRect newRect)
{
	TRACE();
	frame		= newRect;
	if (shortString)	
		delete shortString;
	shortString	= new BString(myString);
	editorFont->TruncateString(shortString,B_TRUNCATE_MIDDLE,frame.Width()-2);
	frame.bottom=frame.top+fontHeight+4;
}

void StringRenderer::MouseDown(BPoint where)
{
	TRACE();
	BRect	textRect= frame;
	textRect.InsetBy(2,0);
	textRect.OffsetBy(-2,1);
	TextEditorControl	*editer	= new TextEditorControl(textRect,"StringEditor",changeMessage,B_FOLLOW_LEFT|B_FOLLOW_TOP);
	editer->SetTarget(editor->BelongTo());
	editor->AddChild(editer);
	editer->SetText(myString);
	editer->MakeFocus(true);
	editer->SelectAll();
	
}

void StringRenderer::Draw(BView *drawOn, BRect updateRect)
{	
	drawOn->MovePenTo(frame.left+2,frame.top+fontHeight+1);
	drawOn->DrawString(shortString->String());
}

