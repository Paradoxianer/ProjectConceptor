#include "MessageXmlWriter.h"

MessageXmlReader::MessageXmlWriter(){
}

MessageXmlWriter::MessageXmlWriter(const BString &fileName){
}

MessageXmlWriter::~MessageXmlWriter(){
}

bool MessageXmlWriter::InitCheck(){
}

void MessageXmlWriter::SetTo(const BString &fileName){
}

status_t MessageXmlWriter::Write(BMessage &message){
}

static status_t MessageXmlWriter::WriteFile(const BString &fileName, BMessage &message){
}
