#include "ColorItem.h"
#include <stdio.h>
#include <interface/InterfaceDefs.h>

#ifdef B_ZETA_VERSION_BETA
	#include <locale/Locale.h>
#else
	#define _T(a) a
#endif 

ColorItem::ColorItem(char *newLabel, rgb_color newValue, BMessage *message):ConfigItem(B_RGB_COLOR_TYPE),BView(BRect(0,0,30,30),"Color",B_FOLLOW_LEFT |B_FOLLOW_TOP,B_WILL_DRAW)
{
	label	= newLabel;
	SetValue(&newValue);

}

void ColorItem::Draw(BRect updateRect)
{
	//calculate FontHeight and postition of the String
	font_height fontHeight;
	GetFontHeight(&fontHeight);
	float baseLine=((fontHeight.ascent)-Bounds().Width())*0.75;
	//draw Label
	SetHighColor(ui_color(B_UI_CONTROL_TEXT_COLOR));
	DrawString(_T(label),BPoint(3,baseLine));
	button->SetViewColor(colorChecker-> ValueAsColor());
}

void* ColorItem::GetValue(void)
{

}

void ColorItem::SetValue(void* newValue)
{
}

