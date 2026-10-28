#include "AttributeText.h"

#include <ctype.h>
#include <interface/GraphicsDefs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ProjectConceptorDefs.h"


BString NumberText(float number)
{
	char	buffer[32];
	snprintf(buffer,sizeof(buffer),"%g",number);
	return BString(buffer);
}


status_t AttributeValueText(const BMessage &attribute, BString *text)
{
	type_code	type	= B_ANY_TYPE;
	if (attribute.GetInfo(P_C_ATTRIBUTE_VALUE,&type) != B_OK)
		return B_BAD_VALUE;
	text->SetTo("");
	switch (type) {
		case B_STRING_TYPE: {
			const char	*value	= NULL;
			attribute.FindString(P_C_ATTRIBUTE_VALUE,&value);
			text->SetTo(value);
			return B_OK;
		}
		case B_BOOL_TYPE: {
			bool	value	= false;
			attribute.FindBool(P_C_ATTRIBUTE_VALUE,&value);
			text->SetTo(value ? "true" : "false");
			return B_OK;
		}
		case B_FLOAT_TYPE: {
			float	value	= 0;
			attribute.FindFloat(P_C_ATTRIBUTE_VALUE,&value);
			*text	= NumberText(value);
			const char	*unit	= NULL;
			if ((attribute.FindString(P_C_ATTRIBUTE_UNIT,&unit) == B_OK)
					&& (unit[0] != '\0'))
				*text << " " << unit;
			return B_OK;
		}
		case B_RGB_COLOR_TYPE: {
			const rgb_color	*color	= NULL;
			ssize_t			size	= 0;
			if (attribute.FindData(P_C_ATTRIBUTE_VALUE,B_RGB_COLOR_TYPE,
					(const void **)&color,&size) != B_OK)
				return B_BAD_VALUE;
			text->SetToFormat("#%02x%02x%02x",color->red,color->green,color->blue);
			return B_OK;
		}
	}
	return B_BAD_TYPE;
}


status_t ParseNumberWithUnit(const char *text, float *number, BString *unit)
{
	BString	all(text);
	all.Trim();
	int32	end		= 0;
	bool	digits	= false;
	while (end < all.Length()) {
		char	c	= all.ByteAt(end);
		if (isdigit(c))
			digits	= true;
		else if (((c != '-') && (c != '+')) || (end > 0)) {
			if ((strchr(".,' ",c) == NULL) || !digits)
				break;
		}
		end++;
	}
	if (!digits)
		return B_BAD_VALUE;
	BString	numeral(all.String(),end);
	// "25 €": the space before the unit doesn't group digits
	while ((numeral.Length() > 0) && (strchr(".,' ",numeral.ByteAt(numeral.Length() - 1)) != NULL)) {
		numeral.Truncate(numeral.Length() - 1);
		end--;
	}
	numeral.RemoveAll(" ");
	numeral.RemoveAll("'");
	// a separator that comes more than once groups digits
	if (numeral.FindFirst('.') != numeral.FindLast('.'))
		numeral.RemoveAll(".");
	if (numeral.FindFirst(',') != numeral.FindLast(','))
		numeral.RemoveAll(",");
	int32	dot		= numeral.FindLast('.');
	int32	comma	= numeral.FindLast(',');
	if ((dot >= 0) && (comma >= 0))
		numeral.RemoveAll((dot > comma) ? "," : ".");
	numeral.ReplaceAll(',','.');
	char	*rest	= NULL;
	double	value	= strtod(numeral.String(),&rest);
	if ((rest == NULL) || (*rest != '\0'))
		return B_BAD_VALUE;
	*number	= (float)value;
	unit->SetTo(all.String() + end);
	unit->Trim();
	return B_OK;
}
