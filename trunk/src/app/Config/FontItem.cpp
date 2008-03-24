#include "FontItem.h"
#include <stdio.h>
#include <interface/InterfaceDefs.h>

#ifdef B_ZETA_VERSION_BETA
	#include <locale/Locale.h>
#else
	#define _T(a) a
#endif 

FontItem::FontItem(char *newLabel, char* newValue, BMessage *message):ConfigItem(B_STRING_TYPE),BTextControl(BRect(0,0,100,10),"checker",_T(newLabel),NULL,message)
{
	SetValue(newValue);
}

void* FontItem::GetValue(void)
{
	return (void *) Text();
}

void FontItem::SetValue(void* newValue)
{
	SetText((char *)newValue);
}
