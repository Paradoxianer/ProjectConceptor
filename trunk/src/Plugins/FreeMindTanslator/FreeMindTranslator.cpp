#include <TranslatorAddOn.h>
#include <TranslationKit.h>
#include <ByteOrder.h>
#include <Message.h>
#include <Screen.h>
#include <Locker.h>
#include <FindDirectory.h>
#include <Path.h>
#include <Alert.h>
#include <File.h>
#include <PopUpMenu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <CheckBox.h>
#include <Bitmap.h>

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "FreeMindTranslator.h"
#include "SettingsManager.h"
#include "ConfigView.h"

status_t Identify(BPositionIO * inSource, const translation_format * inFormat,	BMessage * ioExtension,	translator_info * outInfo, uint32 outType)
{
	status_t err	= B_OK;
	char		*xmlString = new char[10];
	if (outType == 0) 
		outType = P_C_FREEMIND_TYPE;
	if (outType != P_C_FREEMIND_TYPE && outType != P_C_DOCUMENT_RAW_TYPE) {
		return B_NO_TRANSLATOR;
	}	
	if (inSource->Read(xmlString, 10)==10)
	{
		if (strstr(xmlString,"<map")!=NULL)
			outType = P_C_DOCUMENT_RAW_TYPE;
	}
	else
	{
		BMessage	*testMessage	= new BMessage();
		BMessage	*tmpMessage		= new BMessage();
		err = testMessage->Unflatten(inSource);
		if (err == B_OK)
		{
			if (err == B_OK)
			{
				err = testMessage->FindMessage("PDocument::allNodes",tmpMessage);
				if (err==B_OK)
					outType = P_C_FREEMIND_TYPE;
			}
			if (err != B_OK)
				err = B_NO_TRANSLATOR;
		}
	}
	
	//outInfo->group = P_C_FREEMIND_TYPE;
	
	if (outType == P_C_FREEMIND_TYPE) {
		outInfo->type = P_C_FREEMIND_TYPE;
		outInfo->quality = 0.3;		
		outInfo->capability = 0.7;	
		strcpy(outInfo->name, "Freemind Mindmap format");
		strcpy(outInfo->MIME, "application/x-freemind");
	}
	else {
		outInfo->type = P_C_DOCUMENT_RAW_TYPE;
		outInfo->quality = 0.4;		
		outInfo->capability = 0.7;	
		strcpy(outInfo->name, "ProjectConceptor format converted from Freemind");
		strcpy(outInfo->MIME, P_C_DOCUMENT_MIMETYPE);
	}		
/*	BMessage		*testMessage	= new BMessage();
	BMessage		*tmpMessage		= new BMessage();
	if ((!inSource) || (!outInfo))
		err = B_BAD_VALUE;
	else
	{
		if (err == B_OK)
		{
			err = testMessage->Unflatten(inSource);
			if (err == B_OK)
			{
				if (err == B_OK)
					err = testMessage->FindMessage("PDocument::allNodes",tmpMessage);
				if (err != B_OK)
				{
					err = B_NO_TRANSLATOR;
				}
				else
				{
					outInfo->group = B_TRANSLATOR_NONE;
					outInfo->type = P_C_DOCUMENT_RAW_TYPE;
					outInfo->quality = 0.3;
					outInfo->capability = 0.8;
					strcpy(outInfo->name, "ProjectConceptor nativ format");
					strcpy(outInfo->MIME, P_C_DOCUMENT_MIMETYPE);
				}
			}
		}
	}
	err = B_NO_TRANSLATOR;
	*/
	return err;
}

status_t Translate(BPositionIO * inSource,const translator_info *tInfo,	BMessage * ioExtension,	uint32 outType,	BPositionIO * outDestination)
{
	status_t		err					= B_OK;
	if ( (outType != P_C_DOCUMENT_RAW_TYPE) &&  (outType != P_C_FREEMIND_TYPE))
		return B_NO_TRANSLATOR;
	Converter	*converter	= new Converter(inSource,ioExtension,outDestination);
	if (outType != P_C_DOCUMENT_RAW_TYPE)
		converter->ConvertPDoc2FreeMind();
	else
		converter->ConvertFreeMind2PDoc();
	return err;
}

