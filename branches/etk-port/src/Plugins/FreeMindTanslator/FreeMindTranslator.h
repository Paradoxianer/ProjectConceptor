#ifndef FREE_MIND_TRANSLATOR_H
#define FREE_MIND_TRANSLATOR_H

#include <TranslatorAddOn.h>
#include <TranslationKit.h>
#include <app/Message.h>
#include <List.h>
#include "ProjectConceptorDefs.h"
//ProjectConceptor Main Type

char translatorName[] = "FreeMindTranslator";
char translatorInfo[] = "Read and write FreeMind data";
int32 translatorVersion = 10; /* format is revision+minor*10+major*100 */



translation_format inputFormats[] = {
	{	P_C_DOCUMENT_RAW_TYPE,	B_TRANSLATOR_NONE,  0.4, 0.8, P_C_DOCUMENT_MIMETYPE, "ProjectConceptor RAW Document" },
	{	P_C_DOCUMENT_TYPE,	B_TRANSLATOR_NONE,  0.4, 0.8, P_C_DOCUMENT_MIMETYPE, "ProjectConceptor Document" },
	{	P_C_DOCUMENT_TYPE,	B_TRANSLATOR_TEXT,  0.4, 0.8, "text/xml", "FreeMind" },
	{	0, 0, 0, 0, "\0", "\0" }
};

translation_format outputFormats[] = {
//	{	P_C_DOCUMENT_TYPE,	B_TRANSLATOR_NONE,  0.4, 0.8, P_C_DOCUMENT_MIMETYPE, "ProjectConceptor Document" },
	{	P_C_DOCUMENT_RAW_TYPE,	B_TRANSLATOR_NONE,  0.4, 0.8, P_C_DOCUMENT_MIMETYPE, "ProjectConceptor RAW Document" },
	{	P_C_DOCUMENT_TYPE,	B_TRANSLATOR_TEXT,  0.4, 0.8, "text/xml", "FreeMind" },
	{	0, 0, 0, 0, "\0", "\0" }
};

/*status_t ConvertPDoc2FreeMind(BPositionIO * inSource, BMessage * ioExtension,	BPositionIO * outDestination);
status_t ConvertFreeMind2PDoc(BPositionIO * inSource, BMessage * ioExtension,	BPositionIO * outDestination);
status_t ConvertPDoc2PDoc(BPositionIO * inSource, BMessage * ioExtension,	BPositionIO * outDestination);
status_t WriteNode(BList *done,BMessage *connection,BPositionIO * outDestination);
status_t FindConnections(BList *done,BMessage *node);*/

class Converter
{
public:
				Converter(BPositionIO * inSource, BMessage * ioExtension,	BPositionIO * outDestination);
	status_t	ConvertPDoc2FreeMind();
	status_t	ConvertFreeMind2PDoc();

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
	// config stuff
	bool		saveUndo;
	bool		saveMacro;
	bool		restoreWindowPos;
	int32		undoLevel;

};

#endif
