#include "MessageXmlReader.h"
#include "tinyxml/tinyxml.h"
#include <storage/File.h>
#include <support/Debug.h>
#include <mail/mail_encoding.h>
#include <string.h>


map<BString, int> MessageXmlReader::bmessageTypes;

MessageXmlReader::MessageXmlReader(){
    Init();
}

MessageXmlReader::MessageXmlReader(const BString &fileName){
    Init();
    SetTo(fileName);
}
// @todo implement B_REF_TYPE and B_POINTER_REF
void MessageXmlReader::Init()
{
	filePath = new BString("");
	   if (bmessageTypes.size() == 0){
        bmessageTypes[BString("BMessage")]=1;
        bmessageTypes[BString("bool")]=2;
        bmessageTypes[BString("int8")]=3;
        bmessageTypes[BString("int16")]=4;
        bmessageTypes[BString("int32")]=5;
        bmessageTypes[BString("int64")]=6;
        bmessageTypes[BString("float")]=7;
        bmessageTypes[BString("double")]=8;
        bmessageTypes[BString("string")]=9;
        bmessageTypes[BString("BPoint")]=10;
        bmessageTypes[BString("BRect")]=11;
        bmessageTypes[BString("B_REF_TYPE")]=12;
        bmessageTypes[BString("B_POINTER_TYPE")]=13;
    }
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
	return NULL;
}


BMessage* MessageXmlReader::ReadFile(const BString &fileName) {
   	TiXmlDocument	 doc;
	doc.LoadFile(fileName.String());
	if (doc.Error())
        return NULL;
	else{
		TRACE();
		TiXmlElement *element =	 doc.FirstChildElement("BMessage");
		return ProcessXML(element);
	}
	return NULL;
}

BMessage* MessageXmlReader::ProcessXML(TiXmlElement *element, BMessage *nodeMessage){
	BMessage *bMessage	= new BMessage;
	const char *what=element->Attribute("what");
	if (what == NULL)
		bMessage->what = 0;
	else	
		bMessage->what = atoi(what);
	TiXmlElement *child;
	child= element->FirstChildElement ();
	for( ; child;){
		printf("type: %s\n",child->Attribute("type"));
		switch (	bmessageTypes[BString(child->Attribute("type"))]){
			case 1:{
				const char *cName = child->Attribute("name");
				bMessage->AddMessage(cName, (const BMessage *)ProcessXML(child));
			}
			break;
			case 2:{
				BString value = BString(child->Attribute("value"));
				if (value.ICompare("TRUE") == B_OK)
					bMessage->AddBool(child->Attribute("name"),true);
				else
					bMessage->AddBool(child->Attribute("name"),false);
			}
			break;
			case 3:{
					bMessage->AddInt8(child->Attribute("name"),atoi(child->Attribute("value")));
			}
			break;
			case 4:{
					bMessage->AddInt16(child->Attribute("name"),atoi(child->Attribute("value")));
			}
			break;
			case 5:{
					bMessage->AddInt32(child->Attribute("name"),atoi(child->Attribute("value")));
			}
			break;
			case 6:{
					bMessage->AddInt64(child->Attribute("name"),atol(child->Attribute("value")));
			}
			break;
			case 7:{
					bMessage->AddFloat(child->Attribute("name"),atof(child->Attribute("value")));
			}
			break;
			case 8:{
					bMessage->AddDouble(child->Attribute("name"),atof(child->Attribute("value")));
			}
			break;
			case 9:{
					bMessage->AddString(child->Attribute("name"),child->Attribute("value"));
			}
			break;
			case 10:{
					BPoint tmpPoint;
					tmpPoint.x = atof(child->Attribute("x"));
					tmpPoint.y = atof(child->Attribute("y"));
					bMessage->AddPoint(child->Attribute("name"),tmpPoint);
			}
			break;
			case 11:{
					BRect tmpRect;
					tmpRect.top			= atof(child->Attribute("top"));
					tmpRect.left			= atof(child->Attribute("left"));
					tmpRect.bottom	= atof(child->Attribute("bottom"));
					tmpRect.right		= atof(child->Attribute("right"));
					bMessage->AddRect(child->Attribute("name"),tmpRect);
			}
			break;
			case 12:{
				printf("type: %s not yet implementd\n",child->Attribute("type"));
				break;
 			}
			case 13:{
				bMessage->AddPointer(child->Attribute("name"),(const void*)atoi(child->Attribute("value")));
				break;
			}
			case 14:{
				printf("type: %s not yet implementd\n",child->Attribute("type"));

			}
			break;
			default :{
				char 		code[4];
				const char 	*value	= child->Attribute("value");
				ssize_t		size	= strlen(value);
				ssize_t		len;
				char		*data	= new char[((size*2)/3)+1];
				const char	*encode	= child->Attribute("encode");
				//first make shure we only have 4 chars
				strncpy((char*)&code, child->Attribute("type"), 4);
				uint32		type = code[3] << 24 | code[2] << 16 | code[1] << 8 | code[0];
				//for  now we just do a base64 decode
				if (value != NULL)
				len=decode_base64(data, value,size);
				if (len >0)
					bMessage->AddData(child->Attribute("name"), type,(const void*)data,len);
				bMessage->PrintToStream();
			}
		}
		child= child->NextSiblingElement();
	}
	return bMessage;
}
