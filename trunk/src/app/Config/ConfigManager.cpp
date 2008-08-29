#include "ConfigManager.h"
#include <stdio.h>
#include <interface/InterfaceDefs.h>

#ifdef B_ZETA_VERSION_BETA
	#include <locale/Locale.h>
#else
	#define _T(a) a
#endif



ConfigManager::ConfigManager(BMessage *newConfig, char *path){

}


void ConfigManager::SetConfigMessage(BMessage *newConfig){

}

void  ConfigManager::LoadDefaults(void){

}

void ConfigManager::SaveConfig(void){

}

TiXmlElement  ConfigManager::ProcessMessage(char *name, BMessage *msg){
	TiXmlElement	xmlNode(name);
	xmlNode.SetAttribute("what",msg->what;);
	char		*name;
	uint32	type;
	int32		count;
	void		*data;
	ssize_t	numBytes;
	#ifdef B_ZETA_VERSION_1_0_0
		while (data->GetInfo(B_MESSAGE_TYPE,i ,(const char **)&name, &type, &count) == B_OK) {
	#else
		while (data->GetInfo(B_MESSAGE_TYPE,i ,(char **)&name, &type, &count) == B_OK) {
	#endif
		switch(type){
			case B_MESSAGE_TYPE){
				msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes);
				xmlNode.InsertChild(ProcessMessage(name,(BMessage*)data));
			}
			case B_
		}
	}
}