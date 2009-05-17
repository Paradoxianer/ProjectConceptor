#ifndef MESSAGEXMLREADER_H_INCLUDED
#define MESSAGEXMLREADER_H_INCLUDED

class MessageXmlReader
{
public:
	MessageXmlReader();
	MessageXmlReader(const BString &fileName);
	~MessageXmlReader();

	bool InitCheck();
	void SetTo(const BString &fileName);

	BMessage Read();

	// Vielleicht zur convenience
	// tut im endeffekt nur obiges
	static BMessage ReadFile(const BString &fileName);

protected:
    BFile file;
};


#endif // MESSAGEXMLREADER_H_INCLUDED
