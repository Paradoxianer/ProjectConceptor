#ifndef ARCHIVE_FONT_H
#define ARCHIVE_FONT_H

#include <support/Archivable.h>
#include <interface/Font.h>

/**
 * @class AFont
 *
 * @brief  Font Class wich is able to store Font information in a BMessage
 *
 * @author Paradoxon powered by Jesus Christ
 * @version 0.01
 * @date 2008/02/04
 * @Contact: mail@projectconceptor.de
 *
 */

AFont: AFont(): BFont(), BArchivable()
{
}

AFont:AFont(BMessage *archive): BFont(), BArchivable()
{
	float	tmpFloat;
	int8	tmpInt8;
	int16	tmpInt16;
	int32	tmpInt;
	char	*family;
	char	*style;
	if (archive->FindFloat("Font::Rotation",&tmpFloat) == B_OK)
		SetRoation(tmpFloat);
	if (archive->FindFloat("Font::Size",&tmpFloat) == B_OK)
		SetSize(tmpFloat);
	if (archive->FindFloat("Font::Spacing",&tmpFloat) == B_OK)
		SetSpacing(tmpFloat);
	if (archive->FindFloat("Font::Shear",&tmpFloat) == B_OK)
		SetShear(tmpFloat);
	if (archive->FindInt8("Font::Encoding",&tmpInt8) == B_OK)
		SetEncoding(tmpInt8);
	if (archive->FindInt16("Font::Face",&tmpInt16) == B_OK)
		SetRoation(tmpInt16);
	if (archive->FindInt32("Font::Flags",&tmpInt) == B_OK)
		SetRoation(tmpInt);
	if (archive->FindFloat("Font::Rotation",&tmpFloat) == B_OK)
		SetRoation(tmpFloat);
	if ((archive->FindString("Font::Family",family) == B_OK) &&
		(archive->FindString("Font::Style",style) == B_OK))
	{
		SetFamilyAndStyle((const font_family)family,(const font_style)style);
	}

}

status_t AFont:Archive(BMessage *archive, bool deep = true)
{
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
	archive->AddString("Font::Family",family);
	archive->AddString("Font::Style",style);
}

BArchivable *AFont:Instantiate(BMessage *archive)
{
	if ( !validate_instantiation(archive, "TheClass") )
		return NULL;
	return new AFont(archive);
}