status_t MakeConfig(BMessage * ioExtension,	BView * * outView, BRect * outExtent)
{
	status_t err	= B_OK;
	*outView = new ConfigView();
	return err;
}

status_t GetConfigMessage(BMessage * ioExtension)
{
	status_t err = B_OK;

	return err;
}

Converter::Converter(BPositionIO * inSource, BMessage * ioExtension,	BPositionIO * outDestination)
{
	in		= inSource;
	config	= ioExtension;
	out		= outDestination;
	//necessary to avoid problems
	out->Seek(0, SEEK_SET);
	in->Seek(0, SEEK_SET);
	allConnections		= NULL;
	allNodes			= NULL;
	selected			= NULL;
}

status_t Converter::ConvertPDoc2FreeMind()
{
	status_t		err					= B_OK;
	BMessage		*inMessage			= new BMessage();
	BMessage		*tmpMessage			= new BMessage();
	void			*id					= NULL;

 	allConnections	= new BMessage();
	selected		= new BMessage();
	allNodes		= new BMessage();


	err = inMessage->Unflatten(in);
	if (err == B_OK)
	{
		inMessage->FindMessage("PDocument::allConnections",allConnections);
		inMessage->FindMessage("PDocument::selected",selected);
		inMessage->FindMessage("PDocument::allNodes",allNodes);
		int32 i = 0;
		while(allNodes->FindMessage("node",i,tmpMessage)==B_OK)
		{
			tmpMessage->FindPointer("this",&id);
			nodes[(int32)id]=tmpMessage;
			tmpMessage = new BMessage();
-			i++;
		}
		i = 0;
		while(allConnections->FindMessage("node",i,tmpMessage)==B_OK)
		{
			tmpMessage->FindPointer("this",&id);
			connections[(int32)id]=tmpMessage;
			tmpMessage = new BMessage();
			i++;
		}

		BMessage	*node= GuessStartNode();
		TiXmlDocument	doc;
		TiXmlElement	freeMap("map");
		freeMap.SetAttribute("version","0.9.0");
		freeMap.SetAttribute("background_color","#ffffff");
		TiXmlComment	comment("this File was gernerated by ProjectConceptor! - To view this file, download free mind mapping software FreeMind from http://freemind.sourceforge.net");
		freeMap.InsertEndChild(comment);

		tmpMessage=GuessStartNode();
	//	tmpMessage = nodes.begin()->second;
		freeMap.InsertEndChild(ProcessNode(tmpMessage));
		doc.InsertEndChild(freeMap);
		TiXmlPrinter	printer;
//		printer.SetStreamPrinting();
//		printer.SetLineBreak("\n");
//		printer.SetIndent("\t");
		doc.Accept( &printer );
		out->Write(printer.CStr(),strlen(printer.CStr()));
	}
	return err;
}

status_t Converter::ConvertFreeMind2PDoc()
{
	BMessage	*document		= new BMessage();
	BMessage	*allNodes		= new BMessage();
	BMessage	*allConnections	= new BMessage();
	char		*xmlString;
	off_t		start,end;
	in->Seek(0,SEEK_SET);
	start = in->Position();
	in->Seek(0,SEEK_END);
	end = in->Position();
	in->Seek(0,SEEK_SET);	
	size_t		size= end-start;
	xmlString=new char[size+1];
	in->Read(xmlString, size);
	TiXmlDocument	doc;
	doc.Parse(xmlString);
	delete xmlString;
	if (doc.Error())
		return B_ERROR;
	else
	{
		TiXmlNode*		node	= NULL;
		TiXmlElement*	element	= NULL;
		node = doc.FirstChild("map");
		node = node->FirstChild("node");
		element	= node->ToElement();
		CreateNode(allNodes, allConnections,element);
	}
	document->AddMessage("PDocument::allConnections",allConnections);
	document->AddMessage("PDocument::allNodes",allNodes);
	status_t err= document->Flatten(out);
}


