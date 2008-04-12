#include "ColorItem.h"
#include <stdio.h>
#include <interface/InterfaceDefs.h>

#ifdef B_ZETA_VERSION_BETA
	#include <locale/Locale.h>
#else
	#define _T(a) a
#endif 

ColorItem::ColorItem(BRect frame, char *newLabel, rgb_color newValue, BMessage *message):BControl(frame,"Color",newLabel,message,B_FOLLOW_LEFT_RIGHT , 0)
{
	SetColor(newValue);
}

void ColorItem::Draw(BRect updateRect)
{
	//calculate FontHeight and postition of the String
	font_height fontHeight;
	GetFontHeight(&fontHeight);
	float baseLine=((fontHeight.ascent)-Bounds().Width())*0.75;
	//draw Label
#ifndef __HAIKU__
	SetHighColor(ui_color(B_UI_CONTROL_TEXT_COLOR));
#endif
	DrawString(_T(label),BPoint(3,baseLine));
	button->SetViewColor(colorChecker-> ValueAsColor());
}

rgb_color ColorItem::Color(void)
{
	return rgb_color();
}

void ColorItem::SetColor(rgb_color newValue)
{
}

