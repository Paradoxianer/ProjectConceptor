#ifndef MESSAGEXMLWRITER_H_INCLUDED
#define MESSAGEXMLWRITER_H_INCLUDED

#include <String.h>
#include <Message.h>
#include <DataIO.h>

#include "tinyxml.h"


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
	status_t WriteFile(const BString &fileName, const BMessage &message);

	// Same output as Write(), but to an arbitrary stream instead of the
	// file at filePath - for callers (translators) that only ever get a
	// BPositionIO, not necessarily one backed by a real on-disk path.
	status_t WriteTo(BMessage &message, BPositionIO *destination);

private:
    TiXmlElement	ProcessMessage(const char *name,BMessage *node);
    BString *filePath;
    TiXmlDocument	doc;

};


#endif // MESSAGEXMLWRITE_H_INCLUDED
