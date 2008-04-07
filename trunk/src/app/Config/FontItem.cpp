#include "FontItem.h"
#include <stdio.h>
#include <interface/InterfaceDefs.h>

#ifdef B_ZETA_VERSION_BETA
	#include <locale/Locale.h>
#else
	#define _T(a) a
#endif 

FontItem::FontItem(char *, char*, BMessage *)
{
}

AFont* FontItem::GetValue(void)
{
	return NULL;
}

void FontItem::SetValue(AFont* )
{
}
