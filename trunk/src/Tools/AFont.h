#ifndef ARCHIVE_FONT_H
#define ARCHIVE_FONT_H

#include <app/Message.h>
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
class AFont : public BFont, public BArchivable
{
public:
							AFont();
							AFont(BMessage *archive);
							~AFont(void);
	virtual status_t		Archive(BMessage *archive, bool deep = true) const;
	static BArchivable		*Instantiate(BMessage *archive);

};
#endif
