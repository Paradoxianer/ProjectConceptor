#ifndef TRANSLATOR_EXPORTER_H
#define TRANSLATOR_EXPORTER_H
/*
 * @author Paradoxon powered by Jesus Christ
 */
#include "ImportExport.h"
#include "PDocument.h"

class TranslatorExporter : public ImportExport
{

public:
						TranslatorExporter();
	
	virtual PDocument	Import(BEntry *entry, BMessage *settings);
	virtual PDocument	Import(BDataIO *io, BMessage *settings);
	virtual void*		Export(PDocument *doc, BMessage *settings);
	virtual status_t	GetOutputFormats(const translation_format **outFormats,  int32 *outNumFormats);
	virtual status_t	GetInputFormats(const translation_format **outFormats,  int32 *outNumFormats);

protected:
};
#endif
