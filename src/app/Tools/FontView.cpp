
#include "FontView.h"
#include <string.h>

FontView::FontView(void)
 : BListView("font_view")
{
	SetSelectionMessage(new BMessage());
	//first find the normal font then use it to construck the 
	//fonts for the fontitems
	filterString = "";
}


FontView::~FontView(void)
{

}


void
FontView::AttachedToWindow(void)
{
	RescanForFonts();
}


void
FontView::SetFontSize(uint16 size)
{
}


void
FontView::SelectFont(font_family family, font_style style, float size)
{
}


void
FontView::SelectFont(BFont &font)
{
}


void
FontView::MessageReceived(BMessage *msg)
{
	switch (msg->what) {
/*		case '': {
			break;
		}*/
		default: {
			BView::MessageReceived(msg);
			break;
		}
	}
}

void
FontView::RescanForFonts(void)
{	
	DeleteAll();
	int32 numFamilies = count_font_families();
	for ( int32 i = 0; i < numFamilies; i++ )
	{
		font_family localfamily;
		if ( get_font_family ( i, &localfamily ) == B_OK )
		{
			if (strcasestr(localfamily,filterString) != NULL)
				AddItem(new FontItem(localfamily));
		}
	}
}

void FontView::SetFilter(char *newFilter)
{
	filterString=newFilter;
	int32		count	= CountItems();
	RescanForFonts();
}

void
FontView::DeleteAll(void)
{	
	int32		numFamilies	= CountItems();
	/*BListItem	*tmpItem	= NULL;
	for ( int32 i = 0; i < numFamilies; i++ )
	{
		tmpItem=ItemAt(0);
		if (tmpItem != NULL)
		{
			RemoveItem(tmpItem);
			delete tmpItem;
		}
	}*/
	MakeEmpty();
}

FontItem::FontItem(font_family myFamily): BListItem()
{
	memcpy(family,myFamily,B_FONT_FAMILY_LENGTH + 1);
	font	= BFont();
	font.SetFamilyAndFace(family,B_REGULAR_FACE);	
}

void  FontItem::DrawItem(BView *owner, BRect frame, bool complete)
{
	owner->SetDrawingMode(B_OP_OVER);
	rgb_color color;
	if (IsSelected())
		color = ui_color(B_LIST_SELECTED_BACKGROUND_COLOR);
	else
		color = ui_color(B_LIST_BACKGROUND_COLOR);
	
	owner->SetHighColor(color);
	owner->FillRect(frame);
	if (IsSelected()) 
		color = ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR);
	else
		color = ui_color(B_LIST_ITEM_TEXT_COLOR);
	owner->SetHighColor(color);
	owner->MovePenTo(frame.left+4, frame.bottom-2);
   	owner->DrawString(family);
	//TODO figure out the perfekt place for the divierowner->MovePenTo(frame.left+owner->GetDivPos(), frame.bottom-2);
	owner->MovePenTo(frame.left+140, frame.bottom-2);
	//save the BView font
	
	BFont tmpFont;
	owner->GetFont(&tmpFont);
	owner->SetFont(&font);
	//draw the PreviewString
	owner->DrawString(PREVIEW_STR);
	//and restore the default BView Font
	owner->SetFont(&tmpFont);
}


float FontItem::Height() const
{
	float dHeight=BListItem::Height();
	font_height fHeight;
	//geht the height of this font
	font.GetHeight(&fHeight);
	//if the hight of this font is smaller than the default us the default height
	if (fHeight.leading < dHeight)
		return dHeight;
	else
		return fHeight.leading;
}


float FontItem::Width() const
{
	//TODO .. use view Font for calculation of the family Length
	float familyLength		= font.StringWidth(family);
	float descriptionLenght = font.StringWidth(PREVIEW_STR);
	return (familyLength+descriptionLenght);
}
