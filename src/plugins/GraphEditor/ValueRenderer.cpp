#include "ValueRenderer.h"

#include <Catalog.h>
#include <stdio.h>
#include <string.h>
#include <app/Roster.h>
#include <interface/MenuItem.h>
#include <interface/PopUpMenu.h>
#include <interface/View.h>
#include <locale/DateFormat.h>
#include <locale/NumberFormat.h>
#include <storage/Entry.h>
#include <storage/Path.h>
#include <support/Beep.h>
#include <support/DateTime.h>
#include <support/Url.h>

#include "AttributeText.h"
#include "DatePopup.h"
#include "ProjectConceptorDefs.h"
#include "TextEditorControl.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ValueRenderer"


ValueRenderer::ValueRenderer(GraphEditor *parentEditor,
		const BMessage &forAttribute, BRect valueRect, BMessage *changeMessage)
	: Renderer(parentEditor,NULL),
	change(changeMessage),
	attribute(forAttribute),
	kind(forAttribute.what)
{
	textColor	= editor->Style().text;
	mutedColor	= editor->Style().mutedText;
	font		= *be_plain_font;
	font.SetSize(editor->Style().attributeFontSize);
	font_height	height;
	font.GetHeight(&height);
	ascent		= height.ascent;
	if (kind != P_C_ATTRIBUTE_COLOR)
		AttributeValueText(attribute,&value);
	SetFrame(valueRect);
}


ValueRenderer::~ValueRenderer()
{
	delete change;
}


void ValueRenderer::SetFrame(BRect newRect)
{
	frame			= newRect;
	frame.bottom	= frame.top + ascent + 6;
}


void ValueRenderer::SetColors(rgb_color text, rgb_color muted)
{
	textColor	= text;
	mutedColor	= muted;
}


BString ValueRenderer::DisplayText(void)
{
	switch (kind) {
		case P_C_ATTRIBUTE_FILE: {
			BString	leaf(value);
			int32	slash	= leaf.FindLast('/');
			if ((slash >= 0) && (slash < leaf.Length() - 1))
				leaf.Remove(0,slash + 1);
			return leaf;
		}
		case P_C_ATTRIBUTE_NUMBER: {
			float	number	= 0;
			attribute.FindFloat(P_C_ATTRIBUTE_VALUE,&number);
			BString	text;
			BNumberFormat	format;
			if (format.Format(text,(double)number) != B_OK)
				text	= NumberText(number);
			const char	*unit	= NULL;
			if ((attribute.FindString(P_C_ATTRIBUTE_UNIT,&unit) == B_OK)
					&& (unit[0] != '\0'))
				text << " " << unit;
			return text;
		}
		case P_C_ATTRIBUTE_DATE: {
			int32	year = 0, month = 0, day = 0;
			BDate	date;
			if ((sscanf(value.String(),"%" B_SCNd32 "-%" B_SCNd32 "-%" B_SCNd32,
					&year,&month,&day) == 3) && date.SetDate(year,month,day)) {
				BString		text;
				BDateFormat	format;
				if (format.Format(text,date,B_MEDIUM_DATE_FORMAT) == B_OK)
					return text;
			}
			return value;
		}
		case P_C_ATTRIBUTE_COLOR: {
			BString	text;
			AttributeValueText(attribute,&text);
			return text;
		}
	}
	return value;
}


BString ValueRenderer::Placeholder(void)
{
	switch (kind) {
		case P_C_ATTRIBUTE_LINK:	return B_TRANSLATE("Add address");
		case P_C_ATTRIBUTE_FILE:	return B_TRANSLATE("Choose file…");
		case P_C_ATTRIBUTE_DATE:	return B_TRANSLATE("Choose date…");
		case P_C_ATTRIBUTE_CHOICE:	return B_TRANSLATE("Choose…");
	}
	return BString();
}


BRect ValueRenderer::TextFrame(void)
{
	BString	text	= DisplayText();
	BString	shown(text);
	font.TruncateString(&shown,B_TRUNCATE_MIDDLE,frame.Width() - 4);
	return BRect(frame.left,frame.top,
		frame.left + 2 + font.StringWidth(shown.String()),frame.bottom);
}


