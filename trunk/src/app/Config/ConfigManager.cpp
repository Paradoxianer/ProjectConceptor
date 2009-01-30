#include "ConfigManager.h"
#include <stdio.h>
#include <app/Messenger.h>
#include <interface/InterfaceDefs.h>
#include <storage/Path.h>

#ifdef B_ZETA_VERSION_BETA
	#include <locale/Locale.h>
#else
	#define _T(a) a
#endif



ConfigManager::ConfigManager(BMessage *newConfig, char *path){

}

//rebuild the hole Config View
void ConfigManager::SetConfigMessage(BMessage *newConfig){

}

void  ConfigManager::LoadDefaults(void){

}

void ConfigManager::SaveConfig(void){

}

TiXmlElement  ConfigManager::ProcessMessage(char *nodeName, BMessage *msg){
	TiXmlElement	xmlNode(nodeName);
	xmlNode.SetAttribute("type","BMessage");
	xmlNode.SetAttribute("what",msg->what);
	char		*name;
	uint32	type;
	int32		count;
	void		*data;
	int32		i;
	ssize_t	numBytes;
	#ifdef B_ZETA_VERSION_1_0_0
		while (msg->GetInfo(B_ANY_TYPE,i ,(const char **)&name, &type, &count) == B_OK) {
	#else
		while (msg->GetInfo(B_ANY_TYPE,i ,(char **)&name, &type, &count) == B_OK) {
	#endif
		switch(type){
			case B_MESSAGE_TYPE:{
				msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes);
				xmlNode.InsertEndChild(ProcessMessage(name,(BMessage*)data));
			}
			case B_BOOL_TYPE:{
		        //FindBool()
		        if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					TiXmlElement	xmlSubNode(name);
					xmlSubNode.SetAttribute("type","bool");
					if  (*((bool *)data) = true)
						xmlSubNode.SetAttribute("value","true");
					else
						xmlSubNode.SetAttribute("value","false");
					xmlNode.InsertEndChild(xmlSubNode);
				}	
			}
			case B_INT8_TYPE:{
		        //FindInt8()	an int8 or uint8	
				 if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					TiXmlElement	xmlSubNode(name);
					xmlSubNode.SetAttribute("type","int8");
					xmlSubNode.SetAttribute("value",*(int8*)data);
					xmlNode.InsertEndChild(xmlSubNode);
				}	
			}
			case B_INT16_TYPE:{
		        //FindInt16()	an int16 or uint16	
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					TiXmlElement	xmlSubNode(name);
					xmlSubNode.SetAttribute("type","int16");
					xmlSubNode.SetAttribute("value",*((int16*)data));
					xmlNode.InsertEndChild(xmlSubNode);
				}
			}
			case B_INT32_TYPE:{
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					TiXmlElement	xmlSubNode(name);
					xmlSubNode.SetAttribute("type","int32");
					xmlSubNode.SetAttribute("value",*((int32 *)data));
					xmlNode.InsertEndChild(xmlSubNode);
				}	
			}
			case B_INT64_TYPE:{
		        //FindInt8()	an int8 or uint8	
				 if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					TiXmlElement	xmlSubNode(name);
					xmlSubNode.SetAttribute("type","int64");
					xmlSubNode.SetAttribute("value",*((int64*)data));
					xmlNode.InsertEndChild(xmlSubNode);
				}	
			}
			case B_FLOAT_TYPE:{
		        //FindInt16()	an int16 or uint16	
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					TiXmlElement	xmlSubNode(name);
					xmlSubNode.SetAttribute("type","float");
					xmlSubNode.SetDoubleAttribute("value",*((float*)data));
					xmlNode.InsertEndChild(xmlSubNode);
				}
			}
			case B_DOUBLE_TYPE:{
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					TiXmlElement	xmlSubNode(name);
					xmlSubNode.SetAttribute("type","double");
					xmlSubNode.SetDoubleAttribute("value",*((double *)data));
					xmlNode.InsertEndChild(xmlSubNode);		
				}	
			}	
			case B_STRING_TYPE:{
				 if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					TiXmlElement	xmlSubNode(name);
					xmlSubNode.SetAttribute("type","string");
					xmlSubNode.SetAttribute("value",(char *)data);
					xmlNode.InsertEndChild(xmlSubNode);
				}	
			}
			case B_POINT_TYPE:{
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					TiXmlElement	xmlSubNode(name);
					xmlSubNode.SetAttribute("type","BPoint");
					xmlSubNode.SetDoubleAttribute("x",((BPoint*)data)->x);
					xmlSubNode.SetDoubleAttribute("y",((BPoint*)data)->y);
					xmlNode.InsertEndChild(xmlSubNode);
				}
			}
			case B_RECT_TYPE:{
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					TiXmlElement	xmlSubNode(name);
					xmlSubNode.SetAttribute("type","BRect");
					xmlSubNode.SetDoubleAttribute("top",((BRect*)data)->top);
					xmlSubNode.SetDoubleAttribute("left",((BRect*)data)->left);
					xmlSubNode.SetDoubleAttribute("bottom",((BRect*)data)->bottom);
					xmlSubNode.SetDoubleAttribute("right",((BRect*)data)->right);
					xmlNode.InsertEndChild(xmlSubNode);
				}	
			}	
			case B_REF_TYPE:{
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					BPath *path= new BPath((entry_ref*)data);
					TiXmlElement	xmlSubNode(name);
					xmlSubNode.SetAttribute("type","B_REF_TYPE");
					xmlSubNode.SetAttribute("path",path->Path());
					xmlNode.InsertEndChild(xmlSubNode);
				}	
			}	
	/*		case B_MESSENGER_TYPE:{
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					BMessage *messengerArchive	= new BMessage();
					((BMessenger*)data)->Archive(messengerArchive , true);
					TiXmlElement	xmlSubNode(name);
					xmlSubNode.SetAttribute("type","B_MESSENGER_TYPE");
					xmlSubNode.InsertEndChild(ProcessMessage("MessengerNode",messengerArchive));
					xmlNode.InsertEndChild(xmlSubNode);
				}	
			}*/	
			case B_POINTER_TYPE:{
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					TiXmlElement	xmlSubNode(name);
					xmlSubNode.SetAttribute("type","B_POINTER_TYPE");
					xmlSubNode.SetDoubleAttribute("value",(int32)data);
					xmlNode.InsertEndChild(xmlSubNode);
				}	
			}
		}
	}
}
