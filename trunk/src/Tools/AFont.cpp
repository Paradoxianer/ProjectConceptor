#ifndef ARCHIVE_FONT_H
#define ARCHIVE_FONT_H

#include "AFont.h"


AFont::AFont():BFont():BArchivable()
{
}

AFont::AFont():BFont():BArchivable()
{
}

public:
							AFon();
							AFont(BMessage *archive);
							~AFont(void);
	virtual status_t		Archive(BMessage *archive, bool deep = true) const;
	static BArchivable		*Instantiate(BMessage *archive);

};
#endif
