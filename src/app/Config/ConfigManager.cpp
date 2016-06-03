#include "ConfigManager.h"
#include <stdio.h>
#include <app/Messenger.h>
#include <interface/InterfaceDefs.h>
#include <storage/Entry.h>
#include <storage/Path.h>
#include <support/String.h>

#include <support/Debug.h>
#include <map>
#include <Catalog.h>

#include "MessageXmlReader.h"
#include "MessageXmlWriter.h"



ConfigManager::ConfigManager(char *_path, BMessage *newConfig){
// if no Message was passed we just create a Message :)
    if (newConfig==NULL) {
        config = new BMessage();
        config->AddBool("AutoSave",true);
        config->AddString("Test","TestString");
        config->AddPoint("My nice Point", BPoint(55,77));
    }
    else
        config = newConfig;
    path = new BString(_path);
    SaveConfig();
}

//rebuild the whole Config View
void ConfigManager::SetConfigMessage(BMessage *newConfig){

}

void  ConfigManager::LoadConfig(void){
	TRACE();
	MessageXmlReader messageXml = MessageXmlReader();
	messageXml.SetTo(*path);
	if (messageXml.InitCheck())
        config=messageXml.Read();
    config->PrintToStream();
}

void ConfigManager::SaveConfig(){
	TRACE();
	config->PrintToStream();
	MessageXmlWriter messageXml = MessageXmlWriter();
	messageXml.SetTo(*path);
	if (messageXml.InitCheck())
        messageXml.Write(*config);
}

