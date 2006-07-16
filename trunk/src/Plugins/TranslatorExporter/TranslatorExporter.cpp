#include <app/Clipboard.h>
#include <support/Debug.h>

#include <translation/TranslatorRoster.h>
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

status_t TranslatorExporter::GetOutputFormats(const translation_format **outFormats,  int32 *outNumFormats)
{
	BTranslatorRoster	*roster				= BTranslatorRoster::Default();
	int32				num_translators		= 0;
	int32				i,q					= 0;
	int32				supportNum			= 10;
	translator_id		*translators		= NULL;
	translation_format	*supportFormats		= new translation_format[10];
	translation_format	*allFormats			= NULL;
	*outNumFormats							= 0;
	roster->GetAllTranslators(&translators, &num_translators);
	allFormats	=	new translation_format[num_translators*10];
	for (i=0;i<num_translators;i++) 
	{
		roster->GetOutputFormats(translators[i], (const translation_format**)&supportFormats,&supportNum);
		for (q=0;q<supportNum;q++) 
		{
			if  ( (supportFormats[q].type == P_C_DOCUMENT_TYPE) || (strcmp(supportFormats[q].MIME,"application/application/x-vnd.projectconceptor-document") == B_OK ) )
			{
				allFormats[*outNumFormats] = supportFormats[q];
				(*outNumFormats)++;
			}
		}
		supportNum=0;
	}
	delete [] translators; // clean up our droppings
/*	p_c_i_o_format	*pcioformat= new p_c_i_o_format;
//	application/application/x-vnd.projectconceptor-document
	strcpy(pcioformat->name,"Test");
	*outFormats	= pcioformat;
	*outNumFormats = 1;*/
	if (outNumFormats)
		*outFormats		= allFormats;
	else
		*outFormats		= NULL;
	return B_OK;
}

status_t TranslatorExporter::GetInputFormats(const translation_format **outFormats,  int32 *outNumFormats)
{
}