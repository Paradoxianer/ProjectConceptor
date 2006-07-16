#ifndef STANDARD_TRANSLATOR_H
#define STANDARD_TRANSLATOR_H

#include <TranslatorAddOn.h>
#include <TranslationKit.h>
#include "ProjectConceptorDefs.h"
//ProjectConceptor Main Type

char translatorName[] = "ProjectConceptorTranslator";
char translatorInfo[] = "Reads and writes the nativ ProjectConcpetor files";
int32 translatorVersion = 10; /* format is revision+minor*10+major*100 */



translation_format inputFormats[] = {
	{	P_C_DOCUMENT_TYPE,	B_TRANSLATOR_NONE,  0.4, 0.8, "application/application/x-vnd.projectconceptor-document", "ProjectConceptor Document" },
	{	0, 0, 0, 0, "\0", "\0" }
};

translation_format outputFormats[] = {
	{	P_C_DOCUMENT_TYPE,	B_TRANSLATOR_NONE,  0.4, 0.8, "application/application/x-vnd.projectconceptor-document", "ProjectConceptor Document" },
	{	0, 0, 0, 0, "\0", "\0" }
};

#endif
