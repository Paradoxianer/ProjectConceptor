#include "StringRenderer.h"
#include "ProjectConceptorDefs.h"
#include "PCommandManager.h"

#include <interface/Font.h>
#include <interface/View.h>
#include <interface/GraphicsDefs.h>
#include <interface/Window.h>

#include <support/String.h>




StringRenderer::StringRenderer(SampleEditor *parentEditor,char *forString,BRect stringRect,BMessage *message=NULL)
{
	TRACE();
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
	string = newString;
	if (shortString)	
		delete shortString;
	shortString	= new BString(string);
	editorFont->TruncateString(shortString,B_TRUNCATE_MIDDLE,frame.Width()-2);
}
	
void StringRenderer::SetFrame(BRect newRect)
{
	TRACE();
	frame		= newRect;
	if (shortString)	
		delete shortString;
	shortString	= new BString(string);
	editorFont->TruncateString(shortString,B_TRUNCATE_MIDDLE,frame.Width()-2);
	frame.bottom=frame.top+fontHeight+2;
	startPoint	= new BPoint(frame.left+2,frame.top+fontHeight);

}

void StringRenderer::Draw(BView *drawOn, BRect updateRect)
{	
	drawOn->MovePenTo(startPoint->x,startPoint->y);
	drawOn->DrawString(shortString->String());
}

