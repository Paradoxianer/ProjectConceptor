#include <TranslatorAddOn.h>
#include <TranslationKit.h>

//ProjectConceptor Main Type
#define P_C_MAIN_TYPE 'pcMT'

char translatorName[] = "PPMTranslator";
char translatorInfo[] = "Reads and writes images in the PPM file format. http://www.be.com/";
int32 translatorVersion = 100; /* format is revision+minor*10+major*100 */



translation_format inputFormats[] = {
	{	P_C_MAIN_TYPE,	B_TRANSLATOR_NONE,  0.4, 0.8, "application/application/x-vnd.projectconceptor-document", "ProjectConceptor Standart Translator" },
	{	0, 0, 0, 0, "\0", "\0" }
};

translation_format outputFormats[] = {
	{	P_C_MAIN_TYPE,	B_TRANSLATOR_NONE,  0.4, 0.8, "application/application/x-vnd.projectconceptor-document", "ProjectConceptor Standart Translator"  },
	{	0, 0, 0, 0, "\0", "\0" }
};
