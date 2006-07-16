#ifndef IMPORT_EXPORT_H
#define IMPORT_EXPORT_H

#include "PDocument.h"
#include <app/Message.h>
#include <support/DataIO.h>
#include <storage/Entry.h>



class ImportExport
{


public:
							ImportExport(void);

		virtual PDocument	Import(BEntry *entry, BMessage *settings)	= 0;
		virtual PDocument	Import(BDataIO *io, BMessage *settings)		= 0;
		virtual void*		Export(PDocument *doc, BMessage *settings)	= 0;
		virtual status_t	GetOutputFormats(const translation_format **outFormats,  int32 *outNumFormats)	= 0;
		virtual status_t	GetInputFormats(const translation_format **outFormats,  int32 *outNumFormats)	= 0;

protected:

private:
};
#endif
