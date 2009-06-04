#include "MessageXmlReader.h"
#include "tinyxml/tinyxml.h"
#include <storage/File.h>
#include <support/Debug.h>




MessageXmlReader::MessageXmlReader(){
    filePath=new BString("");
    if (bmessageTypes.size() == 0){
        bmessageTypes[BString("BMessage")]=1;
        bmessageTypes[BString("bool")]=2;
        bmessageTypes[BString("int8")]=3;
        bmessageTypes[BString("nt16")]=4;
        bmessageTypes[BString("nt32")]=5;
        bmessageTypes[BString("nt64")]=6;
        bmessageTypes[BString("float")]=7;
        bmessageTypes[BString("double")]=8;
        bmessageTypes[BString("string")]=9;
        bmessageTypes[BString("BPoint")]=10;
        bmessageTypes[BString("BRect")]=11;
        bmessageTypes[BString("B_REF_TYPE")]=12;
        bmessageTypes[BString("B_POINTER_REF")]=13;
    }
}

MessageXmlReader::MessageXmlReader(const BString &fileName){
    SetTo(fileName);
}

MessageXmlReader::~MessageXmlReader(){
}

status_t MessageXmlReader::InitCheck(){
    BFile   file (filePath->String(),B_READ_ONLY);
    return file.InitCheck();
}

void MessageXmlReader::SetTo(const BString &fileName){
    filePath->SetTo(fileName);
}

BMessage* MessageXmlReader::Read(){
    TiXmlDocument	 doc;
	doc.LoadFile(filePath->String());
	if (doc.Error())
        return NULL;
	else{
		TRACE();
		TiXmlElement *element =	 doc.FirstChildElement("BMessage");
		return ProcessXML(element);
	}
}

BMessage* MessageXmlReader::ReadFile(const BString &fileName){

}

BMessage* MessageXmlReader::ProcessXML(TiXmlElement *element){
	BMessage *bMessage	= new BMessage;
	bMessage->what = atoi(element->Attribute("what"));
	element= element->FirstChildElement("Data");
	for( ; element;){
		printf("type %s\n",element->Attribute("type"));
		switch (	bmessageTypes[BString(element->Attribute("type"))]){
			case 1:{
				bMessage->AddMessage(element->Attribute("name"), (const BMessage *)ProcessXML(element));
			}
			break;
			case 2:{
				BString value = BString(element->Attribute("value"));
				if (value.ICompare("TRUE") == B_OK)
					bMessage->AddBool(element->Attribute("name"),true);
				else
					bMessage->AddBool(element->Attribute("name"),false);
			}
			break;
			case 3:{
					bMessage->AddInt8(element->Attribute("name"),atoi(element->Attribute("value")));
			}
			break;
			case 4:{
					bMessage->AddInt16(element->Attribute("name"),atoi(element->Attribute("value")));
			}
			break;
			case 5:{
					bMessage->AddInt32(element->Attribute("name"),atoi(element->Attribute("value")));
			}
			break;
			case 6:{
					bMessage->AddInt64(element->Attribute("name"),atol(element->Attribute("value")));
			}
			break;
			case 7:{
					bMessage->AddInt32(element->Attribute("name"),atof(element->Attribute("value")));
			}
			break;
			case 8:{
					bMessage->AddInt32(element->Attribute("name"),atof(element->Attribute("value")));
			}
			break;
			case 9:{
					bMessage->AddString(element->Attribute("name"),element->Attribute("value"));
			}
			break;
			case 10:{
					BPoint tmpPoint;
					tmpPoint.x = atof(element->Attribute("x"));
					tmpPoint.y = atof(element->Attribute("y"));
					bMessage->AddPoint(element->Attribute("name"),tmpPoint);
			}
			break;
			case 11:{
					BRect tmpRect;
					tmpRect.top			= atof(element->Attribute("top"));
					tmpRect.left			= atof(element->Attribute("left"));
					tmpRect.bottom	= atof(element->Attribute("bottom"));
					tmpRect.right		= atof(element->Attribute("right"));
					bMessage->AddRect(element->Attribute("name"),tmpRect);
			}
			break;
			case 12:{
			}
			break;
			case 13:{
			}
			break;
			case 14:{
			}
			break;
		}
		element= element->NextSiblingElement();
	}
	return bMessage;
}