void ValueRenderer::Draw(BView *drawOn, BRect updateRect)
{
	drawOn->PushState();
	drawOn->SetFont(&font);
	BPoint	baseline(frame.left + 2,frame.top + ascent + 2);
	float	width	= frame.Width() - 4;
	if (kind == P_C_ATTRIBUTE_COLOR) {
		const rgb_color	*color	= NULL;
		ssize_t			size	= 0;
		BRect	swatch(frame.left + 2,frame.top + 3,frame.left + 2 + ascent*1.6f,
			frame.top + 3 + ascent);
		if (attribute.FindData(P_C_ATTRIBUTE_VALUE,B_RGB_COLOR_TYPE,
				(const void **)&color,&size) == B_OK) {
			drawOn->SetHighColor(*color);
			drawOn->FillRoundRect(swatch,2,2);
		}
		drawOn->SetHighColor(GraphColors::Mix(mutedColor,editor->Style().cardFill,0.4f));
		drawOn->StrokeRoundRect(swatch,2,2);
		baseline.x	= swatch.right + 6;
		width		-= swatch.Width() + 6;
		drawOn->SetHighColor(mutedColor);
	}
	BString	text	= DisplayText();
	bool	empty	= (text.Length() == 0);
	if (empty) {
		text	= Placeholder();
		drawOn->SetHighColor(mutedColor);
	} else if (kind != P_C_ATTRIBUTE_COLOR)
		drawOn->SetHighColor(textColor);
	bool	link	= !empty && ((kind == P_C_ATTRIBUTE_LINK) || (kind == P_C_ATTRIBUTE_FILE));
	if (link)
		drawOn->SetHighColor(editor->Style().accent);
	if (kind == P_C_ATTRIBUTE_CHOICE)
		width	-= ascent;
	font.TruncateString(&text,B_TRUNCATE_MIDDLE,width);
	drawOn->DrawString(text.String(),baseline);
	float	textWidth	= font.StringWidth(text.String());
	if (link)
		drawOn->StrokeLine(baseline + BPoint(0,2),baseline + BPoint(textWidth,2));
	if (kind == P_C_ATTRIBUTE_CHOICE) {
		// a small triangle: there is a list behind it
		float	x	= baseline.x + textWidth + ascent*0.5f;
		float	y	= frame.top + ascent*0.55f + 2;
		float	s	= ascent*0.3f;
		drawOn->SetHighColor(mutedColor);
		drawOn->FillTriangle(BPoint(x - s,y),BPoint(x + s,y),BPoint(x,y + s));
	}
	drawOn->PopState();
}


BCursorID ValueRenderer::CursorAt(BPoint where)
{
	switch (kind) {
		case P_C_ATTRIBUTE_LINK:
		case P_C_ATTRIBUTE_FILE:
			if ((value.Length() > 0) && TextFrame().Contains(where))
				return B_CURSOR_ID_FOLLOW_LINK;
			return B_CURSOR_ID_I_BEAM;
		case P_C_ATTRIBUTE_NUMBER:
			return B_CURSOR_ID_I_BEAM;
	}
	return B_CURSOR_ID_FOLLOW_LINK;
}


void ValueRenderer::MouseDown(BPoint where, int32 buttons, int32 clicks,
	int32 modifiers)
{
	switch (kind) {
		case P_C_ATTRIBUTE_LINK:
		case P_C_ATTRIBUTE_FILE:
			// the text opens, the free space behind it edits
			if ((value.Length() > 0) && TextFrame().Contains(where))
				Open();
			else
				Edit();
			break;
		case P_C_ATTRIBUTE_CHOICE:
			ShowChoices();
			break;
		default:
			Edit();
			break;
	}
}


bool ValueRenderer::HasEdit(void)
{
	return (kind == P_C_ATTRIBUTE_LINK) || (kind == P_C_ATTRIBUTE_FILE)
		|| (kind == P_C_ATTRIBUTE_CHOICE);
}


void ValueRenderer::Edit(void)
{
	switch (kind) {
		case P_C_ATTRIBUTE_LINK:
			OpenTextEditor(value.String(),
				EditorMessage(P_C_ATTRIBUTE_VALUE,B_STRING_TYPE),true);
			break;
		case P_C_ATTRIBUTE_FILE:
			editor->ChooseAttributeFile(*change,false);
			break;
		case P_C_ATTRIBUTE_NUMBER: {
			// GraphEditor splits the text into number and unit
			BMessage	*message	= new BMessage(G_E_ATTRIBUTE_NUMBER);
			message->AddMessage("change",change);
			BMessage	valueContainer;
			message->AddMessage("valueContainer",&valueContainer);
			BString	text;
			AttributeValueText(attribute,&text);
			OpenTextEditor(text.String(),message,true);
			break;
		}
		case P_C_ATTRIBUTE_CHOICE: {
			const char	*choices	= "";
			attribute.FindString(P_C_ATTRIBUTE_CHOICES,&choices);
			OpenTextEditor(choices,EditorMessage(P_C_ATTRIBUTE_CHOICES,B_STRING_TYPE),false);
			break;
		}
		case P_C_ATTRIBUTE_COLOR: {
			const rgb_color	*color	= NULL;
			ssize_t			size	= 0;
			rgb_color		current	= editor->Style().accent;
			if (attribute.FindData(P_C_ATTRIBUTE_VALUE,B_RGB_COLOR_TYPE,
					(const void **)&color,&size) == B_OK)
				current	= *color;
			editor->PickAttributeColor(*change,current,ScreenPoint());
			break;
		}
		case P_C_ATTRIBUTE_DATE: {
			DatePopup	*popup	= new DatePopup(ScreenPoint(),value,*change,
				BMessenger(NULL,editor->BelongTo()));
			popup->Show();
			break;
		}
	}
}


