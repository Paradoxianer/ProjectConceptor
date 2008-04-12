#include "StringItem.h"
#include <stdio.h>
#include <interface/InterfaceDefs.h>

#ifdef B_ZETA_VERSION_BETA
	#include <locale/Locale.h>
#else
	#define _T(a) a
#endif 

StringItem::StringItem(char *newLabel, char* newValue, BMessage *message):BTextControl(BRect(0,0,100,10),"checker",_T(newLabel),NULL,message)
{
	SetValue(newValue);
}

void* StringItem::GetValue(void)
{
	return (void *) Text();
}

void StringItem::SetValue(void* newValue)
{
	SetText((char *)newValue);
}