TiXmlElement Converter::ProcessNode(BMessage *node)
{
	int32			i = 0;
	void			*tmpNode		= NULL;
	void			*fromNode		= NULL;
	void			*toNode			= NULL;
	BMessage		*connection		= NULL;
	BMessage		*data			= new BMessage();
	BMessage		*attrib			= new BMessage();

	bool			found			= false;
	char			*name			= NULL;

	TiXmlElement	xmlNode("node");
	node->FindPointer("this", &tmpNode);
	//add this node to the processed List
	processedIDs.insert((int32)tmpNode);
	//find the data field where name and attributes are stored
	node->FindMessage("Node::Data",data);
	data->FindString("Name",(const char **)&name);

	xmlNode.SetAttribute("ID",(int32)tmpNode);
	xmlNode.SetAttribute("TEXT",(const char *)name);
	//add all Attributes
	type_code	type	= 0;
	int32		count	= 0;
	#ifdef B_ZETA_VERSION_1_0_0
		while (data->GetInfo(B_MESSAGE_TYPE,i ,(const char **)&name, &type, &count) == B_OK)
	#else
		while (data->GetInfo(B_MESSAGE_TYPE,i ,(char **)&name, &type, &count) == B_OK)
	#endif
	{
		if ( (data->FindMessage(name,count-1,attrib) == B_OK) && (attrib) )
		{
			char *attribName	= NULL;
			char *value			= NULL;
			attrib->FindString("Name",(const char **)&attribName);
			attrib->FindString("Value",(const char **)&value);
			//**need to hanlde bool
			TiXmlElement	xmlAttrib("attribute");
			xmlAttrib.SetAttribute("NAME",attribName);
			if(value)
				xmlAttrib.SetAttribute("VALUE",value);
			xmlNode.InsertEndChild(xmlAttrib);
		}
		i++;
	}
	//find all outgoing connections
	map<int32,BMessage*>::iterator iter;
	iter = connections.begin();
	while (iter!=connections.end())
	{
		connection=(*iter).second;
		connection->FindPointer("Node::from",&fromNode);
		connection->FindPointer("Node::to", &toNode);
		if ((fromNode == tmpNode) && (processedIDs.find((*iter).first) == processedIDs.end()))
		{
			//check if the node was already insert if so we "connect via a arrowlink
			if (processedIDs.find((int32)toNode) != processedIDs.end())
			{
				TiXmlElement	xmlLink("arrowlink");
				xmlLink.SetAttribute("ID",(*iter).first);
				xmlLink.SetAttribute("DESTINATION",(int32)toNode);
				xmlNode.InsertEndChild(xmlLink);
				processedIDs.insert((*iter).first);
			}
			else
			{
				map<int32,BMessage*>::iterator	found;
				found = nodes.find((int32)toNode);
				if (found!=nodes.end())
				{
					processedIDs.insert((*iter).first);
					xmlNode.InsertEndChild(ProcessNode((*found).second));
				}
			}
		}
		else if ((toNode == tmpNode) && (processedIDs.find((*iter).first)==processedIDs.end()))
		{
			//check if the node was already insert if so we "connect via a arrowlink
			if (processedIDs.find((int32)fromNode)!=processedIDs.end())
			{
				TiXmlElement	xmlLink("arrowlink");
				xmlLink.SetAttribute("ID",(*iter).first);
				xmlLink.SetAttribute("DESTINATION",(int32)fromNode);
				xmlNode.InsertEndChild(xmlLink);
			}
		}
		iter++;
	}
	return xmlNode;
}