BString ValueRenderer::Address(const BString &link)
{
	BString	address(link);
	address.Trim();
	if ((address.FindFirst(':') < 0) && (address.Length() > 0))
		address.Prepend("https://");
	return address;
}


void ValueRenderer::Open(void)
{
	if (kind == P_C_ATTRIBUTE_LINK) {
		BUrl	url(Address(value).String(),true);
		url.OpenWithPreferredApplication();
		return;
	}
	entry_ref	ref;
	if ((get_ref_for_path(value.String(),&ref) != B_OK)
			|| (be_roster->Launch(&ref) != B_OK))
		beep();
}


void ValueRenderer::ShowChoices(void)
{
	const char	*choices	= "";
	attribute.FindString(P_C_ATTRIBUTE_CHOICES,&choices);
	BPopUpMenu	*menu	= new BPopUpMenu("choices",false,false);
	BString		all(choices);
	int32		start	= 0;
	while (start < all.Length()) {
		int32	end	= all.FindFirst('\n',start);
		if (end < 0)
			end	= all.Length();
		BString	choice(all.String() + start,end - start);
		start	= end + 1;
		if (choice.Length() == 0)
			continue;
		BMenuItem	*item	= new BMenuItem(choice.String(),NULL);
		item->SetMarked(choice == value);
		menu->AddItem(item);
	}
	if (menu->CountItems() > 0)
		menu->AddSeparatorItem();
	BMenuItem	*editItem	= new BMenuItem(B_TRANSLATE("Edit choices…"),NULL);
	menu->AddItem(editItem);
	BMenuItem	*picked	= menu->Go(ScreenPoint(),false,true);
	if (picked == editItem)
		Edit();
	else if ((picked != NULL) && (value != picked->Label())) {
		const char	*label	= picked->Label();
		BMessage	*command	= ChangeOf(*change,P_C_ATTRIBUTE_VALUE,B_STRING_TYPE,
			label,strlen(label) + 1);
		BMessenger(NULL,editor->BelongTo()).SendMessage(command);
		delete command;
	}
	delete menu;
}


BPoint ValueRenderer::ScreenPoint(void)
{
	float	scale	= editor->Scale();
	BPoint	point(frame.left*scale,frame.bottom*scale);
	editor->ConvertToScreen(&point);
	return point;
}


BMessage* ValueRenderer::EditorMessage(const char *field, type_code type)
{
	BMessage	*message	= new BMessage(*change);
	BMessage	valueContainer;
	message->FindMessage("valueContainer",&valueContainer);
	valueContainer.AddString("name",field);
	valueContainer.AddInt32("type",type);
	message->ReplaceMessage("valueContainer",&valueContainer);
	return message;
}


BMessage* ValueRenderer::ChangeOf(const BMessage &change, const char *field,
	type_code type, const void *data, ssize_t size)
{
	BMessage	*message	= new BMessage(change);
	BMessage	valueContainer;
	message->FindMessage("valueContainer",&valueContainer);
	valueContainer.AddString("name",field);
	valueContainer.AddInt32("type",type);
	valueContainer.AddData("newValue",type,data,size);
	message->ReplaceMessage("valueContainer",&valueContainer);
	return message;
}


void ValueRenderer::OpenTextEditor(const char *text, BMessage *message,
	bool singleLine)
{
	float	scale	= editor->Scale();
	BRect	box		= frame;
	if (!singleLine)
		box.bottom	= box.top + 6*(ascent + 6);
	BRect	textRect(box.left*scale,box.top*scale,box.right*scale,box.bottom*scale);
	// the control is not scaled itself, see StringRenderer::MouseDown()
	TextEditorControl	*editer	= new TextEditorControl(textRect,"ValueEditor",
		message,B_FOLLOW_LEFT|B_FOLLOW_TOP);
	editer->SetSingleLine(singleLine);
	if (message->what == G_E_ATTRIBUTE_NUMBER)
		editer->SetTarget(editor);
	else
		editer->SetTarget(editor->BelongTo());
	BFont	zoomed	= font;
	zoomed.SetSize(font.Size()*scale);
	editer->SetFontAndColor(&zoomed);
	editor->AddChild(editer);
	editer->SetText(text);
	editer->MakeFocus(true);
	editer->SelectAll();
	editor->SetActiveTextEditor(editer);
}
