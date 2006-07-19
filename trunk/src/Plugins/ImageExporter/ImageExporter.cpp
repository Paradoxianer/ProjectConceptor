#include <app/Clipboard.h>
#include <support/Debug.h>

#include <translation/TranslatorRoster.h>
#include "ProjectConceptorDefs.h"
#include "ImageExporter.h"
#include "Indexer.h"


ImageExporter::ImageExporter():ImportExport()
{
}

PDocument *ImageExporter::Import(translation_format *inFormat, BEntry *entry, BPositionIO *io= NULL)
{
}

status_t ImageExporter::Export(PDocument *doc,translation_format *outFormat , BEntry *entry, BPositionIO *io= NULL)
{
	status_t	err 		= B_OK;
	BMessage	*archived	= new BMessage();
	doc->Archive(archived,true);
	//search the translator with this outputformat
	BTranslatorRoster	*roster				= BTranslatorRoster::Default();
	int32				num_translators		= 0;
	int32				i,q					= 0;
	int32				supportNum			= 10;
	translator_id		*translators		= NULL;
	translation_format	*supportFormats		= new translation_format[10];
	translation_format	*allFormats			= NULL;
	translator_info		*translatorInfo		= new translator_info;
	int32				outNumFormats		= 0;
	bool				found				= false;
	roster->GetAllTranslators(&translators, &num_translators);
	allFormats	=	new translation_format[num_translators*10];
	i			=	0;
	while ((!found) && (i<num_translators))
	{
		roster->GetOutputFormats(translators[i], (const translation_format**)&supportFormats,&supportNum);
		for (q=0;q<supportNum;q++) 
		{
			if  ( (strcmp(supportFormats[q].name,outFormat->name) == B_OK ) && (strcmp(supportFormats[q].MIME,outFormat->MIME) == B_OK ) )
			{
				found=true;
				translatorInfo->type		= outFormat->type;
				translatorInfo->translator	= translators[i];
				translatorInfo->group 		= outFormat->group;
				translatorInfo->quality		= outFormat->quality;
				translatorInfo->capability	= outFormat->capability;
				translatorInfo->name		= outFormat->name;
				translatorInfo->MIME		= outFormat->MIME; 
				
			}
		}
		supportNum=10;
		i++;
	}
	
	if (found)
	{
		i--;
		BPositionIO	*input= new BMallocIO();
		archived->Flatten(input);
		if (entry)
			io	= new BFile(entry,B_ERASE_FILE|B_CREATE_FILE|B_READ_WRITE);
		err=	roster->Translate(input,translatorInfo,NULL,io,P_C_DOCUMENT_TYPE);
		delete input;
	}
	delete [] translators; // clean up our droppings
/*	p_c_i_o_format	*pcioformat= new p_c_i_o_format;
//	application/application/x-vnd.projectconceptor-document
	strcpy(pcioformat->name,"Test");
	*outFormats	= pcioformat;
	*outNumFormats = 1;*/
/*	if (outNumFormats)
		*outFormats		= allFormats;
	else
		*outFormats		= NULL;
	return B_OK;
	if (entry) 
	{
		BFile *file=	new BFile(entry,B_WRITE_ONLY | B_ERASE_FILE | B_CREATE_FILE);
		err=file->InitCheck();
		PRINT(("ERROR\tSave file error %s\n",strerror(err)));
		err = archived->Flatten(file);
	}
	if (err==B_OK)
	{
		doc->ResetModified();
	}
	else
		PRINT(("ERROR:\tPDocument","Save error %s\n",strerror(err)));*/
	return err;

}

status_t ImageExporter::GetOutputFormats(const translation_format **outFormats,  int32 *outNumFormats)
{
	BTranslatorRoster			*roster				= BTranslatorRoster::Default();
	int32						num_translators		= 0;
	int32						i,q					= 0;
	int32						outNum				= 0;
	int32						inNum				= 0;
	translator_id				*translators		= NULL;
	const	translation_format	*oFormats;
	const	translation_format	*iFormats;

	translation_format	*allFormats			= NULL;
	*outNumFormats							= 0;
	roster->GetAllTranslators(&translators, &num_translators);
	allFormats	=	new translation_format[num_translators];
	for (i=0;i<num_translators;i++) 
	{
		roster->GetInputFormats(translators[i], &iFormats, &inNum);
		for (q=0;q<inNum;q++) 
		{
			if (iFormats[q].type == B_TRANSLATOR_BITMAP)
			{
				roster->GetOutputFormats(translators[i], &oFormats, &outNum);
				for (int j=0; j<outNum; j++)
				{
					//	and take the first output format that isn't BBitmap
					if( oFormats[j].type != B_TRANSLATOR_BITMAP)
					{
						allFormats[*outNumFormats] = oFormats[j];
						(*outNumFormats)++;
					}
				}
			}
		}
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

status_t ImageExporter::GetInputFormats(const translation_format **outFormats,  int32 *outNumFormats)
{
}