#include "PatternItem.h"
#include <stdio.h>
#include <interface/InterfaceDefs.h>
#include <TypeConstants.h>

#ifdef B_ZETA_VERSION_BETA
	#include <locale/Locale.h>
#else
	#define _T(a) a
#endif 

PatternItem::PatternItem(char *newLabel, pattern newValue, BMessage *message):BView(BRect(0,0,100,10),"patterner",B_FOLLOW_LEFT|B_FOLLOW_TOP,0)
{
	SetValue(&newValue);
}

void* PatternItem::GetValue(void)
{
}

void PatternItem::SetValue(void* newValue)
{
}
