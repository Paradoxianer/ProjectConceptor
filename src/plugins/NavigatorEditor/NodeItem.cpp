#include "ProjectConceptorDefs.h"
#include "NodeItem.h"
#include <stdio.h>
#include <interface/InterfaceDefs.h>


NodeItem::NodeItem(BMessage *newNode,
		uint32 level,
		bool expanded)
	: BaseListItem(P_C_CLASS_TYPE,level,expanded)
{
	node			= newNode;
	background		= ui_color(B_CONTROL_BACKGROUND_COLOR);
	backgroundHi	= ui_color(B_CONTROL_HIGHLIGHT_COLOR);
	foreground		= ui_color(B_CONTROL_TEXT_COLOR);
	ValueChanged();
}



/*void NodeItem::Update(BView *newOwner, const BFont *font)
{
	BListItem::Update(newOwner,font);
	font_height	fontHeight;
	font->GetHeight(&fontHeight);
	textLine=fontHeight.ascent;
}*/

void NodeItem::DrawItem(BView *owner, BRect bounds, bool complete)
{
	BRect	newBounds=bounds;
	newBounds.InsetBy(1,1);
	owner->SetDrawingMode(B_OP_OVER);
	owner->MovePenTo(bounds.left+4, bounds.bottom-2);
	rgb_color color;
	if (IsSelected())
		color = backgroundHi;
	else
		color = background;
	owner->SetHighColor(color);
	owner->FillRoundRect(bounds,3,3);
	if (IsEnabled())
		owner->SetHighColor(foreground);	
	else
		owner->SetHighColor(tint_color(foreground,B_DISABLED_LABEL_TINT));
	owner->MovePenTo(newBounds.left+4, newBounds.bottom-2); 
	owner->DrawString(label);
	owner->FillTriangle(BPoint(newBounds.right-2,newBounds.top+newBounds.Height()/2),BPoint(newBounds.right-10,newBounds.top+1),BPoint(newBounds.right-10,newBounds.bottom-1));
	owner->SetHighColor(205,205,205,255);
//	owner->StrokeRoundRect(newBounds,3,3);
	
	owner->SetHighColor(foreground);	
}
void NodeItem::Update(BView *owner, const BFont *font)
{
	BListItem::Update(owner,font);
	SetHeight(Height()+2);
}
void NodeItem::Select(void)
{
}

void NodeItem::Deselect(void)
{
}

void NodeItem::SetExpanded(bool expande)
{
	printf("SetExpanded\n");
};

void NodeItem::ValueChanged(void)
{
	BMessage	*data	= new BMessage();
	if (node->FindMessage("Node::Data",data) == B_OK)
	{
		data->FindString("Name",(const char **)&label);
	}
}
