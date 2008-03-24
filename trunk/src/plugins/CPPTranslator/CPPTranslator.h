#ifndef CPP_TRANSLATOR_H
#define CPP_TRANSLATOR_H

#include <TranslatorAddOn.h>
#include <TranslationKit.h>
#include <app/Message.h>
#include <List.h>
#include "ProjectConceptorDefs.h"
//ProjectConceptor Main Type

char translatorName[] = "CPPTranslator";
char translatorInfo[] = "Translates ProjectConceptor files from and to sourcecode and makefiles";
int32 translatorVersion = 10; /* format is revision+minor*10+major*100 */



translation_format inputFormats[] = {
	{	P_C_DOCUMENT_RAW_TYPE, B_TRANSLATOR_TEXT,  0.4, 0.8, text/plain, "makefile" },
	{	P_C_DOCUMENT_TYPE, B_TRANSLATOR_TEXT,  0.4, 0.8, "text/x-source-code", "cpp header" },
	{	0, 0, 0, 0, "\0", "\0" }
};

translation_format outputFormats[] = {
	{	P_C_DOCUMENT_RAW_TYPE,	B_TRANSLATOR_NONE,  0.4, 0.8, P_C_DOCUMENT_MIMETYPE, "ProjectConceptor RAW Document" },
	{	P_C_DOCUMENT_RAW_TYPE,	B_TRANSLATOR_NONE,  0.4, 0.8, P_C_DOCUMENT_MIMETYPE, "ProjectConceptor RAW Document" },
	{	0, 0, 0, 0, "\0", "\0" }
};
status_t FindMakeFIleHint(BPositionIO * inSource);

class Converter
{
public:
				Converter(BPositionIO * inSource, BMessage * ioExtension,	BPositionIO * outDestination);
	status_t	ConvertNode2CPP();
	status_t	ConvertCPP2Node();
	status_t	ConvertPDoc2make();
	status_t	Convertmake2PDoc();

protected:
	status_t	WriteConnection(BMessage *connection);
	status_t	FindConnections(BMessage *node);

	BPositionIO *in;
	BMessage	*config;
	BPositionIO *out;
	BList		*doneNode;
	BList		*doneConnection;
	BMessage	*allConnections;
	BMessage	*allNodes;
	BMessage	*selected;
	BMessage	*commandManager;

};

#endif
