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
	
	virtual PDocument*	Import(translation_format *inFormat, BEntry *entry, BPositionIO *io= NULL);
	virtual status_t	Export(PDocument *doc,  translation_format *outFormat,BEntry *entry, BPositionIO *io= NULL);
	virtual status_t	GetOutputFormats(const translation_format **outFormats,  int32 *outNumFormats);
	virtual status_t	GetInputFormats(const translation_format **outFormats,  int32 *outNumFormats);

protected:
};
#endif
