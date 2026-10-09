#include "VectorIcon.h"

#include <Application.h>
#include <Bitmap.h>
#include <IconUtils.h>
#include <Resources.h>


BBitmap* LoadVectorIcon(BResources *res, const char *name, float size)
{
	if ((res == NULL) || (name == NULL))
		return NULL;
	size_t		length	= 0;
	const void	*data	= res->LoadResource(B_VECTOR_ICON_TYPE,name,&length);
	if (data == NULL)
		return NULL;
	BBitmap	*icon	= new BBitmap(BRect(0,0,size-1,size-1),B_RGBA32);
	if ((icon->InitCheck() != B_OK)
		|| (BIconUtils::GetVectorIcon((const uint8 *)data,length,icon) != B_OK)) {
		delete icon;
		return NULL;
	}
	return icon;
}


BBitmap* LoadAppVectorIcon(const char *name, float size)
{
	return LoadVectorIcon(BApplication::AppResources(),name,size);
}
