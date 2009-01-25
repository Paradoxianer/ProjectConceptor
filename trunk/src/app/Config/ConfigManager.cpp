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

//rebuild the hole Config View
void ConfigManager::SetConfigMessage(BMessage *newConfig){

}

void  ConfigManager::LoadDefaults(void){

}

void ConfigManager::SaveConfig(void){

}

TiXmlElement  ConfigManager::ProcessMessage(char *name, BMessage *msg){
	TiXmlElement	xmlNode(name);
	xmlNode.SetAttribute("type","BMessage");
	xmlNode.SetAttribute("what",msg->what;);
	char		*name;
	uint32	type;
	int32		count;
	void		*data;
	ssize_t	numBytes;
	#ifdef B_ZETA_VERSION_1_0_0
		while (data->GetInfo(B_ANY_TYPE,i ,(const char **)&name, &type, &count) == B_OK) {
	#else
		while (data->GetInfo(B_ANY_TYPE,i ,(char **)&name, &type, &count) == B_OK) {
	#endif
		switch(type){
			case B_MESSAGE_TYPE:{
				msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes);
				xmlNode.InsertChild(ProcessMessage(name,(BMessage*)data));
			}
			case B_BOOL_TYPE:{
		        //FindBool()
		        if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					TiXmlElement	xmlSubNode(name);
					xmlSubNode.SetAttribute("type","bool");
					if  (((bool)*data) = true)
						xmlSubNode.SetValue("true");
					else
						xmlSubNode.SetValue("true");
					xmlNode.InsertChild(xmlSubNode);
				}	
			}
			case B_INT8_TYPE:{
		        //FindInt8()	an int8 or uint8	
				 if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					TiXmlElement	xmlSubNode(name);
					xmlSubNode.SetAttribute("type","int8");
					xmlSubNode.SetValue((int8)*data);
					xmlNode.InsertChild(xmlSubNode);
				}	
			}
			case B_INT16_TYPE:{
		        //FindInt16()	an int16 or uint16	
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					TiXmlElement	xmlSubNode(name);
					xmlSubNode.SetAttribute("type","int16");
					xmlSubNode.SetValue((int16)*data);
					xmlNode.InsertChild(xmlSubNode);
				}
			}
			case B_INT32_TYPE:{
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					TiXmlElement	xmlSubNode(name);
					xmlSubNode.SetAttribute("type","int32");
					xmlSubNode.SetValue((int32)*data);
					xmlNode.InsertChild(xmlSubNode);
				}	
			}
			case B_INT64_TYPE:{
		        //FindInt8()	an int8 or uint8	
				 if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					TiXmlElement	xmlSubNode(name);
					xmlSubNode.SetAttribute("type","int64");
					xmlSubNode.SetValue((int64)*data);
					xmlNode.InsertChild(xmlSubNode);
				}	
			}
			case B_FLOAT_TYPE:{
		        //FindInt16()	an int16 or uint16	
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					TiXmlElement	xmlSubNode(name);
					xmlSubNode.SetAttribute("type","float");
					xmlSubNode.SetValue((float)*data);
					xmlNode.InsertChild(xmlSubNode);
				}
			}
			case B_DOUBLE_TYPE:{
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					TiXmlElement	xmlSubNode(name);
					xmlSubNode.SetAttribute("type","double");
					xmlSubNode.SetValue((double)*data);
					xmlNode.InsertChild(xmlSubNode);		
				}	
			}	
			case B_STRING_TYPE:{
				 if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					TiXmlElement	xmlSubNode(name);
					xmlSubNode.SetAttribute("type","string");
					xmlSubNode.SetValue((char *)data);
					xmlNode.InsertChild(xmlSubNode);
				}	
			}
			case B_POINT_TYPE:{
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					TiXmlElement	xmlSubNode(name);
					xmlSubNode.SetAttribute("type","BPoint");
					xmlSubNode.SetAttribute("x",((BPoint)*data).x);
					xmlSubNode.SetAttribute("y",((BPoint)*data).y);
					xmlNode.InsertChild(xmlSubNode);
				}
			}
			case B_RECT_TYPE:{
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					TiXmlElement	xmlSubNode(name);
					xmlSubNode.SetAttribute("type","BRect");
					xmlSubNode.SetValue("top",((BRect)*data).top);
					xmlSubNode.SetValue("left",((BRect)*data).left);
					xmlSubNode.SetValue("bottom",((BRect)*data).bottom);
					xmlSubNode.SetValue("right",((BRect)*data).right);
					xmlNode.InsertChild(xmlSubNode);
				}	
			}	
			case B_REF_TYPE:{
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					BPath path= new BPath((entry_ref*)data);
					TiXmlElement	xmlSubNode(name);
					xmlSubNode.SetAttribute("type","B_REF_TYPE");
					xmlSubNode.SetValue("path",path.Path());
					xmlNode.InsertChild(xmlSubNode);
				}	
			}	
			case B_MESSENGER_TYPE:{
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					BMessage *messengerArchive;
					((BMessenger*)data)->Archive(messengerArchive,true);
					TiXmlElement	xmlSubNode(name);
					xmlSubNode.SetAttribute("type","B_MESSENGER_TYPE");
					xmlSubNode.InsertChild(ProcessMessage("MessengerNode",messengerArchive));
					xmlNode.InsertChild(xmlSubNode);
				}	
			}	
			case B_POINTER_TYPE:{
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					TiXmlElement	xmlSubNode(name);
					xmlSubNode.SetAttribute("type","B_POINTER_TYPE");
					xmlSubNode.SetValue("pointer",(int32)data);
					xmlNode.InsertChild(xmlSubNode);
				}	
			}
		}
	}
}
