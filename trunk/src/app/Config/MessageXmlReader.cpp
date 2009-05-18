#include "MessageXmlReader.h"
#include "tinyxml/tinyxml.h"


MessageXmlReader::MessageXmlReader(){
}

MessageXmlReader::MessageXmlReader(const BString &fileName){
}

MessageXmlReader::~MessageXmlReader(){
}

bool MessageXmlReader::InitCheck(){
}

void MessageXmlReader::SetTo(const BString &fileName){
}

BMessage MessageXmlReader::Read(){
    TiXmlDocument	 doc;
	doc.LoadFile(filePath.String());
	if (doc.Error())
		//need to do something
	//	start=0;
	;
	else{
		TRACE();
		TiXmlElement *element =	 doc.FirstChildElement("BMessage");
		ProcessXML(element)->PrintToStream();
	}
}

static BMessage MessageXmlReader::ReadFile(const BString &fileName){

}
