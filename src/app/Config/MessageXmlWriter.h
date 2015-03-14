#ifndef MESSAGEXMLWRITER_H_INCLUDED
#define MESSAGEXMLWRITER_H_INCLUDED

#include <String.h>
#include <Message.h>

#include "tinyxml/tinyxml.h"


class MessageXmlWriter
{
public:
	MessageXmlWriter();
	MessageXmlWriter(const BString &fileName);
	~MessageXmlWriter();

	status_t InitCheck();
	void SetTo(const BString &fileName);

	status_t Write(BMessage &message);

	// Vielleicht zur convenience
	// tut im endeffekt nur obiges
	static status_t WriteFile(const BString &fileName, const BMessage &message);

private:
    TiXmlElement	ProcessMessage(BMessage *node);
    BString *filePath;

};


#endif // MESSAGEXMLWRITE_H_INCLUDED
