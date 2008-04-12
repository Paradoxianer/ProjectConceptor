#include "BoolItem.h"
#include <stdio.h>
#include <interface/InterfaceDefs.h>

#ifdef B_ZETA_VERSION_BETA
	#include <locale/Locale.h>
#else
	#define _T(a) a
#endif 

BoolItem::BoolItem(char *newLabel, bool newValue, BMessage *message):BCheckBox(BRect(0,0,100,10),"checker",_T(newLabel),message)
{
	SetValue((void *)newValue);
}



void* BoolItem::GetValue(void)
{
	bool value = Value();
	return (void *) &value;
}

void BoolItem::SetValue(void* newValue)
{
	bool	value		= *((bool *)newValue);
	BCheckBox::SetValue(value);
}
