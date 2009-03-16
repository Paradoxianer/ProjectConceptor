#include "ConfigManager.h"
#include <stdio.h>
#include <app/Messenger.h>
#include <interface/InterfaceDefs.h>
#include <storage/Path.h>

#include <support/Debug.h>


//needed for strerror
#include <string.h>

#ifdef B_ZETA_VERSION_BETA
	#include <locale/Locale.h>
#else
	#define _T(a) a
#endif



ConfigManager::ConfigManager(char *path, BMessage *newConfig){
// if no Message was passed we just create a Message :)
 if (newConfig==NULL) {
 	config = new BMessage();
 } 
 else
 	config = newConfig;
 BFile *file = new BFile(path,B_READ_WRITE | B_CREATE_FILE);
 //** @todo check for errors if the config file was created*/
}

//rebuild the hole Config View
void ConfigManager::SetConfigMessage(BMessage *newConfig){

}

void  ConfigManager::LoadDefaults(void){
	char		*xmlString;
	off_t		start,end;
	file->Seek(0,SEEK_SET);
	start = file->Position();
	file->Seek(0,SEEK_END);
	end = file->Position();
	file->Seek(0,SEEK_SET);
	size_t		size= end-start;
	xmlString=new char[size+1];
	file->Read(xmlString, size);
	TiXmlDocument	doc;
	doc.Parse(xmlString);
	delete xmlString;
	if (doc.Error())
		//need to do something
		start=0;
	else{		
	}	
}

void ConfigManager::SaveConfig(){
	TRACE();
  	TiXmlDocument	 doc;
  	doc.InsertEndChild(	ProcessMessage(config));
  	TiXmlPrinter	printer;
  	doc.Accept( &printer);
  	printf("%s",printer.CStr());
  	TRACE();
	//file->Write(printer.CStr(),strlen(printer.CStr()));
}

TiXmlElement  ConfigManager::ProcessMessage( BMessage *msg){
	TRACE();
	msg->PrintToStream();
	TiXmlElement	xmlNode("BMessage");
	xmlNode.SetAttribute("type","BMessage");
	xmlNode.SetAttribute("what",(int32)(msg->what));
	char		*name;
	uint32	type;
	int32		count;
	void		*data;
	int32		i =0;
	ssize_t	numBytes;
	TRACE();
	#ifdef B_ZETA_VERSION_1_0_0
		while (msg->GetInfo(B_ANY_TYPE,i ,(const char **)&name, &type, &count) == B_OK) {
	#else
		while (msg->GetInfo(B_ANY_TYPE,i ,(char **)&name, &type, &count) == B_OK) {
	#endif
		TRACE();
		TiXmlElement	xmlSubNode("Data");
		xmlSubNode.SetAttribute("name",name);
		switch(type){
			case B_MESSAGE_TYPE:{
				TRACE();
				xmlSubNode.SetAttribute("type","BMessage");
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes)== B_OK){
					xmlSubNode.InsertEndChild(ProcessMessage((BMessage*)data));
				}
				break;
			}
			case B_BOOL_TYPE:{
		        TRACE();
		        //FindBool()
		        xmlSubNode.SetAttribute("type","bool");
		        if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					if  (*((bool *)data) = true)
						xmlSubNode.SetAttribute("value","true");
					else
						xmlSubNode.SetAttribute("value","false");
				}	
				break;
			}
			case B_INT8_TYPE:{
		        TRACE();
		        //FindInt8()	an int8 or uint8	
				xmlSubNode.SetAttribute("type","int8");
				 if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					xmlSubNode.SetAttribute("value",*(int8*)data);
				}
				break;	
			}
			case B_INT16_TYPE:{
		        TRACE();
		        //FindInt16()	an int16 or uint16	
				xmlSubNode.SetAttribute("type","int16");
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					xmlSubNode.SetAttribute("value",*((int16*)data));
				}
				break;
			}
			case B_INT32_TYPE:{
				TRACE();
				xmlSubNode.SetAttribute("type","int32");
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					xmlSubNode.SetAttribute("value",*((int32 *)data));
				}	
				break;
			}
			case B_INT64_TYPE:{
				TRACE();
				xmlSubNode.SetAttribute("type","int64");
				 if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					xmlSubNode.SetAttribute("value",*((int64*)data));
				}	
				break;
			}
			case B_FLOAT_TYPE:{
		        TRACE();
				xmlSubNode.SetAttribute("type","float");
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					xmlSubNode.SetDoubleAttribute("value",*((float*)data));
				}
				break;
			}
			case B_DOUBLE_TYPE:{
					TRACE();
				xmlSubNode.SetAttribute("type","double");
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					xmlSubNode.SetDoubleAttribute("value",*((double *)data));
				}	
				break;
			}	
			case B_STRING_TYPE:{
				TRACE();
				xmlSubNode.SetAttribute("type","string");
				 if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					printf("Testausgabe %s",(char *)data);
					xmlSubNode.SetAttribute("value",(char *)data);
				}
				else
				 	printf("%s", (char *)data);
				 TRACE();
				break;
			}
			case B_POINT_TYPE:{
				TRACE();
				xmlSubNode.SetAttribute("type","BPoint");
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					xmlSubNode.SetDoubleAttribute("x",((BPoint*)data)->x);
					xmlSubNode.SetDoubleAttribute("y",((BPoint*)data)->y);
				}
				break;
			}
			case B_RECT_TYPE:{
				TRACE();
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					xmlSubNode.SetAttribute("type","BRect");
					xmlSubNode.SetDoubleAttribute("top",((BRect*)data)->top);
					xmlSubNode.SetDoubleAttribute("left",((BRect*)data)->left);
					xmlSubNode.SetDoubleAttribute("bottom",((BRect*)data)->bottom);
					xmlSubNode.SetDoubleAttribute("right",((BRect*)data)->right);
				}	
				break;
			}	
			case B_REF_TYPE:{
				TRACE();
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					BPath *path= new BPath((entry_ref*)data);
					xmlSubNode.SetAttribute("type","B_REF_TYPE");
					xmlSubNode.SetAttribute("path",path->Path());
				}	
				break;
			}	
	/*		case B_MESSENGER_TYPE:{
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					BMessage *messengerArchive	= new BMessage();
					((BMessenger*)data)->Archive(messengerArchive , true);
					xmlSubNode.SetAttribute("type","B_MESSENGER_TYPE");
					xmlSubNode.InsertEndChild(ProcessMessage(messengerArchive));
				}	
			}*/	
			case B_POINTER_TYPE:{
				TRACE();
				if (msg->FindData(name, B_ANY_TYPE, i, (const void **)&data, &numBytes) == B_OK){
					xmlSubNode.SetAttribute("type","B_POINTER_TYPE");
					xmlSubNode.SetDoubleAttribute("value",(int32)data);
				}	
				break;
			}
		}
		xmlNode.InsertEndChild(xmlSubNode);
		i++;
	}
	return xmlNode;
}
