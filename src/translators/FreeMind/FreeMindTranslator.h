#ifndef FREE_MIND_TRANSLATOR_H
#define FREE_MIND_TRANSLATOR_H

#include <TranslatorAddOn.h>
#include <TranslationKit.h>
#include <app/Message.h>
#include <List.h>
#include <map>
#include <set>
#include <String.h>
#include "ProjectConceptorDefs.h"
#include "tinyxml.h"

#define P_C_FREEMIND_TYPE 'free'


//ProjectConceptor Main Type

const int32	X_START		= 500;
const int32	Y_START		= 500;
const int32	NODE_WIDTH	= 150;
const int32	NODE_HEIGHT	= 100;
char translatorName[]	= "FreeMindTranslator";
char translatorInfo[]	= "Read and write FreeMind data";
int32 translatorVersion = 10; /* format is revision+minor*10+major*100 */



translation_format inputFormats[] = {
	{	P_C_DOCUMENT_RAW_TYPE,	B_TRANSLATOR_NONE,  0.4, 0.8, P_C_DOCUMENT_MIMETYPE, "ProjectConceptor RAW Document" },
	//{	P_C_DOCUMENT_TYPE,	B_TRANSLATOR_NONE,  0.4, 0.8, P_C_DOCUMENT_MIMETYPE, "ProjectConceptor Document" },
	{	P_C_FREEMIND_TYPE,	B_TRANSLATOR_TEXT,  0.4, 0.8, "text/xml", "FreeMind" },
	{	0, 0, 0, 0, "\0", "\0" }
};

translation_format outputFormats[] = {
//	{	P_C_DOCUMENT_TYPE,	B_TRANSLATOR_NONE,  0.4, 0.8, P_C_DOCUMENT_MIMETYPE, "ProjectConceptor Document" },
	{	P_C_DOCUMENT_RAW_TYPE,	B_TRANSLATOR_NONE,  0.4, 0.8, P_C_DOCUMENT_MIMETYPE, "ProjectConceptor RAW Document" },
	{	P_C_FREEMIND_TYPE,	B_TRANSLATOR_TEXT,  0.4, 0.8, "text/xml", "FreeMind" },
	{	0, 0, 0, 0, "\0", "\0" }
};


class Converter
{
public:
					Converter(BPositionIO * inSource, BMessage * ioExtension,	BPositionIO * outDestination);
	status_t		ConvertPDoc2FreeMind();
	status_t		ConvertFreeMind2PDoc();

protected:
	//used to convert PDoc 2 FreeMind
	TiXmlElement	ProcessNode(BMessage *node);
	BMessage		*GuessStartNode(void);

	//used to convert FreeMind 2 PDoc
	status_t		CreateNode(BMessage *nodeS,BMessage *connectionS,TiXmlElement *parent, int32 level, int32 line);
	status_t		CreateConnection(BMessage *connectionS,TiXmlElement *start,TiXmlElement *end);
	int32			GetID(const char *idString);
	int32			GetRGB(const char *rgbString);
	BPositionIO 	*in;
	BMessage		*config;
	BPositionIO		*out;
	std::set<int32>		processedIDs;
	std::map<int32,BMessage*>	nodes;
	std::map<int32,BMessage*>	connections;
	// FreeMind ids ("ID_123", "Freemind_Link_123") -> sequential node ids
	std::map<BString,int32>	importIds;

	BMessage		*allConnections;
	BMessage		*allNodes;
	BMessage		*selected;
	BRect			middel;

};

#endif
