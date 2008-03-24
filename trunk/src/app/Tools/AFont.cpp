#include "AFont.h"
#include <string.h>

AFont::AFont(): BFont(), BArchivable()
{
}

AFont::AFont(BMessage *archive): BFont(), BArchivable()
{
	float	tmpFloat;
	int8	tmpInt8;
	int16	tmpInt16;
	int32	tmpInt;
	char	*family;
	char	*style;
	if (archive->FindFloat("Font::Rotation",&tmpFloat) == B_OK)
		SetRotation(tmpFloat);
	if (archive->FindFloat("Font::Size",&tmpFloat) == B_OK)
		SetSize(tmpFloat);
	if (archive->FindFloat("Font::Spacing",&tmpFloat) == B_OK)
		SetSpacing(tmpFloat);
	if (archive->FindFloat("Font::Shear",&tmpFloat) == B_OK)
		SetShear(tmpFloat);
	if (archive->FindInt8("Font::Encoding",&tmpInt8) == B_OK)
		SetEncoding(tmpInt8);
	if (archive->FindInt16("Font::Face",&tmpInt16) == B_OK)
		SetFace(tmpInt16);
	if (archive->FindInt32("Font::Flags",&tmpInt) == B_OK)
		SetFlags(tmpInt);
	if ((archive->FindString("Font::Family",(const char **)&family) == B_OK) &&
		(archive->FindString("Font::Style",(const char **)&style) == B_OK))
	{
		font_family fontFamily;
		strcpy(fontFamily , family);
		font_style fontStyle;
		strcpy(fontStyle, style);
		SetFamilyAndStyle(fontFamily,fontStyle);
	}

}

AFont::~AFont()
{
}


status_t AFont::Archive(BMessage *archive, bool) const
{
	status_t	err		= B_OK;
	archive->AddString("class", "Font");
	archive->AddFloat("Font::Rotation",Rotation());
	archive->AddFloat("Font::Size",Size());
	archive->AddFloat("Font::Spacing", Spacing());
	archive->AddFloat("Font::Shear",Shear());
	archive->AddInt8("Font::Encoding",Encoding());
	archive->AddInt16("Font::Face",Face());
	archive->AddInt32("Font::Flags",Flags());
	font_family	*family;
	font_style	*style;
	GetFamilyAndStyle(family, style);
	archive->AddString("Font::Family",(const char*)family);
	archive->AddString("Font::Style",(const char*)style);
	return err;
}

BArchivable *AFont::Instantiate(BMessage *archive)
{
	if ( !validate_instantiation(archive, "TheClass") )
		return NULL;
	return new AFont(archive);
}

