#include <TranslatorAddOn.h>
#include <TranslationKit.h>
#include <ByteOrder.h>
#include <Message.h>
#include <Screen.h>
#include <Locker.h>
#include <FindDirectory.h>
#include <Path.h>
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
	}*/
	err = B_NO_TRANSLATOR;
	return err;
}

status_t Translate(BPositionIO * inSource,const translator_info *tInfo,	BMessage * ioExtension,	uint32 outType,	BPositionIO * outDestination)
{
	status_t		err					= B_OK;
	Converter	*converter	= new Converter(inSource,ioExtension,outDestination);
	converter->ConvertPDoc2FreeMind();
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
	TiXmlElement	*node;
	BMessage		*pDocNode = new pDocNode(P_C_CLASS_TYPE);
	BMessage		*data = new pDocNode();
	for( node = node.FirstChild("node");
			node;
			count++;)
	{
		CreateConnecion(connectionS, parent,node);
		CreateNode(nodeS,connectionS, node);
		node = node->NextSibling())
	}8
	if (parent->Attribute("TEXT"))
		data->AddString("Name",parent->Attribute("TEXT"));
	else
		data->AddString("Name","Unnamed");
	if (parent->Attribute("ID"))
	{
		char	*idString = parent->Attribute("ID");
		int32	id	= GetID(idString);
		pDocNode->AddPointer("this",(void *)id);
	}
	if (parent->Attribute("CREATED")
		pDocNode->AddInt("Node::created",atoi(parent->Attribute("CREATED"));
	if (parent->Attribute("MODIFIED")
		pDocNode->AddInt("Node::modified",atoi(parent->Attribute("MODIFIED"));
	//find all Attributes
	for (node = node.FirstChild("arrowlink");
		node;
		count++;)
	{
		CreateConnecion(connectionS,parent,node);
		node = node->NextSibling());
	}
	nodeS->AddMessage("node",pDocNode);
}

status_t Converter::CreateConnecion(BMessage *container,TiXmlElement *start,TiXmlElement *end)
{
	BMessage	*connection	= new BMessage(P_C_CONNECTION_TYPE);
	BMessage	*data	= new BMessage();
	char	*idFrom = start->Attribute("ID");
	connection->AddPointer("Node::from",GetID(idFrom));
	char	*idTo;
	//** richtige funktion zum vergleichen
	if (strcmp(node->Value(),"node"))
		idTo	= end->Attribute("ID");
	else
		idTo	= end->Attribute("DESTINATION");
	connection->AddPointer("Node::to",GetID(idTo));
	if (parent->Attribute("TEXT"))
			data->AddString("Name",parent->Attribute("TEXT"));
	else
		data->AddString("Name","Unnamed");
	container-AddMessage("nodes", conncetion);
}

int32 Converter::IDtoRefference(char *idString)
{
	//**find a function to delte the Freemind_Link_ prestring
	char idNumber;
	strncpy(idNumber,idString,);
	return atoi(idNumber);
}