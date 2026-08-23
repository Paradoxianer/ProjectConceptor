#ifndef MESSAGEXMLREADER_H_INCLUDED
#define MESSAGEXMLREADER_H_INCLUDED

#include <support/String.h>
#include <Message.h>
#include <DataIO.h>

#include <map>
using namespace std;

#include "tinyxml.h"


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

	// Same as Read(), but parses from an arbitrary stream instead of the
	// file at filePath - for callers (translators) that only ever get a
	// BPositionIO, not necessarily one backed by a real on-disk path.
	// Returns NULL on a read or parse failure - never a half-built BMessage.
	BMessage* ReadFrom(BPositionIO *source);

private:
	void	Init();
    BString *filePath;
    BMessage* ProcessXML(TiXmlElement *element, BMessage *nodeMessage=NULL);
    static map<BString, int>  	bmessageTypes;

};


#endif // MESSAGEXMLREADER_H_INCLUDED
