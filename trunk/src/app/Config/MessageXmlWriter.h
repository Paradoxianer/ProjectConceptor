#ifndef MESSAGEXMLWRITER_H_INCLUDED
#define MESSAGEXMLWRITER_H_INCLUDED

class MessageXmlWriter
{
public:
	MessageXmlWriter();
	MessageXmlWriter(const BString &fileName);
	~MessageXmlWriter();

	bool InitCheck();
	void SetTo(const BString &fileName);

	status_t Write(BMessage &message);

	// Vielleicht zur convenience
	// tut im endeffekt nur obiges
	static status_t WriteFile(const BString &fileName, const BMessage &message);

protected:
    BString filePath;

private:
    TiXmlElement	ProcessMessage(BMessage *node);

};


#endif // MESSAGEXMLREADER_H_INCLUDED
