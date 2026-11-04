#ifndef FREE_MIND_TRANSLATOR_H
#define FREE_MIND_TRANSLATOR_H

#include <TranslatorAddOn.h>
#include <TranslationKit.h>

#include "ProjectConceptorDefs.h"

char translatorName[]	= "FreeMindTranslator";
char translatorInfo[]	= "Read and write FreeMind mind maps";
int32 translatorVersion = 200; /* format is revision+minor*10+major*100 */

translation_format inputFormats[] = {
	{	P_C_DOCUMENT_RAW_TYPE,	B_TRANSLATOR_NONE,  0.4, 0.8, P_C_DOCUMENT_MIMETYPE, "ProjectConceptor RAW Document" },
	{	P_C_FREEMIND_TYPE,	B_TRANSLATOR_TEXT,  0.4, 0.8, "application/x-freemind", "FreeMind" },
	{	0, 0, 0, 0, "\0", "\0" }
};

translation_format outputFormats[] = {
	{	P_C_DOCUMENT_RAW_TYPE,	B_TRANSLATOR_NONE,  0.4, 0.8, P_C_DOCUMENT_MIMETYPE, "ProjectConceptor RAW Document" },
	{	P_C_FREEMIND_TYPE,	B_TRANSLATOR_TEXT,  0.4, 0.8, "application/x-freemind", "FreeMind" },
	{	0, 0, 0, 0, "\0", "\0" }
};

#endif
