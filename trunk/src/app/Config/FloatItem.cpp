#include "FloatItem.h"
#include <stdio.h>
#include <interface/InterfaceDefs.h>

#ifdef B_ZETA_VERSION_BETA
	#include <locale/Locale.h>
#else
	#define _T(a) a
#endif 

FloatItem::FloatItem(char *newLabel, float newValue, BMessage *message):BTextControl(BRect(0,0,100,10),"checker",_T(newLabel),"",message)
{
	SetValue((void *)&newValue);
}
void* FloatItem::GetValue(void)
{
	float value = atof(Text());
	return (void *) &value;
}

void FloatItem::SetValue(void* newValue)
{
	float	value		= *((float *)newValue);
	char*	svalue		= new char[24];
	sprintf(svalue,"%.2f",newValue);
	SetText(svalue);
}
