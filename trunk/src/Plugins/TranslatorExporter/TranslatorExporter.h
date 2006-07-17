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
	
	virtual PDocument*	Import(BMessage *settings, BEntry *entry,BDataIO *io= NULL);
	virtual status_t	Export(PDocument *doc, BMessage *settings,BEntry *entry,BDataIO *io= NULL);
	virtual status_t	GetOutputFormats(const translation_format **outFormats,  int32 *outNumFormats);
	virtual status_t	GetInputFormats(const translation_format **outFormats,  int32 *outNumFormats);

protected:
};
#endif
