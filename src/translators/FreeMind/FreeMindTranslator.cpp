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
	//read the first 10 chars check if it starts with <map then we suggest it is a freemind file
	if (inSource->Read(xmlString, 10)==10)
	{
		if (strstr(xmlString,"<map")!=NULL)
			outType = P_C_DOCUMENT_RAW_TYPE;
		else
			return B_NO_TRANSLATOR;
	}
	//need to rework this... unflattens the whole Datastream to check if PDocument::allNodes are in here
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
				return B_NO_TRANSLATOR;
		}
	}


	if (outType == P_C_FREEMIND_TYPE) {
		// outtype is text becaus xml is text
		outInfo->group = B_TRANSLATOR_TEXT;
		outInfo->type = P_C_FREEMIND_TYPE;
		outInfo->quality = 0.3;
		outInfo->capability = 0.7;
		strcpy(outInfo->name, "Freemind Mindmap format");
		strcpy(outInfo->MIME, "application/x-freemind");
	}
	else {
	// outtype group ist always B_TRANSLATOR_NONE because we dont support a standart type
		outInfo->group = B_TRANSLATOR_NONE;
		outInfo->type = P_C_DOCUMENT_RAW_TYPE;
		outInfo->quality = 0.4;
		outInfo->capability = 0.7;
		strcpy(outInfo->name, "ProjectConceptor format converted from Freemind");
		strcpy(outInfo->MIME, P_C_DOCUMENT_MIMETYPE);
	}
	return err;
}

status_t Translate(BPositionIO * inSource,const translator_info *tInfo,	BMessage * ioExtension,	uint32 outType,	BPositionIO * outDestination)
{
	status_t		err					= B_OK;
	if ( (outType != P_C_DOCUMENT_RAW_TYPE) &&  (outType != P_C_FREEMIND_TYPE))
		return B_NO_TRANSLATOR;
	Converter	*converter	= new Converter(inSource,ioExtension,outDestination);
	if (outType == P_C_DOCUMENT_RAW_TYPE)
		converter->ConvertFreeMind2PDoc();
	else
		converter->ConvertPDoc2FreeMind();
	inSource->Seek(0, SEEK_SET);
	outDestination->Seek(0, SEEK_SET);
	inSource->Seek(0,SEEK_SET);
	outDestination->Seek(0,SEEK_SET);
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
		if (tmpMessage != NULL) {
			freeMap.InsertEndChild(ProcessNode(tmpMessage));
			doc.InsertEndChild(freeMap);
		}
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
	middel.Set(400,400,600,550);
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
		CreateNode(allNodes, allConnections,element,0,0);
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
	node->FindMessage(P_C_NODE_DATA,data);
	data->FindString(P_C_NODE_NAME,(const char **)&name);

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
		connection->FindPointer(P_C_NODE_CONNECTION_FROM,&fromNode);
		connection->FindPointer(P_C_NODE_CONNECTION_TO, &toNode);
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
	if (connection->FindPointer(P_C_NODE_CONNECTION_FROM, &fromNode) == B_OK)
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
			connection->FindPointer(P_C_NODE_CONNECTION_TO,&toNode);
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
	//we didnt find a node while searching through the connections...
	if ((*iter).second == NULL){
		iter=nodes.begin();
	}
	return (*iter).second;
}

