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