#include <app/Clipboard.h>
#include <support/Debug.h>
#include "ProjectConceptorDefs.h"
#include "TranslatorExporter.h"
#include "Indexer.h"


TranslatorExporter::TranslatorExporter():ImportExport()
{
}

PDocument TranslatorExporter::Import(BEntry *entry, BMessage *settings)
{
}
PDocument TranslatorExporter::Import(BDataIO *io, BMessage *settings)
{
}

void* TranslatorExporter::Export(PDocument *doc, BMessage *settings)
{
}

status_t TranslatorExporter::GetOutputFormats(const p_c_i_o_format **outFormats,  int32 *outNumFormats)
{
	p_c_i_o_format	*pcioformat= new p_c_i_o_format;
	strcpy(pcioformat->name,"Test");
	*outFormats	= pcioformat;
	*outNumFormats = 1;
	return B_OK;
}

status_t TranslatorExporter::GetInputFormats(const p_c_i_o_format **outFormats,  int32 *outNumFormats)
{
}