status_t Converter::CreateNode(BMessage *nodeS,BMessage *connectionS,TiXmlElement *parent,int32 level, int32 thisLine)
{
	TiXmlNode		*node;
	BMessage		*pDocNode	= new BMessage(P_C_CLASS_TYPE);
	BMessage		*data		= new BMessage();
	BMessage		*pattern	= new BMessage();
	int32			line		= 0;
	for( node = parent->FirstChild("node"); node;)
	{
		CreateConnection(connectionS, parent,node->ToElement());
		CreateNode(nodeS,connectionS, node->ToElement(),level+1, line);
		line++;
		node = node->NextSibling();
	}
	if (parent->Attribute("TEXT"))
		data->AddString(P_C_NODE_NAME,parent->Attribute("TEXT"));
	else
		data->AddString(P_C_NODE_NAME,"Unnamed");
	if (parent->Attribute("ID"))
	{
		const char	*idString = parent->Attribute("ID");
		int32	id	= GetID(idString);
		pDocNode->AddPointer("this",(void *)id);
	}
	if (parent->Attribute("CREATED"))
		pDocNode->AddInt32(P_C_NODE_CREATED,atoi(parent->Attribute("CREATED")));
	if (parent->Attribute("MODIFIED"))
		pDocNode->AddInt32(P_C_NODE_MODIFIED,atoi(parent->Attribute("MODIFIED")));
	if (parent->Attribute("BACKGROUND_COLOR"))
		pattern->AddInt32("FillColor",GetRGB(parent->Attribute("BACKGROUND_COLOR")));
	if (parent->Attribute("COLOR"))
		pattern->AddInt32("BorderColor",GetRGB(parent->Attribute("COLOR")));
	//find all Attributes
	for (node = parent->FirstChild("arrowlink"); node;)
	{
		CreateConnection(connectionS,parent,node->ToElement());
		node = node->NextSibling();
	}
	pDocNode->AddMessage(P_C_NODE_DATA,data);
	pDocNode->AddMessage(P_C_NODE_PATTERN,pattern);
	BRect	*nodeRect	= new BRect(100,100,200,150);
	if (pDocNode->FindRect(P_C_NODE_FRAME,nodeRect) !=  B_OK)
	{
		int32	left, top, right, bottom;
		if (level == 0)
		{
			left	= X_START;
			top		= Y_START;
		}
		else
		{
			if (parent->Attribute("POSITION"))
			{
				if (strcmp(parent->Attribute("POSITION"),"left") != 0)
					left	= (level*(NODE_WIDTH+10))+X_START;
				else
					left	= X_START-(level*(NODE_WIDTH+10));
			}
			top		= (thisLine*(NODE_HEIGHT+10))+ 10;
		}
		right	= left + NODE_WIDTH;
		bottom	= top + NODE_HEIGHT;
		nodeRect->Set(left,top, right, bottom);
		pDocNode->AddRect(P_C_NODE_FRAME,*nodeRect);
	}
	nodeS->AddMessage("node",pDocNode);
}

status_t Converter::CreateConnection(BMessage *container,TiXmlElement *start,TiXmlElement *end)
{
	BMessage	*connection	= new BMessage(P_C_CONNECTION_TYPE);
	BMessage	*data	= new BMessage();
	char	*idFrom = (char*)start->Attribute("ID");
	connection->AddPointer(P_C_NODE_CONNECTION_FROM,(void *)GetID(idFrom));
	char	*idTo;
	const char	*value= end->Value();
	int32	found	= strcmp(end->Value(),"node");
	if (found == 0)
	{
		idTo	=  (char*)end->Attribute("ID");
		data->AddString(P_C_NODE_NAME,"Unnamed");
	}
	else
	{
		idTo	= (char*)end->Attribute("DESTINATION");
		if (end->Attribute("TEXT"))
			data->AddString(P_C_NODE_NAME,end->Attribute("TEXT"));
		else
			data->AddString(P_C_NODE_NAME,"Unnamed");
	}
	connection->AddPointer(P_C_NODE_CONNECTION_TO,(void *)GetID(idTo));
	connection->AddMessage(P_C_NODE_DATA,data);
	container->AddMessage("nodes", connection);
}

int32 Converter::GetID(const char *idString)
{
	int32	id;
	if (strstr(idString,"Freemind")!=NULL)
		sscanf(idString,"Freemind_Link_%d", &id);
	else
		sscanf(idString,"%d",&id);
	return id;
}

int32 Converter::GetRGB(const char *rgbString){
	rgb_color	rgb;
	sscanf(rgbString,"#%x%x%x",rgb.red,rgb.green,rgb.blue);
	return *((int32 *) &rgb);
}
