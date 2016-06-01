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
    }
    else
        config = newConfig;
    path = new BString(_path);
}

//rebuild the whole Config View
void ConfigManager::SetConfigMessage(BMessage *newConfig){

}

void  ConfigManager::LoadDefaults(void){
	TRACE();
	MessageXmlReader *messageXml = new MessageXmlReader();
	messageXml->SetTo(*path);
	if (messageXml->InitCheck())
        config=messageXml->Read();
}

void ConfigManager::SaveConfig(){
	TRACE();

}

