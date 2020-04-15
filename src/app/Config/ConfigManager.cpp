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
    if (newConfig == NULL) {
        config = new BMessage();
        TestInit();
    }
    else
        config = newConfig;
    path = new BString(_path);
    SaveConfig();
}

BMessage* ConfigManager::GetConfigMessage(const char *name){
	BMessage	*tmpMessage = NULL;
	status_t	err			= B_OK;
	ssize_t		size;
	
	if (name == NULL)
		return config;
	else {
		err = config->FindData(name, B_MESSAGE_TYPE,(const void **)&tmpMessage,&size);
		if (size<=0)
		err = B_ERROR;
	}
	if (err != B_OK)
		return NULL;
	else
		return tmpMessage;
}


status_t ConfigManager::SetConfigMessage(const char *name,BMessage *newConfig){
	status_t err	= B_OK;
	if (name == NULL) {
		delete config;
		config =newConfig;
	}
	else{
		err = config->ReplaceMessage(name,newConfig);
		if (err == B_NAME_NOT_FOUND || err ==B_BAD_INDEX) {
			err = config->AddMessage(name,newConfig);
		}
	}
	return err;
}

void  ConfigManager::LoadConfig(void){
	TRACE();
	MessageXmlReader messageXml = MessageXmlReader();
	messageXml.SetTo(*path);
	if (messageXml.InitCheck())
        config=messageXml.Read();
	DEBUG_ONLY(config->PrintToStream());
}

void ConfigManager::SaveConfig(){
	TRACE();
	DEBUG_ONLY(config->PrintToStream());
	MessageXmlWriter messageXml = MessageXmlWriter();
	messageXml.SetTo(*path);
	if (messageXml.InitCheck())
        messageXml.Write(*config);
}

void ConfigManager::TestInit(){
	BMessage	* subMessage = new BMessage();
	subMessage->AddBool("TestBool",false);
	subMessage->AddString("TestString","This is a TestString");
	subMessage->AddInt32("TestInt32",42);
	config->AddBool("AutoSave",true);
    config->AddString("Test","TestString");
    config->AddPoint("My nice Point", BPoint(55,77));
    config->AddMessage("SubConfig",subMessage);
}