BMessage* Converter::GuessStartNode(void)
{
	BMessage	*connection	= NULL;
	void		*fromNode	= NULL;
	void		*toNode		= NULL;
	void		*nodeID		= NULL;
	bool		found;
	set<int32>	visited;
	map<int32,BMessage*>::iterator iter;
	iter = connections.begin();
	//if there is a node given... we search for a Connection wich points to this node
	connection = (*iter).second;
	if (connection->FindPointer("Node::from", &fromNode) == B_OK)
	{
		nodeID = fromNode;
		visited.insert((int32)fromNode);
		found = true;
	}
	while (found && (visited.find((int32)fromNode) == visited.end()))
	{
		found=false;
		iter = connections.begin();
		while ((iter!=connections.end()) && (!found))
		{
			connection=(*iter).second;
			connection->FindPointer("Node::To",&toNode);
			if (toNode == fromNode)
			{
				visited.insert((int32)fromNode);
				nodeID=fromNode;
				fromNode=toNode;
				found=true;
			}
			iter++;
		}
	}
	iter = nodes.find((int32)nodeID);
	return (*iter).second;

}

status_t Converter::CreateNode(BMessage *nodeS,BMessage *connectionS,TiXmlElement *parent)
{
	TiXmlNode		*node;
	BMessage		*pDocNode	= new BMessage(P_C_CLASS_TYPE);
	BMessage		*data		= new BMessage();
	for( node = parent->FirstChild("node"); node;)
	{
		CreateConnection(connectionS, parent,node->ToElement());
		CreateNode(nodeS,connectionS, node->ToElement());
		node = node->NextSibling();
	}
	if (parent->Attribute("TEXT"))
		data->AddString("Name",parent->Attribute("TEXT"));
	else
		data->AddString("Name","Unnamed");
	if (parent->Attribute("ID"))
	{
		const char	*idString = parent->Attribute("ID");
		int32	id	= GetID(idString);
		pDocNode->AddPointer("this",(void *)id);
	}
	if (parent->Attribute("CREATED"))
		pDocNode->AddInt32("Node::created",atoi(parent->Attribute("CREATED")));
	if (parent->Attribute("MODIFIED"))
		pDocNode->AddInt32("Node::modified",atoi(parent->Attribute("MODIFIED")));
	//find all Attributes
	for (node = parent->FirstChild("arrowlink"); node;)
	{
		CreateConnection(connectionS,parent,node->ToElement());
		node = node->NextSibling();
	}
	pDocNode->AddMessage("Node::Data",data);
	nodeS->AddMessage("node",pDocNode);
}

status_t Converter::CreateConnection(BMessage *container,TiXmlElement *start,TiXmlElement *end)
{
	BMessage	*connection	= new BMessage(P_C_CONNECTION_TYPE);
	BMessage	*data	= new BMessage();
	char	*idFrom = (char*)start->Attribute("ID");
	connection->AddPointer("Node::from",(void *)GetID(idFrom));
	char	*idTo;
	const char	*value= end->Value();
	int32	found	= strcmp(end->Value(),"node");
	if (found == 0)
	{
		idTo	=  (char*)end->Attribute("ID");
		data->AddString("Name","Unnamed");
	}
	else
	{
		idTo	= (char*)end->Attribute("DESTINATION");
		if (end->Attribute("TEXT"))
			data->AddString("Name",end->Attribute("TEXT"));
		else
			data->AddString("Name","Unnamed");
	}
	connection->AddPointer("Node::to",(void *)GetID(idTo));
	connection->AddMessage("Node::Data",data);
	container->AddMessage("nodes", connection);
}

int32 Converter::GetID(const char *idString)
{
	char	*idNumber;
	int32	id;
	if (strstr(idString,"Freemind")!=NULL)
		sscanf(idString,"Freemind_Link_%d", &idNumber);
	else
		sscanf(idString,"%d",&idNumber);
	return id;
}
