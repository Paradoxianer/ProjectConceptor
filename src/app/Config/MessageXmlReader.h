#ifndef MESSAGEXMLREADER_H_INCLUDED
#define MESSAGEXMLREADER_H_INCLUDED

#include <support/String.h>
#include <Message.h>

#include <map>
using namespace std;

#include "tinyxml/tinyxml.h"


class MessageXmlReader
{
public:
	MessageXmlReader();
	MessageXmlReader(const BString &fileName);
	~MessageXmlReader();
	
	status_t InitCheck();
	void SetTo(const BString &fileName);

	BMessage* Read();

	// Vielleicht zur convenience
	// tut im endeffekt nur obiges
	 BMessage* ReadFile(const BString &fileName);

private:
	void	Init();
    BString *filePath;
    BMessage* ProcessXML(TiXmlElement *element, BMessage *nodeMessage=NULL);
    static map<BString, int>  	bmessageTypes;

};


#endif // MESSAGEXMLREADER_H_INCLUDED
